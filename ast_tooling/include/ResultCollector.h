#ifndef ASTOOLING_RESULTCOLLECTOR_H
#define ASTOOLING_RESULTCOLLECTOR_H

#include "Violation.h"
#include <memory>
#include <vector>
namespace astooling {
class ViolationSet;

class ResultCollector {
protected:
  ResultCollector();
  ~ResultCollector();

private:
  std::vector<ViolationSet *> _collection;
  std::unique_ptr<ViolationSet> _compilerErrorSet;
  std::unique_ptr<ViolationSet> _compilerWarningSet;

public:
  static ResultCollector *getInstance();

  void add(ViolationSet *violationSet);
  const std::vector<ViolationSet *>& getCollection() const;

  void addError(const Violation& violation);
  ViolationSet* getCompilerErrorSet() const;

  void addWarning(const Violation& violation);
  ViolationSet* getCompilerWarningSet() const;
};

} // namespace astooling

#endif
