#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_PERFORMANCE_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_PERFORMANCE_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class PerformanceEvent final : public ForensicEvent {

  public:
    PerformanceEvent(std::unique_ptr<protocol::Forensics::PerformanceEvent> event){log_event_ = std::move(event);}
    WTF::String FrameId() {return log_event_->getFrameId();}
    double now() {return log_event_->getNow();}
    WTF::String ToString() override {
      return "FrameId: " + FrameId() + " now: " + String::Number(now());
    }
  private:
    std::unique_ptr<protocol::Forensics::PerformanceEvent> log_event_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_PERFORMANCE_EVENT_H_