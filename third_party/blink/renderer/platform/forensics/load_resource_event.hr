/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/j89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOAD_RESOURCE_EVENT_H
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOAD_RESOURCE_EVENT_H

#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class PLATFORM_EXPORT LoadResourceEvent {
 USING_FAST_MALLOC(LoadResourceEvent);
 public:

  LoadResourceEvent();

  explicit LoadResourceEvent(
      std::unique_ptr<protocol::Forensics::LoadResourceEvent> event);

  KURL Url() { return KURL(log_event_->getRequestUrl()); }
  String ResponseBody() { return log_event_->getResponseBody(); }
  String HttpMethod() { return log_event_->getHttpMethod(); }
  String ContentType() { return log_event_->getHttpContentType(); }
  String ResourceType() { return log_event_->getResourceType(); }
  String ResponseMimeType() { return log_event_->getResponseMimeType(); }
  int FetchResponseType() { return log_event_->getFetchResponseType(); }
  bool IsServiceWorkerPassThrough() { return log_event_->getIsServiceWorkerPassThrough(); }
  int StatusCode() { return log_event_->getHttpStatusCode(); }
  String ResponseHeaders() { return log_event_->getResponseHeader(); }
  String Identifier();

 private:
  std::unique_ptr<protocol::Forensics::LoadResourceEvent> log_event_;

 friend class ResourceDataStore;
};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOAD_RESOURCE_EVENT_H
