#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_REPLAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_REPLAYER_H_

#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/modules/storage/storage_area.h"

namespace blink {

class ReplayFrame;

class StorageReplayer final : public GarbageCollected<StorageReplayer> {

  public:
    explicit StorageReplayer(ReplayFrame* frame){replay_frame_ = frame;}
    void Trace(Visitor* visistor);
    String ReplayStorageGetItem(const String& key,
                                StorageArea::StorageType storage_type);
    String ReplayStorageKey(String key,
                            StorageArea::StorageType storage_type);

  private:
    Member<ReplayFrame> replay_frame_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_REPLAYER_H_
