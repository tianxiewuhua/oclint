#ifndef ASTOOLING_COMPILERINSTANCE_H
#define ASTOOLING_COMPILERINSTANCE_H

#include <clang/Frontend/CompilerInstance.h>
#include <memory>
#include <vector>

namespace astooling {

class CompilerInstance : public clang::CompilerInstance {

public:
  void start();
  void end();

private:
  void setupTarget();

  std::vector<std::unique_ptr<clang::FrontendAction>> _actions;
};
} // namespace astooling

#endif
