#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_JS_EVENT_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_JS_EVENT_H

#include "base/time/time.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class EventTarget;
class EventReplayer;


class JSEvent : public ForensicEvent {
  friend EventReplayer;
  public:
    JSEvent(std::unique_ptr<protocol::Forensics::RecordedEvent> event);
    virtual ~JSEvent(){}
    //virtual void FireEvent(EventTarget* target);

    WTF::String FrameId() const { return log_event->getFrameId();}
    int TargetId() const { return log_event->getTargetId();}
    WTF::String EventType() const { return log_event->getJsEventType();}
    WTF::String Interface() const { return log_event->getInterfaceName();}
    WTF::String EventTargetInterface() const {return log_event->getEventTargetInterfaceName();}
    base::TimeDelta TimeDelta();
    int SequenceNumber(){return log_event->getSequenceNumber();}

    WTF::String ToString() override;

  protected:
    std::unique_ptr<protocol::Forensics::RecordedEvent> log_event;

};

}

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_JS_EVENT_H
