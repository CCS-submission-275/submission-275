#include "third_party/blink/renderer/core/inspector/forensics/storage_replayer.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"


namespace blink {

void StorageReplayer::Trace(Visitor* visitor) {
  visitor->Trace(replay_frame_);
}

String StorageReplayer::ReplayStorageKey(
    String key,
    StorageArea::StorageType storage_type) {
  VLOG(7) << "Getting value for key: " << key;

  ForensicStorageEvent* event = ForensicDataStore::GetInstance()
      ->NextStorageEvent(replay_frame_, "key");

  if (!event) {
    VLOG(7) << "No event for call to Storage.key()";
    return String();
  } else
    return event->Key();
}

String StorageReplayer::ReplayStorageGetItem(
    const String& key,
    StorageArea::StorageType storage_type) {

  VLOG(7) << "Getting Item for key: " << key;
  ForensicStorageEvent* event = ForensicDataStore::GetInstance()
      ->NextStorageEvent(replay_frame_, "getItem");

  if (!event) {
    VLOG(7) << "[StorageError]: No event for call to Storage.key()";
    return String();
  } else if (event->IsNull()){
    return String();
  } else {
    VLOG(7) << "Replaying value: " << event->ToString() << "Is Null: " << event->Value().IsNull();
    return event->Value();
  }
}

} // blink