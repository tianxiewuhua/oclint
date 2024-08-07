#include "Options.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <iostream>
#include "Driver.h"
#include "ToolingAnalyzer.h"

using namespace std;
using namespace llvm;
using namespace clang::tooling;

llvm::cl::OptionCategory ASToolingOptionCategory("Astooling options");

int prepare() { return 0; }

int main(int argc, const char **argv) {
  static int staticSymbol;
  llvm::Expected<CommonOptionsParser> extecpedParser =
      CommonOptionsParser::create(argc, argv, ASToolingOptionCategory);

  if (!extecpedParser) {
    llvm::errs() << extecpedParser.takeError();
    return -1;
  }

  CommonOptionsParser &optionsParser = extecpedParser.get();
  astooling::option::process(argv[0]);

  int prepare_status = prepare();
  if (prepare_status) {
    return prepare_status;
  }

  astooling::Driver driver;
  astooling::ToolingAnalyzer analyzer;

  driver.run(optionsParser.getCompilations(), optionsParser.getSourcePathList(), analyzer);

  return 0;
}
