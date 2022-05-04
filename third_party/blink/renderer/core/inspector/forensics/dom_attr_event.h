#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_ATTR_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_ATTR_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_attr_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"

namespace blink {

enum ForensicDOMAttrEventType {
    kRemoveAttr,
    kModifyAttr,
    kInvalidAttr,
};

class DOMAttrEvent final : public ForensicEvent {

  public:
    WTF::String FrameId() {return log_event_->getFrameId();}
    WTF::String GetEventType() {return log_event_->getEventType();}
    WTF::String GetAttrName() { return log_event_->getAttrName();}
    WTF::String GetOldValue() { return log_event_->getOldValue("Invalid");}
    WTF::String GetNewValue() { return log_event_->getNewValue("Invalid");}
    WTF::String ToString() override;
    ForensicDOMAttrEventType GetEventAttrType();
    DOMAttrEvent(std::unique_ptr<protocol::Forensics::DOMAttrEvent> event);
  private:
    // The "audit" log related to this event.
    std::unique_ptr<protocol::Forensics::DOMAttrEvent> log_event_;
    int node_id_;
};

} //blink namespace

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_ATTR_EVENT_H_
