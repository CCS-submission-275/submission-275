#include "third_party/blink/renderer/core/inspector/forensics/animation_frame_event.h"

namespace blink {


AnimationFrameEvent::AnimationFrameEvent(
    std::unique_ptr<protocol::Forensics::AnimationFrameEvent> event) {
  log_event_ = std::move(event);
}

String AnimationFrameEvent::ToString() {
  String out =
      "FrameID: " + FrameId()
      + "CallbackId: " + String::Number(CallbackId());
  return out;
}



} // blink
