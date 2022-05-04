#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_XML_HTTP_REQUEST_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_XML_HTTP_REQUEST_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/xmlhttprequest/xml_http_request.h"

namespace blink {

class XMLHttpRequestProgressEventThrottle;

class ReplayXMLHttpRequest : public GarbageCollected<ReplayXMLHttpRequest>{

  public:
    void Trace(Visitor* visitor);
    void BindTarget(XMLHttpRequest* req) {req_ = req;}
    // Synchronously executes the callback registered with SetTimeout or SetInterval.
    void Dispatch(Event* event);

  private:
    Member<XMLHttpRequest>  req_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_DOM_TIMER_H_
