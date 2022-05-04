//
// Created by  on 5/11/21.
//

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_RECORDER_H_
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/modules/storage/storage_area.h"


namespace blink {

class LocalFrame;

struct StorageGetItemData {
  String key;
  String value;
  StorageArea::StorageType storage_type;
  bool is_null;
};

struct StorageKeyData {
  String key;
  unsigned index;
  StorageArea::StorageType storage_type;
};

class CORE_EXPORT StorageRecorder : ForensicRecorder {
  DISALLOW_COPY(StorageRecorder);

  public:
  // Methods for recording localStorage and sessionStorage.
  static void RecordGetItem(LocalFrame* frame,
                            const String& key,
                            const String& value,
                            const StorageArea::StorageType& storage_type);
  static void RecordKey(LocalFrame* frame,
                        unsigned index,
                        const String& key,
                        const StorageArea::StorageType& storage_type);

  static void RecordGetItemInternal(
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<StorageGetItemData> storage_data,
      std::unique_ptr<ThreadData> thread_data);

  static void RecordKeyInternal(
        std::unique_ptr<ForensicFrameData> frame,
        std::unique_ptr<StorageKeyData> storage_data,
        std::unique_ptr<ThreadData> thread_data);

};


} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_STORAGE_RECORDER_H_
