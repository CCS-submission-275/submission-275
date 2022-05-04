#include "third_party/blink/renderer/core/inspector/forensics/storage_event.h"

namespace blink {

ForensicStorageEvent::ForensicStorageEvent(
    std::unique_ptr<protocol::Forensics::StorageEvent> event) {
  log_event_ = std::move(event);
}

String ForensicStorageEvent::ToString() {
  WTF::String out;
  out = "Frame Id: " + FrameId()
        + " API: " + ApiName()
        + " Type: " + Type()
        + " Value: " + Value()
        + " Key: " + Key()
        + " Index: " + String::Number(Index());
  return out;
}

}