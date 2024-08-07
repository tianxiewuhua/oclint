#ifndef ASTOOLING_TOOLING_ANALYZER_H
#define ASTOOLING_TOOLING_ANALYZER_H

#include "BaseAnalyzer.h"

namespace astooling {

class ToolingAnalyzer : public BaseAnalyzer {
public:
  void analyze(std::vector<clang::ASTContext *> &contexts) override;
};

} // namespace astooling

#endif
