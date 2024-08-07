#include "Violation.h"

using namespace astooling;

Violation::Violation(RuleBase *violatedRule, std::string violationFilePath,
                     int violationStartLine, int violationStartColumn,
                     int violationEndLine, int violationEndColumn,
                     std::string violationMessage)
    : path(std::move(violationFilePath)), message(std::move(violationMessage)),
      rule(violatedRule), startLine(violationStartLine),
      startColumn(violationStartColumn), endLine(violationEndLine),
      endColumn(violationEndColumn) {}

bool Violation::operator==(const Violation &rhs) const {
  return (rule == rhs.rule) && (path == rhs.path) &&
         (startLine == rhs.startLine) && (startColumn == rhs.startColumn) &&
         (endLine == rhs.endLine) && (endColumn == rhs.endColumn) &&
         (message == rhs.message);
}
