
#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_V8_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_V8_RECORDER_H_

#include <memory>
#include <unordered_map>
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"

namespace blink {
class JSONObject;

class CORE_EXPORT V8Recorder : ForensicRecorder {
  DISALLOW_COPY(V8Recorder);

 public:
  static void DumpV8Records(v8::PlatformInstrumentationData& data,
                            const String&);

  static void CryptoRandomValues(const String& values);

 private:
  static void DumpV8RecordsInternal(const String& source,
                                    const v8::PlatformInstrumentationData& data,
                                    std::unique_ptr<ThreadData> thread_data);

  static void CryptoRandomValuesInternal(
      const String& values,
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data);

  static std::unique_ptr<JSONArray> mapToJSONArray(
      const std::unordered_map<std::string, std::vector<double>>& map);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_V8_RECORDER_H_
