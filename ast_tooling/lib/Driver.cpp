#include "Driver.h"
#include "BaseAnalyzer.h"
#include "CompilerInstance.h"
#include "DiagnosticDispatcher.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/LLVM.h"
#include "clang/Driver/Action.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Job.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/Util.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include <clang/Driver/Driver.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Support/Host.h>
#include <memory>
#include <string>
#include <sys/_types/_size_t.h>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace astooling;

typedef std::pair<std::string, clang::tooling::CompileCommand>
    CompileCommandPair;
typedef std::vector<CompileCommandPair> CompileCommandPairs;

static void constructCompileCommands(
    CompileCommandPairs &compileCommandPairs,
    const clang::tooling::CompilationDatabase &compilationDatabase,
    llvm::ArrayRef<std::string> sourcePaths) {
  for (const std::string &sourcePath : sourcePaths) {
    std::string filePath(clang::tooling::getAbsolutePath(sourcePath));
    std::vector<clang::tooling::CompileCommand> compileCmdsForFile =
        compilationDatabase.getCompileCommands(filePath);
    if (compileCmdsForFile.empty()) {
      continue;
    }

    for (clang::tooling::CompileCommand &compileCmd : compileCmdsForFile) {
      compileCommandPairs.push_back(std::make_pair(filePath, compileCmd));
    }
  }
}

static std::vector<std::string>
adjustedArguments(std::vector<std::string> &unadjustedCommandLine,
                  const std::string &filename) {
  clang::tooling::ArgumentsAdjuster argAdjuster =
      clang::tooling::combineAdjusters(
          clang::tooling::getClangStripOutputAdjuster(),
          clang::tooling::getClangSyntaxOnlyAdjuster());
  return argAdjuster(unadjustedCommandLine, filename);
}

std::string stringReplace(std::string orig, std::string oldStr,
                          std::string newStr) {
  std::string::size_type pos(orig.find(oldStr));

  while (pos != std::string::npos) {
    orig.replace(pos, oldStr.length(), newStr);
    pos = orig.find(oldStr, pos + newStr.length());
  }

  return orig;
}

static clang::driver::Driver *makeDriver(clang::DiagnosticsEngine *diagnostics,
                                         const char *binName) {
  clang::driver::Driver *driver = new clang::driver::Driver(
      binName, llvm::sys::getDefaultTargetTriple(), *diagnostics);
  driver->setTitle("ASTooling");
  return driver;
}

static const llvm::opt::ArgStringList *
getCC1Args(clang::driver::Compilation *compilation) {
  const clang::driver::JobList &jobList = compilation->getJobs();

  size_t jobSize = jobList.size();

  if (jobSize == 0) {
    // TODO throw exception
  }

  bool offloadCompilation = false;
  if (jobSize > 1) {
    clang::driver::ActionList actions = compilation->getActions();
    for (clang::driver::Action *action : actions) {
      if (llvm::isa<clang::driver::OffloadAction>(action)) {
        offloadCompilation = true;
        break;
      }
    }
  }
  if (jobSize > 1 && !offloadCompilation) {
    // TODO throw exception
  }
  if (!clang::isa<clang::driver::Command>(*jobList.begin())) {
    // TODO throw exception
  }

  const clang::driver::Command &cmd =
      clang::cast<clang::driver::Command>(*jobList.begin());
  if (llvm::StringRef(cmd.getCreator().getName()) != "clang") {
    // TODD throw exception
  }

  return &cmd.getArguments();
}

static clang::CompilerInvocation *
makeCompilerInvocation(std::string &mainExcutable,
                       std::vector<std::string> &commandLine) {
  // TODO: assert comamnd line is empty

  std::vector<const char *> argv;
  for (int index = 0; index != commandLine.size(); index++) {
    if (commandLine[index] != "-gmodules") {
      argv.push_back(commandLine[index].c_str());
    }
  }
  // TODO: what hell
  argv.push_back("-D__OCLINT__");

  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts =
      new clang::DiagnosticOptions();
  clang::DiagnosticsEngine diagnosticsEngine(
      llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(
          new clang::DiagnosticIDs()),
      &*diagOpts, new clang::DiagnosticConsumer());

  const char *const mainBinaryPath = argv[0];
  const std::unique_ptr<clang::driver::Driver> driver(
      makeDriver(&diagnosticsEngine, mainBinaryPath));
  // TODO: why
  driver->setCheckInputsExist(false);

  std::unique_ptr<clang::driver::Compilation> compilation(
      driver->BuildCompilation(llvm::makeArrayRef(argv)));
  const llvm::opt::ArgStringList *args = getCC1Args(compilation.get());

  clang::CompilerInvocation *invocation = new clang::CompilerInvocation;
  clang::CompilerInvocation::CreateFromArgs(*invocation, *args,
                                            *&diagnosticsEngine);
  invocation->getFrontendOpts().DisableFree = false;
  return invocation;
}

static astooling::CompilerInstance *
makeCI(clang::CompilerInvocation *compilerInvocation) {
  astooling::CompilerInstance *CI = new astooling::CompilerInstance();
  std::shared_ptr<clang::CompilerInvocation> invocation =
      std::make_shared<clang::CompilerInvocation>(*compilerInvocation);

  CI->setInvocation(std::move(invocation));
  CI->createDiagnostics(new DiagnosticDispatcher());
  if (!CI->hasDiagnostics()) {
    // TODO throw exceptions
  }

  return CI;
}

static void constructCIs(std::vector<astooling::CompilerInstance *> &CIs,
                         CompileCommandPairs &compilerCommandPairs,
                         std::string mainExcutable) {
  for (CompileCommandPair compilerCommandPair : compilerCommandPairs) {

    std::vector<std::string> adjustedCommandLine = adjustedArguments(
        compilerCommandPair.second.CommandLine, compilerCommandPair.first);

    std::string targetDir =
        stringReplace(compilerCommandPair.second.Directory, "\\ ", " ");

    if (chdir(targetDir.c_str())) {
      // TODO throw exception
    }

    clang::CompilerInvocation *compilerInvocation =
        makeCompilerInvocation(mainExcutable, adjustedCommandLine);
    astooling::CompilerInstance *CI = makeCI(compilerInvocation);

    CI->start();
    if (!CI->getDiagnostics().hasErrorOccurred() && CI->hasASTContext()) {
      CIs.push_back(CI);
    }
  }
}

static void invoke(CompileCommandPairs &compileCommandPairs,
                   std::string &mainExcutable,
                   astooling::BaseAnalyzer &analyzer) {
  std::vector<astooling::CompilerInstance *> CIs;
  constructCIs(CIs, compileCommandPairs, mainExcutable);

  std::vector<clang::ASTContext *> localContexts;
  for (astooling::CompilerInstance *CI : CIs) {
    localContexts.push_back(&CI->getASTContext());
  }

  analyzer.preprocess(localContexts);
  analyzer.analyze(localContexts);
  analyzer.postprocess(localContexts);
}

void Driver::run(const clang::tooling::CompilationDatabase &compilationDatabase,
                 llvm::ArrayRef<std::string> sourcePaths,
                 astooling::ToolingAnalyzer &analyzer) {
  CompileCommandPairs compileCommandPairs;
  constructCompileCommands(compileCommandPairs, compilationDatabase,
                           sourcePaths);

  static int staticSymbol;
  std::string mainExcutable =
      llvm::sys::fs::getMainExecutable("astlint", &staticSymbol);

  for (CompileCommandPair compileCommandPair : compileCommandPairs) {
    CompileCommandPairs oneCompileCommand{compileCommandPair};
    invoke(oneCompileCommand, mainExcutable, analyzer);
  }
}
