/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/j89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOADER_RECORDER_H
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOADER_RECORDER_H

#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/wtf/allocator/allocator.h"
#include "third_party/blink/renderer/platform/heap/heap.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource.h"

#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"


#define _IFA_DISABLE_  if (!record()) return;
#define _IFAC_ "LOG::Forensics"
#define _IFAF_ __func__
#define _IFA_LOG_PREFIX_ _IFAC_ << "::" << _IFAF_ << " :"

namespace blink {

class ResourceRequest;

class ResourceResponse;

class ResourceRequestHead;

class Resource;

class HTTPHeaderMap;

class JSONObject;

struct RequestData {
  String httpMethod;
  String resourceType;
  String httpOrigin;
  String httpContentType;
  String referrerString;
  String requestUrl;
  bool useStreamOnResponse;
  int requestorID;
  int inspectorID;
  String timestamp;
};

struct ResponseData {
  String responseUrl;
  String responseMimeType;
  String contentType;
  bool isServiceWorkerPassThrough;
  int httpStatusCode;
  int fetchResponseType;
};



class PLATFORM_EXPORT LoaderRecorder : public GarbageCollected<LoaderRecorder> {
  DISALLOW_COPY(LoaderRecorder);

 public:
  static void DidFinishLoadingResource(Resource* resource);
  static void DidAppendData(const char* data,
                            size_t length,
                            Resource* resource);
  static bool record() { return recording_; }
  static void setRecord(bool v) { recording_ = v; }

 private:

  static std::unique_ptr<RequestData> copyRequest(const ResourceRequestHead& request,
                                                  const String& resource_type);
  static std::unique_ptr<ResponseData> copyResponse(const ResourceResponse& request);

  static void LogInfo(std::string const& log_str);
  static void LogInfo(const std::string& log_str, double timestamp);

  static void DidFinishLoadingResourceInternal(const String& data,
                                               std::unique_ptr<RequestData> request,
                                               std::unique_ptr<ResponseData> response,
                                               std::unique_ptr<
                                                   CrossThreadHTTPHeaderMapData> request_headers,
                                               std::unique_ptr<
                                                   CrossThreadHTTPHeaderMapData> response_headers,
                                               std::unique_ptr<ThreadData> thread_data);

  static void AddHeaderFields(const HTTPHeaderMap& http_header_map,
                              std::unique_ptr<JSONObject>& message,
                              const AtomicString& header);
  static void AddRequestInfo(std::unique_ptr<RequestData> request,
                             std::unique_ptr<JSONObject>& message);
  static void AddResponseInfo(std::unique_ptr<ResponseData> response,
                              std::unique_ptr<JSONObject>& message);
  static void AddResponseData(const String& data,
                              std::unique_ptr<JSONObject>& message);
  static void AddThreadData(std::unique_ptr<ThreadData>& thread_data,
                            std::unique_ptr<JSONObject>& message);

  static scoped_refptr<base::SequencedTaskRunner> getSequencedTaskRunner();

  static std::unique_ptr<ThreadData> GetThreadData();

  static bool recording_;
};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_LOADER_RECORDER_H