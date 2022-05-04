/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/jallen89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_NAVIGATION_EVENT_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_NAVIGATION_EVENT_H

#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"

namespace blink {

class NavigationEvent final : public ForensicEvent {

public:

  NavigationEvent();

  explicit NavigationEvent(std::unique_ptr<protocol::Forensics::NavigationEvent> event);

  KURL Url() { return url_; }
  String FrameId() {return log_event_->getFrameId();}
  String ToString() override;

private:
  KURL url_;
  std::unique_ptr<protocol::Forensics::NavigationEvent> log_event_;
};
}

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_NAVIGATION_EVENT_H
