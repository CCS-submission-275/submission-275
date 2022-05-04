#include "third_party/blink/renderer/core/inspector/forensics/replay_xml_http_request.h"
#include "third_party/blink/renderer/core/xmlhttprequest/xml_http_request_progress_event_throttle.h"

namespace blink {

void ReplayXMLHttpRequest::Trace(Visitor* visitor) {
  visitor->Trace(req_);
}

void ReplayXMLHttpRequest::Dispatch(Event* event) {
  req_->DispatchEvent(*event);
}



} // blink
