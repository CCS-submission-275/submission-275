#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"

namespace blink {

TimerEvent::TimerEvent(std::unique_ptr<protocol::Forensics::TimerEvent> event) {
  log_event_ = std::move(event);
}

String TimerEvent::ToString() {
  WTF::String out;
  out = "Frame Id: " + FrameId()
      + "TimerId: " + String::Number(TimerId())
      + "Seq Number: " + String::Number(SequenceNumber());
  return out;
}

base::TimeDelta TimerEvent::TimeDelta() {
  double time_delta = log_event_->getTimeDelta();
  return base::TimeDelta::FromMillisecondsD(time_delta);
}

} // blink