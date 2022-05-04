//
// Created by  on 12/7/20.
//

#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include <string>

namespace blink {

JSEvent::JSEvent(std::unique_ptr<protocol::Forensics::RecordedEvent> event) {
  // Store the event so we can access it later.
  log_event = std::move(event);
}

base::TimeDelta JSEvent::TimeDelta() {
  double time_delta = log_event->getTimeDelta();
  return base::TimeDelta::FromMillisecondsD(time_delta);
}

WTF::String JSEvent::ToString() {
  WTF::String out;
  out = "FrameID: " + FrameId()
        + " Interface: " + Interface()
        + " Event Type: " + EventType()
        + " Target: " + String::Number(TargetId())
        + " Seq. Number: " + String::Number(SequenceNumber());
  return out;
}

} // blink
