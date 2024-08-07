#include "ResultCollector.h"
#include "Violation.h"
#include <vector>
namespace astooling {

class ViolationSet {
private:
  std::vector<Violation> _violations;

public:
  void addViolation(const Violation &violation);
  int numberOfViolations() const;
  const std::vector<Violation> &getViolations() const;

  bool operator==(const ViolationSet &rhs) const;
};
} // namespace astooling
