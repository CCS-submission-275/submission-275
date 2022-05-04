#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_ANIMATION_FRAME_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_ANIMATION_FRAME_EVENT_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class AnimationFrameEvent final : public ForensicEvent {

public:
  AnimationFrameEvent(
      std::unique_ptr<protocol::Forensics::AnimationFrameEvent> event);
  String FrameId() {return log_event_->getFrameId();}
  int CallbackId() {return log_event_->getCallbackId();}
  double HighResNowMsLegacy() {return log_event_->getHighResNowMsLegacy();}
  double HighResNowMs(){return log_event_->getHighResNowMs();}
  bool isLegacyCall() {return log_event_->getIsLegacyCall();}
  base::TimeDelta TimeDelta() {
    return base::TimeDelta::FromMillisecondsD(log_event_->getTimeDelta());}
  String ToString() override;

private:
    std::unique_ptr<protocol::Forensics::AnimationFrameEvent> log_event_;

};

} // blink


#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_ANIMATION_FRAME_EVENT_H
