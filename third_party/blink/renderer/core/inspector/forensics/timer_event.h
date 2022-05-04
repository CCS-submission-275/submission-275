#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_EVENT_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"


namespace blink {

class TimerEvent final : public ForensicEvent {

  public:
    TimerEvent(std::unique_ptr<protocol::Forensics::TimerEvent> event);
    WTF::String FrameId() {return log_event_->getFrameId();}
    int SequenceNumber(){return log_event_->getSequenceNumber();}
    int TimerId(){return log_event_->getTimerId();}
    base::TimeDelta TimeDelta();
    String ToString() override;
  private:
    std::unique_ptr<protocol::Forensics::TimerEvent> log_event_;

};

} // blink


#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_EVENT_H_
