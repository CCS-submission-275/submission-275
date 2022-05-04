#include "third_party/blink/renderer/core/inspector/forensics/idle_callback_event.h"

namespace blink {

IdleCallbackEvent::IdleCallbackEvent(
        std::unique_ptr<protocol::Forensics::IdleCallbackEvent> event) {
  log_event_ = std::move(event);
}

String IdleCallbackEvent::ToString() {
  String out =
          "FrameID: " + FrameId()
          + "CallbackId: " + String::Number(CallbackId())
          + "CallbackType: " + String::Number(CallbackType());
  return out;
}

} //blink