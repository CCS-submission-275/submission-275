#include "third_party/blink/renderer/core/inspector/forensics/script_execution_event.h"

namespace blink {

ScriptExecutionEvent::ScriptExecutionEvent(
        std::unique_ptr<protocol::Forensics::ScriptExecutionEvent> event){
  log_event_ = std::move(event);
}

WTF::String ScriptExecutionEvent::ToString() {
  WTF::String out = "Frame Id: " + FrameId()
          + "Script Id: " + ScriptId()
          + "Seq Number: " + String::Number(SequenceNumber())
          + "hash: " + String::Number(Hash())
          + "url: " + URL();
  return out;
}


} // blink