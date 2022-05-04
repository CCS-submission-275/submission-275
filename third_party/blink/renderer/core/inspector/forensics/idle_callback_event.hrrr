#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_IDLE_CALLBACK_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_IDLE_CALLBACK_EVENT_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"


namespace blink {
    
class IdleCallbackEvent final : public ForensicEvent {

  public:
    IdleCallbackEvent(
      std::unique_ptr<protocol::Forensics::IdleCallbackEvent> event);
    String FrameId() {return log_event_->getFrameId();}
    int CallbackId() {return log_event_->getCallbackId();}
    int CallbackType() {return log_event_->getCallbackType();}
    double AllottedTime() { return log_event_->getAllottedTime();}
    base::TimeDelta TimeDelta() {
      return base::TimeDelta::FromMillisecondsD(log_event_->getTimeDelta());}
    String ToString() override;
    
  private:
    std::unique_ptr<protocol::Forensics::IdleCallbackEvent> log_event_;
};


} // blink


#endif //CHROMIUM_IDLE_CALLBACK_EVENT_H
