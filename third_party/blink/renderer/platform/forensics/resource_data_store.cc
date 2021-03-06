//
// Created by   on 9/30/20.
//

#include "third_party/blink/renderer/platform/forensics/resource_data_store.h"
#include "base/single_thread_task_runner.h"
#include "third_party/blink/renderer/platform/forensics/load_resource_event.h"
#include "third_party/blink/renderer/platform/forensics/replaying_bytes_consumer.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/json/json_parser.h"
#include "third_party/blink/renderer/platform/json/json_values.h"
#include "third_party/blink/renderer/platform/loader/fetch/response_body_loader.h"
#include "third_party/blink/renderer/platform/wtf/text/unicode.h"

namespace blink {

// static
ResourceDataStore* ResourceDataStore::GetInstance() {
  return base::Singleton<ResourceDataStore>::get();
}

ResourceDataStore::ResourceDataStore() {
  replaying_ = false;
}
ResourceDataStore::~ResourceDataStore() = default;

String ResourceDataStore::GetRequestIdentifier(
    const ResourceRequestHead& request,
    const String& resourceType) {
  std::ostringstream str;
  str << request.Url().GetString();
  str << resourceType;
  str << request.UseStreamOnResponse()
      << (request.HttpOrigin() ? request.HttpOrigin() : "");
  // str << (request.ReferrerString() ? request.ReferrerString() : "");
  str << request.HttpMethod().GetString();
  // not adding headers for now.  thinks it is needed in future.
  //  auto header_json = std::make_unique<JSONObject>();
  //  for (auto it: request.HttpHeaderFields()) {
  //    header_json->SetString(it.key, it.value);
  //  }
  //  str << header_json->ToJSONString();
  return String(str.str().c_str());
}

void ResourceDataStore::LogRequestIdentifier(Resource* resource, const String& key) {
  AtomicString initiator_name = resource->Options().initiator_info.name;
  String resourceType =
      Resource::ResourceTypeToString(resource->GetType(), initiator_name);
  const ResourceRequestHead& request = resource->GetResourceRequest();

  VLOG(7) << "[" << key << "]: " << request.Url().GetString() << ","
          << resourceType << "," << request.UseStreamOnResponse() << ","
          << (request.HttpOrigin() ? request.HttpOrigin() : "")
          << ","
          //<< (request.ReferrerString() ? request.ReferrerString() : "") << ","
          << request.HttpMethod().GetString();
}

void ResourceDataStore::insertLoadResourceEvent(LoadResourceEvent* event) {
  // TODO: Check key is unique or not, may need add more property into key
  VLOG(8) << __func__
          << " Insert loadResource event for :" << event->Identifier() << ":";
  resource_map_.insert(event->Identifier(), event);
}


void ResourceDataStore::LogResourceMap() {
  for (auto it : resource_map_) {
    auto* event = it.value;
    VLOG(7) << __func__
            << "[ResourceMapEntry]: " << event->log_event_->getRequestUrl()
            << "," << event->log_event_->getResourceType() << ","
            << event->log_event_->getUseStreamOnResponse() << ","
            << event->log_event_->getHttpOrigin()
            << ","
            //<< event->log_event_->getReferrerString() << ","
            << event->log_event_->getHttpMethod();
  }
}

String ResourceDataStore::GetRequestIdentifier(Resource* resource) {
  AtomicString initiator_name = resource->Options().initiator_info.name;
  String resourceType =
      Resource::ResourceTypeToString(resource->GetType(), initiator_name);
  const ResourceRequestHead& request = resource->GetResourceRequest();
  return GetRequestIdentifier(request, resourceType);
}

LoadResourceEvent* ResourceDataStore::GetLoadResourceEvent(const String& id) {
  return resource_map_.at(id);
}

void ResourceDataStore::ReconstructResponse(LoadResourceEvent* event,
                                            ResourceResponse& response) {
  response.SetMimeType(AtomicString(event->ContentType()));
  response.SetHttpStatusCode(event->StatusCode());
  response.SetMimeType(AtomicString(event->ResponseMimeType()));
  response.SetWasFetchedViaServiceWorker(event->IsServiceWorkerPassThrough());
  response.SetType(static_cast<network::mojom::FetchResponseType>(
      event->FetchResponseType()));
  std::unique_ptr<JSONValue> value = ParseJSON(event->ResponseHeaders());
  JSONObject* headers = JSONObject::Cast(value.get());

  for (wtf_size_t i = 0; i < headers->size(); ++i) {
    JSONObject::Entry header = headers->at(i);
    String headerValue;
    header.second->AsString(&headerValue);
    response.SetHttpHeaderField(AtomicString(header.first),
                                AtomicString(headerValue));
  }
}

void ResourceDataStore::DispatchLoadFinish(const String& identifier) {
  //  VLOG(7) << __func__ << " start to load " << identifier;
  if (loaded_resource_map_.Contains(identifier)) {
    VLOG(7) << __func__ << " Send signal to finish loading for " << identifier;
    auto resourcePair = loaded_resource_map_.at(identifier);
    Resource* resource = resourcePair.first;
    auto* event = GetLoadResourceEvent(identifier);
    if (resource->GetResourceRequest().UseStreamOnResponse()) {
      ReplayStreamResponseInternal(resource, event, resourcePair.second);
    } else {
      ReplayResponseInternal(resource, event, resourcePair.second);
    }
    loaded_resource_map_.erase(identifier);

    if (back_log_.Contains(identifier)) {
      back_log_.Set(identifier, true);
    }

  } else {
    // we put this into a backlog and revisit it later.
    VLOG(7) << __func__ << " node is not inserted yet, waiting for "
            << identifier;
    back_log_.insert(identifier, false);
  }
}

void ResourceDataStore::ReplayResponseInternal(
    Resource* resource,
    LoadResourceEvent* event,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  //  if (resource->GetType() == ResourceType::kImage)
  //    resource->NotifyStartLoad();
  ResourceResponse response(event->Url());
  ReconstructResponse(event, response);
  resource->ResponseReceived(response);

  if (event->ResponseBody().length()) {
    resource->AppendData(event->ResponseBody().Latin1().c_str(),
                         event->ResponseBody().length());
    resource->Finish(base::TimeTicks::Now(), task_runner.get());
  } else if (event->ResourceType() == "Video") {
    // we skip video's resoure and append emtpy data to it
    resource->FinishAsError(ResourceError::TimeoutError(event->Url()),
                            task_runner.get());
  }
}

bool ResourceDataStore::ReplayResponse(
    Resource* resource,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  String request_identifier = GetRequestIdentifier(resource);
  LoadResourceEvent* event = GetLoadResourceEvent(request_identifier);

  if (!event) {
    LogRequestIdentifier(resource);
    LogResourceMap();
    return false;
  }
  //  VLOG(7) << __func__ << " postpone load event for " << request_identifier;
  loaded_resource_map_.insert(
      request_identifier,
      std::pair<Resource*, scoped_refptr<base::SingleThreadTaskRunner>>(
          resource, task_runner));
  resource->NotifyStartLoad();
  // process the backlog if the resource
  if (back_log_.Contains(request_identifier)) {
    VLOG(7) << __func__ << " response has been loaded for "
            << request_identifier;
    DispatchLoadFinish(request_identifier);
  }
  LogRequestIdentifier(resource, "ResourceHit");

  return true;
}

void ResourceDataStore::ReplayStreamResponseInternal(
    Resource* resource,
    LoadResourceEvent* event,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  auto* bytes_consumer =
      MakeGarbageCollected<ReplayingBytesConsumer>(task_runner);
  //  responseBody = {\\n    \\\"name\\\": \\\"replay\\\",\\n    \\\"value\\\":
  //  \\\"modified\\\"\\n}\"}
  String decoded;
  std::unique_ptr<JSONValue> value =
      ParseJSON(JSONValue::QuoteString(event->ResponseBody()));
  value->AsString(&decoded);
  //  VLOG(7) << __func__ << " decoded string: " << decoded;
  bytes_consumer->Add(ReplayingBytesConsumer::Command(
      ReplayingBytesConsumer::Command::kData, decoded.Latin1().c_str(),
      decoded.length()));
  bytes_consumer->Add(
      ReplayingBytesConsumer::Command(ReplayingBytesConsumer::Command::kDone));
  ResourceResponse response(event->Url());
  ReconstructResponse(event, response);
  resource->ResponseReceived(response);
  ResponseBodyLoader* body_loader = MakeGarbageCollected<ResponseBodyLoader>(
      *bytes_consumer, *MakeGarbageCollected<NoopResponseBodyLoaderClient>(),
      task_runner.get());
  resource->ResponseBodyReceived(*body_loader, task_runner);
  resource->Finish(base::TimeTicks::Now(), task_runner.get());
}

}  // namespace blink
