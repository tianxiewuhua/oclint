#ifndef ASTOOLING_VIOLATION_H
#define ASTOOLING_VIOLATION_H

#include <string>

namespace astooling {

class RuleBase;

class Violation {
public:
  const RuleBase *rule;
  std::string path;
  int startLine;
  int startColumn;
  int endLine;
  int endColumn;
  std::string message;

  Violation(RuleBase *violatedRule, std::string violationFilePath,
            int violationStartLine, int violationStartColumn,
            int violationEndLine, int violationEndColumn,
            std::string violationMessage = "");

  bool operator==(const astooling::Violation &rhs) const;
};

} // namespace astooling

#endif
