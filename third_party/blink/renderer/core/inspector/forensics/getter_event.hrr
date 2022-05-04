#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_GETTER_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_GETTER_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class GetterEvent final : public ForensicEvent {

  public:
    GetterEvent(std::unique_ptr<protocol::Forensics::GetterEvent> event){log_event_ = std::move(event);}
    WTF::String FrameId() {return log_event_->getFrameId();}
    WTF::String ReturnValue() {return log_event_->getReturnValue();}
    WTF::String ReturnType() {return log_event_->getReturnType();}
    WTF::String Interface() {return log_event_->getInterface();}
    WTF::String Attribute() {return log_event_->getAttribute();}
    WTF::String ToString() override;

private:
    std::unique_ptr<protocol::Forensics::GetterEvent> log_event_;

};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_GETTER_EVENT_H_
