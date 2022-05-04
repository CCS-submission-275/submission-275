#include "third_party/blink/renderer/core/inspector/forensics/v8_recorder.h"
#include <memory>
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/platform/json/json_values.h"

namespace blink {

void V8Recorder::CryptoRandomValues(const String& values) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  ScriptState* ss = ScriptState::Current(isolate);
  LocalDOMWindow* dom_window = LocalDOMWindow::From(ss);
  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&V8Recorder::CryptoRandomValuesInternal,
                                values.IsolatedCopy(), GetThreadData(),
                                copyFrameData(dom_window->GetFrame())));
}

void V8Recorder::CryptoRandomValuesInternal(
    const String& values,
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data) {
  auto message = std::make_unique<JSONObject>();
  message->SetString("values", values);
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void V8Recorder::DumpV8Records(v8::PlatformInstrumentationData& data,
                               const String& source) {
  // std::stringstream ss;
  // ss << std::endl << " current timestamps:" << std::endl;
  // for (auto val : data.current_timestamps_) {
  //   ss << val << ", ";
  // }
  // ss << std::endl << " random number:" << std::endl;
  // for (auto val : data.math_random_values_) {
  //   ss << val << ", ";
  // }
  // VLOG(7) << __func__ << GetFrameId(frame) << ss.str();
  if (data.size()) {
    GetTaskRunner()->PostTask(
        FROM_HERE,
        base::BindOnce(&V8Recorder::DumpV8RecordsInternal,
                       source.IsolatedCopy(), data, GetThreadData()));
  }
}

void V8Recorder::DumpV8RecordsInternal(
    const String& source,
    const v8::PlatformInstrumentationData& data,
    std::unique_ptr<ThreadData> thread_data) {
  auto message = std::make_unique<JSONObject>();
  message->SetString("source", source);
  auto dt = std::make_unique<JSONArray>();
  auto rnd = std::make_unique<JSONArray>();

  message->SetArray("mathRandomNumbers",
                    mapToJSONArray(data.math_random_values_));
  message->SetArray("currentTimestamps",
                    mapToJSONArray(data.current_timestamps_));
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

std::unique_ptr<JSONArray> V8Recorder::mapToJSONArray(
    const std::unordered_map<std::string, std::vector<double>>& map) {
  auto out = std::make_unique<JSONArray>();
  for (auto val : map) {
    auto entry = std::make_unique<JSONObject>();
    entry->SetString("securityToken", val.first.c_str());
    auto values = std::make_unique<JSONArray>();
    for (auto it : val.second) {
      // we convert it to string with 16 digits after the decimal point to address precision issue
      // By defaut, C++ only pick 15 digits
      values->PushString(String::Number(it, 20));
    }
    entry->SetArray("values", std::move(values));
    out->PushObject(std::move(entry));
  }
  return out;
}
}  // namespace blink
