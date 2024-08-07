#ifndef ASTOOLING_DRIVER_H
#define ASTOOLING_DRIVER_H

#include "ToolingAnalyzer.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/ADT/ArrayRef.h"
#include <string>

namespace astooling {

class Driver {
public:
  void run(const clang::tooling::CompilationDatabase &compilationDatabase,
           llvm::ArrayRef<std::string> sourcePaths,
           astooling::ToolingAnalyzer &analyzer);
};

} // namespace astooling

#endif
