#include "DiagnosticDispatcher.h"
#include "ResultCollector.h"
#include "Violation.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include <llvm/ADT/SmallString.h>

struct LocalSourceLocation {
  int line;
  int column;
  std::string filename;
};

LocalSourceLocation emptySourceLocation() {
  LocalSourceLocation sourceLoc;
  sourceLoc.line = 0;
  sourceLoc.column = 0;
  sourceLoc.filename = "";

  return sourceLoc;
}

LocalSourceLocation
populateSourceLocation(const clang::Diagnostic &diagnosticInfo) {
  LocalSourceLocation sourceLoc = emptySourceLocation();

  if (!diagnosticInfo.hasSourceManager()) {
    return sourceLoc;
  }

  clang::SourceManager *sourceManager = &diagnosticInfo.getSourceManager();
  clang::SourceLocation location = diagnosticInfo.getLocation();

  llvm::StringRef sourceFilename = sourceManager->getFilename(location);
  if (sourceFilename.empty() && location.isMacroID()) {
    // TODO: what is macro location
    clang::SourceLocation macroLocation =
        sourceManager->getExpansionLoc(location);
    llvm::StringRef expansionFilename =
        sourceManager->getFilename(macroLocation);
    sourceLoc.filename = expansionFilename.str();
  } else {
    sourceLoc.filename = sourceFilename.str();
  }

  sourceLoc.line = sourceManager->getPresumedLineNumber(location);
  sourceLoc.column = sourceManager->getPresumedColumnNumber(location);

  return sourceLoc;
}

void astooling::DiagnosticDispatcher::HandleDiagnostic(
    clang::DiagnosticsEngine::Level diagnosticLevel,
    const clang::Diagnostic &diagnosticInfo) {
  clang::DiagnosticConsumer::HandleDiagnostic(diagnosticLevel, diagnosticInfo);

  clang::SmallString<100> diagnosticMessage;
  diagnosticInfo.FormatDiagnostic(diagnosticMessage);

  LocalSourceLocation localSourceLocation =
      populateSourceLocation(diagnosticInfo);
  Violation violation(nullptr, localSourceLocation.filename,
                      localSourceLocation.line, localSourceLocation.column, 0,
                      0, diagnosticMessage.str().str());

  ResultCollector *results = ResultCollector::getInstance();
  if (diagnosticLevel == clang::DiagnosticsEngine::Warning) {
    results->addWarning(violation);
  }

  if (diagnosticLevel == clang::DiagnosticsEngine::Error ||
      diagnosticLevel == clang::DiagnosticsEngine::Fatal) {
    results->addError(violation);
  }
}
