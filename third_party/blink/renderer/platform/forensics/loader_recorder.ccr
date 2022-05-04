/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/j89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#include "third_party/blink/renderer/platform/forensics/loader_recorder.h"
#include "third_party/blink/renderer/platform/network/http_header_map.h"

#include "base/bind.h"
#include "base/process/process.h"
#include "base/threading/platform_thread.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_event.h"
#include "third_party/blink/renderer/platform/forensics/forensic_file_logger.h"
#include "third_party/blink/renderer/platform/json/json_values.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_request.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_response.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"

namespace blink {
bool LoaderRecorder::recording_ = true;

scoped_refptr<base::SequencedTaskRunner>
LoaderRecorder::getSequencedTaskRunner() {
  return ForensicFileLogger::GetInstance()->GetOrCreateFileThread();
}

void LoaderRecorder::LogInfo(const std::string& log_str) {
  ForensicFileLogger::GetInstance()->log(
      std::to_string(base::Time::Now().ToDoubleT()) + " || " + log_str);
  // VLOG(7) << log_str;
}

void LoaderRecorder::LogInfo(const std::string& log_str, double timestamp) {
  ForensicFileLogger::GetInstance()->log(
      std::to_string(base::Process::Current().Pid()) + ":" +
      std::to_string(base::PlatformThread::CurrentId()) + " || " +
      std::to_string(timestamp) + " || " + log_str);
  // VLOG(7) << log_str;
}

void LoaderRecorder::DidAppendData(const char* data,
                                   size_t length,
                                   Resource* resource) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  AtomicString initiator_name = resource->Options().initiator_info.name;
  DCHECK(resource->GetResourceRequest().UseStreamOnResponse() ||
         initiator_name == "xmlhttprequest");

  String resource_type =
      Resource::ResourceTypeToString(resource->GetType(), initiator_name);

  getSequencedTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &LoaderRecorder::DidFinishLoadingResourceInternal,
          String(data, length),
          copyRequest(resource->GetResourceRequest(), resource_type),
          copyResponse(resource->GetResponse()),
          resource->GetResourceRequest().HttpHeaderFields().CopyData(),
          resource->GetResponse().HttpHeaderFields().CopyData(),
          GetThreadData()));
}

void LoaderRecorder::DidFinishLoadingResource(Resource* resource) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  // we take care of stream response in DidAppendData
  AtomicString initiator_name = resource->Options().initiator_info.name;

  if (resource->GetResourceRequest().UseStreamOnResponse() ||
      initiator_name == "xmlhttprequest")  // these are handled in appendData
    return;
  String resource_type =
      Resource::ResourceTypeToString(resource->GetType(), initiator_name);

  scoped_refptr<const SharedBuffer> buffer = resource->ResourceBuffer();
  StringBuilder data;
  if (buffer) {
    for (const auto& span : *buffer) {
      data.Append(span.data(), span.size());
    }
  }

  getSequencedTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &LoaderRecorder::DidFinishLoadingResourceInternal,
          data.ToString().IsolatedCopy(),
          copyRequest(resource->GetResourceRequest(), resource_type),
          copyResponse(resource->GetResponse()),
          resource->GetResourceRequest().HttpHeaderFields().CopyData(),
          resource->GetResponse().HttpHeaderFields().CopyData(),
          GetThreadData()));
}

void LoaderRecorder::DidFinishLoadingResourceInternal(
    const String& data,
    std::unique_ptr<RequestData> request,
    std::unique_ptr<ResponseData> response,
    std::unique_ptr<CrossThreadHTTPHeaderMapData> request_headers,
    std::unique_ptr<CrossThreadHTTPHeaderMapData> response_headers,
    std::unique_ptr<ThreadData> thread_data) {
  auto message = std::make_unique<JSONObject>();
  HTTPHeaderMap req_headers;
  req_headers.Adopt(std::move(request_headers));

  HTTPHeaderMap res_headers;
  res_headers.Adopt(std::move(response_headers));

  AddRequestInfo(std::move(request), message);
  AddResponseInfo(std::move(response), message);
  AddResponseData(data, message);
  AddHeaderFields(req_headers, message, "requestHeader");
  AddHeaderFields(res_headers, message, "responseHeader");
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void LoaderRecorder::AddThreadData(std::unique_ptr<ThreadData>& thread_data,
                                   std::unique_ptr<JSONObject>& message) {
  _IFA_DISABLE_
  message->SetInteger("tid", thread_data->tid);
  message->SetInteger("pid", thread_data->pid);
  message->SetString("threadName", thread_data->name);
}

void LoaderRecorder::AddHeaderFields(const HTTPHeaderMap& http_header_map,
                                     std::unique_ptr<JSONObject>& message,
                                     const AtomicString& header) {
  _IFA_DISABLE_
  auto header_json = std::make_unique<JSONObject>();
  for (auto it : http_header_map) {
    header_json->SetString(it.key, it.value);
  }
  message->SetString(header, header_json->ToJSONString());
}

void LoaderRecorder::AddRequestInfo(std::unique_ptr<RequestData> request,
                                    std::unique_ptr<JSONObject>& message) {
  _IFA_DISABLE_

  message->SetString("httpMethod", request->httpMethod);
  message->SetString("resourceType", request->resourceType);
  message->SetBoolean("useStreamOnResponse", request->useStreamOnResponse);
  message->SetString("httpOrigin", request->httpOrigin);
  message->SetString("httpContentType", request->httpContentType);
  message->SetString("referrerString", request->referrerString);
  message->SetString("requestUrl", request->requestUrl);
  message->SetInteger("requestorID", request->requestorID);
  message->SetInteger("inspectrorID", request->inspectorID);
  message->SetString("requestBody", "");
  // TODO (): add request body
}

void LoaderRecorder::AddResponseInfo(std::unique_ptr<ResponseData> response,
                                     std::unique_ptr<JSONObject>& message) {
  _IFA_DISABLE_
  message->SetString("responseUrl", response->responseUrl);
  message->SetString("responseMimeType", response->responseMimeType);
  message->SetString("contentType", response->contentType);
  message->SetBoolean("isServiceWorkerPassThrough",
                      response->isServiceWorkerPassThrough);
  message->SetInteger("httpStatusCode", response->httpStatusCode);
  message->SetInteger("fetchResponseType", response->fetchResponseType);
}

void LoaderRecorder::AddResponseData(const String& data,
                                     std::unique_ptr<JSONObject>& message) {
  _IFA_DISABLE_
  message->SetString("responseBody", data);
}

std::unique_ptr<RequestData> LoaderRecorder::copyRequest(
    const ResourceRequestHead& request,
    const String& resource_type) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  std::unique_ptr<RequestData> req = std::make_unique<RequestData>();

  req->httpMethod = request.HttpMethod().GetString().IsolatedCopy();
  req->resourceType = resource_type.IsolatedCopy();
  req->httpOrigin = request.HttpOrigin().GetString().IsolatedCopy();
  req->httpContentType = request.HttpContentType().GetString().IsolatedCopy();
  req->referrerString = request.ReferrerString().IsolatedCopy();
  req->requestUrl = request.Url()
                        .GetString()
                        .IsolatedCopy();  // needed as KURL is not thread safe
  req->useStreamOnResponse = request.UseStreamOnResponse();
  req->requestorID = request.RequestorID();
  req->inspectorID = request.InspectorId();

  return req;
}

std::unique_ptr<ResponseData> LoaderRecorder::copyResponse(
    const ResourceResponse& response) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  std::unique_ptr<ResponseData> res = std::make_unique<ResponseData>();
  res->responseUrl = response.ResponseUrl().GetString().IsolatedCopy();
  res->responseMimeType = response.MimeType().GetString().IsolatedCopy();
  res->contentType = response.HttpContentType().GetString().IsolatedCopy();
  res->isServiceWorkerPassThrough = response.IsServiceWorkerPassThrough();
  res->httpStatusCode = response.HttpStatusCode();
  res->fetchResponseType = static_cast<int>(response.GetType());
  return res;
}

std::unique_ptr<ThreadData> LoaderRecorder::GetThreadData() {
  std::unique_ptr<ThreadData> res = std::make_unique<ThreadData>();
  res->timestamp = base::Time::Now().ToDoubleT();
  res->tid = base::PlatformThread::CurrentId();
  res->name = base::PlatformThread::GetName();
  res->pid = base::Process::Current().Pid();
  return res;
}

}  // namespace blink
