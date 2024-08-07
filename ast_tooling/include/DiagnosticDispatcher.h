#ifndef ASTOOLING_DIAGNOSTICDISPATCHER_H
#define ASTOOLING_DIAGNOSTICDISPATCHER_H

#include <clang/Basic/Diagnostic.h>

namespace astooling {

class DiagnosticDispatcher : public clang::DiagnosticConsumer {
public:
  void HandleDiagnostic(clang::DiagnosticsEngine::Level diagnosticLevel,
                        const clang::Diagnostic& diagnosticInfo) override;
};

}

#endif
