#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_CRYPTO_RANDOM_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_CRYPTO_RANDOM_EVENT_H_

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class CryptoRandomEvent final : public ForensicEvent {

  public:
    explicit CryptoRandomEvent(std::unique_ptr<protocol::Forensics::CryptoRandomEvent> event){log_event_ = std::move(event);}
    WTF::String FrameId() {return log_event_->getFrameId();}
    WTF::String Values() {return log_event_->getValues();}
    WTF::String Url() {return log_event_->getUrl();}
    WTF::String ToString() override;

private:
    std::unique_ptr<protocol::Forensics::CryptoRandomEvent> log_event_;

};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_GETTER_EVENT_H_
