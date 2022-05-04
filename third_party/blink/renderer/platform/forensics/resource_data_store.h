#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_RESOURCE_DATA_STORE_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_RESOURCE_DATA_STORE_H_

#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"
#include "base/memory/singleton.h"
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource.h"
#include "third_party/blink/renderer/platform/loader/fetch/response_body_loader_client.h"

namespace blink {

class LoadResourceEvent;

class PLATFORM_EXPORT ResourceDataStore {
 USING_FAST_MALLOC(ResourceDataStore);

 public:
  static ResourceDataStore* GetInstance();

  void insertLoadResourceEvent(LoadResourceEvent* event);

  LoadResourceEvent* GetLoadResourceEvent(const String& id);
  bool ReplayResponse(Resource* resource,
                      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  bool replaying() { return replaying_; }
  void DispatchLoadFinish(const String& identifier);
  void setReplay(bool v) { replaying_ = v; }
  void PrintIdentifiers();

  // Dumps resource map to VLOG(7).
  void LogResourceMap();

 private:
  class NoopResponseBodyLoaderClient
      : public GarbageCollected<NoopResponseBodyLoaderClient>,
        public ResponseBodyLoaderClient {
   USING_GARBAGE_COLLECTED_MIXIN(NoopResponseBodyLoaderClient);

   public:
    ~NoopResponseBodyLoaderClient() override {}
    void DidReceiveData(base::span<const char>) override {}
    void DidFinishLoadingBody() override {}
    void DidFailLoadingBody() override {}
    void DidCancelLoadingBody() override {}
  };
  void ReconstructResponse(LoadResourceEvent* event,
                           ResourceResponse& response);
  String GetRequestIdentifier(const ResourceRequestHead& request,
                              const String& resourceType);
  String GetRequestIdentifier(Resource* resource);
  void ReplayResponseInternal(Resource* resource, LoadResourceEvent* event,
                      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  void ReplayStreamResponseInternal(Resource* resource, LoadResourceEvent* event,
                            scoped_refptr<base::SingleThreadTaskRunner> task_runner);

  void LogRequestIdentifier(Resource* resource, const String& key = "RequestIdentifier");

  friend base::DefaultSingletonTraits<ResourceDataStore>;
  ResourceDataStore();
  ~ResourceDataStore();

  HashMap<String, LoadResourceEvent*> resource_map_;
  HashMap<String, std::pair<Resource*, scoped_refptr<base::SingleThreadTaskRunner> >> loaded_resource_map_;
  HashMap<String, bool> back_log_;
  bool replaying_;

};

}

#endif
