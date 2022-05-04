#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_EXECUTION_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_EXECUTION_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class ScriptExecutionEvent final : public ForensicEvent {

public:
    ScriptExecutionEvent(std::unique_ptr<protocol::Forensics::ScriptExecutionEvent> event);
    int SequenceNumber(){return log_event_->getSequenceNumber();}
    int Hash(){return log_event_->getHash();}
    WTF::String FrameId() {return log_event_->getFrameId();}
    WTF::String ScriptId() {return log_event_->getScriptId();}
    WTF::String URL() {return log_event_->getUrl();}
    WTF::String ToString() override;

private:
    std::unique_ptr<protocol::Forensics::ScriptExecutionEvent> log_event_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_EXECUTION_EVENT_H_
