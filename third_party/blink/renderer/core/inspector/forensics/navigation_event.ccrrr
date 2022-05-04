/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/jallen89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#include "third_party/blink/renderer/core/inspector/forensics/navigation_event.h"

namespace blink {

NavigationEvent::NavigationEvent() = default;

NavigationEvent::NavigationEvent(
        std::unique_ptr<protocol::Forensics::NavigationEvent> event) {
  url_ = KURL(event->getUrl());
  log_event_ = std::move(event);
}

String NavigationEvent::ToString() {
  String out =
          "FrameId: " + FrameId()
          + "URL: " + log_event_->getUrl();
  return out;
}


}
