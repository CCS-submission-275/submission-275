//
// Created by  on 5/12/21.
//

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class ForensicStorageEvent final : public ForensicEvent {

  public:
    ForensicStorageEvent(std::unique_ptr<protocol::Forensics::StorageEvent> event);
    String ApiName() {return log_event_->getApiName();}
    String FrameId() {return log_event_->getFrameId();}
    String Type() {return log_event_->getType();}
    String Value() {return log_event_->getValue("NA");}
    String Key() {return log_event_->getKey();}
    bool IsNull() {return log_event_->getIsNull(false);}
    int Index() {return log_event_->getIndex(-1);}
    String ToString() override;
  private:
    std::unique_ptr<protocol::Forensics::StorageEvent> log_event_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_EVENT_H_
