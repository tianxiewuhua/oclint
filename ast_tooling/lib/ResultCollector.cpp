#include "ResultCollector.h"
#include "ViolationSet.h"
#include <vector>

static astooling::ResultCollector *_singleton = nullptr;

namespace astooling {

ResultCollector *ResultCollector::getInstance() {
  if (_singleton == nullptr) {
    _singleton = new ResultCollector();
  }

  return _singleton;
}

ResultCollector::ResultCollector()
    : _compilerErrorSet(new ViolationSet),
      _compilerWarningSet(new ViolationSet) {}

void ResultCollector::add(ViolationSet *violationSet) {
  _collection.push_back(violationSet);
}

const std::vector<ViolationSet *> &ResultCollector::getCollection() const {
  return _collection;
}

void ResultCollector::addError(const Violation &violation) {
  _compilerErrorSet->addViolation(violation);
}

void ResultCollector::addWarning(const Violation &violation) {
  _compilerWarningSet->addViolation(violation);
}

ViolationSet *ResultCollector::getCompilerErrorSet() const {
  return _compilerErrorSet.get();
}

ViolationSet *ResultCollector::getCompilerWarningSet() const {
  return _compilerWarningSet.get();
}

} // namespace astooling
