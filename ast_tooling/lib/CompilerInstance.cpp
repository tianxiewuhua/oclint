#include "CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/FrontendOptions.h"
#include <memory>


using namespace astooling;


static clang::FrontendAction *getFrontendAction() {
  return new clang::SyntaxOnlyAction();
}

void CompilerInstance::setupTarget() {
  getTarget().adjust(getDiagnostics(), getLangOpts());

  if (clang::TargetInfo *auxTargetInfo = getAuxTarget()) {
    getTarget().setAuxTarget(auxTargetInfo);
  }
}

void CompilerInstance::start() {
  // TODO assert

  setTarget(clang::TargetInfo::CreateTargetInfo(getDiagnostics(), getInvocation().TargetOpts));
  if (!hasTarget()) {
    return;
  }

  for (clang::FrontendInputFile input : getFrontendOpts().Inputs) {
    if (hasSourceManager()) {
      getSourceManager().clearIDTables();
    }

    clang::FrontendAction *frontendAction = getFrontendAction();
    if (frontendAction->BeginSourceFile(*this, input)) {
      // TODO: handle Execute error?
      static_cast<void>(frontendAction->Execute());
      _actions.emplace_back(frontendAction);
    }
  }
}

void CompilerInstance::end() {
  for (const std::unique_ptr<clang::FrontendAction> &action : _actions) {
    action->EndSourceFile(); 
  }

  getDiagnostics().getClient()->finish();
}
