#include "third_party/blink/renderer/core/inspector/forensics/storage_recorder.h"

namespace blink {

void StorageRecorder::RecordGetItem(
    LocalFrame* frame,
    const String& key,
    const String& value,
    const StorageArea::StorageType& storage_type) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  std::unique_ptr<StorageGetItemData> storage_data =
          std::make_unique<StorageGetItemData>();

  storage_data->key = key.IsolatedCopy();
  storage_data->value = value.IsolatedCopy();
  storage_data->is_null = value.IsNull();
  storage_data->storage_type = storage_type;

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&StorageRecorder::RecordGetItemInternal,
                     copyFrameData(frame),
                     std::move(storage_data),
                     GetThreadData()));
}

void StorageRecorder::RecordGetItemInternal(
        std::unique_ptr<ForensicFrameData> frame,
        std::unique_ptr<StorageGetItemData> storage_data,
        std::unique_ptr<ThreadData> thread_data) {

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetString("key", storage_data->key);
  message->SetString("value", storage_data->value);
  message->SetBoolean("isNull", storage_data->is_null);
  message->SetString("type",
     storage_data->storage_type == StorageArea::StorageType::kLocalStorage ?
     "localStorage" : "sessionStorage");

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void StorageRecorder::RecordKey(
    LocalFrame* frame,
    unsigned index,
    const String& key,
    const StorageArea::StorageType& storage_type) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  std::unique_ptr<StorageKeyData> storage_data =
      std::make_unique<StorageKeyData>();
  storage_data->key = key.IsolatedCopy();
  storage_data->index = index;
  storage_data->storage_type = storage_type;

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&StorageRecorder::RecordKeyInternal,
                     copyFrameData(frame),
                     std::move(storage_data),
                     GetThreadData()));
}

void StorageRecorder::RecordKeyInternal(
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<StorageKeyData> storage_data,
    std::unique_ptr<ThreadData> thread_data) {

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetString("key", storage_data->key);
  message->SetInteger("index", storage_data->index);
  message->SetString("type",
      storage_data->storage_type == StorageArea::StorageType::kLocalStorage ?
      "localStorage" : "sessionStorage");

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}


} // blink
