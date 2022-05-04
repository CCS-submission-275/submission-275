#include "third_party/blink/renderer/core/inspector/forensics/dom_attr_event.h"

namespace blink {

DOMAttrEvent::DOMAttrEvent(std::unique_ptr<protocol::Forensics::DOMAttrEvent> event) {
  // Store the event so we can access it later.
  log_event_ = std::move(event);
  node_id_ = log_event_->getNodeId();
}

ForensicDOMAttrEventType DOMAttrEvent::GetEventAttrType() {

  String type = log_event_->getEventType();
  if (type == "DOMModifyAttr")
    return ForensicDOMAttrEventType::kModifyAttr;
  else if (type == "DOMRemoveAttr")
    return ForensicDOMAttrEventType::kRemoveAttr;
  else
    return ForensicDOMAttrEventType::kInvalidAttr;

}

String DOMAttrEvent::ToString() {return FrameId();}

} // blink