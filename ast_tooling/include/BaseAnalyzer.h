#ifndef ASTOOLING_BASE_ANALYZER_H
#define ASTOOLING_BASE_ANALYZER_H

#include "clang/AST/ASTContext.h"
#include <clang/AST/AST.h>
#include <vector>

namespace astooling {

class BaseAnalyzer {
public:
  virtual void preprocess(std::vector<clang::ASTContext *> &contexts){};
  virtual void analyze(std::vector<clang::ASTContext *> &contexts) = 0;
  virtual void postprocess(std::vector<clang::ASTContext *> &contexts){};
};

} // namespace astooling

#endif
