#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_RECORDER_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_RECORDER_H

#include "third_party/blink/renderer/core/core_export.h"

#include "base/trace_event/common/trace_event_common.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/frame.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/page/chrome_client.h"
#include "third_party/blink/renderer/platform/json/json_values.h"
#include "v8/include/v8.h"


#define _IFA_DISABLE_ \
  if (!record())      \
    return;
//#define _IFA_DEVTOOLS_DISABLE_ if(IsDisabledForDevtools(frame)) return;
#define _IFAC_ "LOG::Forensics"
#define _IFAF_ __func__
#define _IFA_LOG_SPACE_ ";"
#define _IFA_ANDROID_LOGCAT_

#define _IFA_LOG_PREFIX_ _IFAC_ << "::" << _IFAF_ << " :"
#define _IFA_LOG_SUFFIX_ ""

#define _IFA_LOG_FRAME_                                                      \
  _IFA_LOG_SPACE_ << "frame=" << frame << _IFA_LOG_SPACE_                    \
                  << "frame_url=" << EncodeData(GetDocumentURL(frame))       \
                  << _IFA_LOG_SPACE_ << "main_frame=" << GetMainFrame(frame) \
                  << _IFA_LOG_SPACE_                                         \
                  << "local_frame_root=" << GetFrameRoot(frame)              \
                  << _IFA_LOG_SPACE_ << "local_frame_root_url="              \
                  << EncodeData(GetFrameRootURL(frame)) << _IFA_LOG_SPACE_   \
                  << "document=" << (void*)GetDocument(frame)
namespace blink {

class Frame;
class ChromeClient;
class Node;
class ExecutionContext;
class EventListener;
class ScriptSourceCode;
class ScriptState;
class EventTarget;
class SerializedScriptValue;
class DOMWindow;
class LocalDOMWindow;
class Event;
class KURL;

struct WebWindowFeatures;

struct ForensicFrameData {
  String frame;
  String frameUrl;
  String mainFrame;
  String localFrameRoot;
  String localFrameRootUrl;
  String recordId;
};

struct ForensicNodeData {
  int nodeId;
  int prevSiblingId;
  int parentId;
  int nextSiblingId;
  String nodeType;
  String nodeName;
  // String attrName;
  // String removedAttr;
  // String value;
  String markup;
  bool isElementNode = false;
  // String reason;
  double top = -1;
  double bottom = -1;
  double left = -1;
  double right = -1;
};

struct ThreadData {
  int tid;
  int pid;
  double timestamp;
  const char* name;
};


class CORE_EXPORT ForensicRecorder {
  DISALLOW_COPY(ForensicRecorder);

 public:
  static void LogInfo(std::string const& log_str);
  static void LogInfo(const std::string& log_str, double timestamp);
  static String EncodeData(std::string const& input);
  static String EncodeData(String const& input);
  static bool record() { return recording_; }
  static void setRecord(bool v) { recording_ = v; }
  static void ReplayError(LocalFrame* frame, WTF::String error_str);
  static void ReplayErrorInternal(std::unique_ptr<ThreadData> thread_data,
                                  std::unique_ptr<ForensicFrameData> frame_data,
                                  WTF::String error_str);

 protected:
  // utils
  static Document* GetDocument(LocalFrame* frame);
  static String GetDocumentURL(LocalFrame* frame);
  static LocalFrame* GetFrameRoot(LocalFrame* frame);
  static String GetFrameRootURL(LocalFrame* frame);
  static Frame* GetMainFrame(LocalFrame* frame);
  static ChromeClient* GetChromeClient(LocalFrame* frame);
  static void AddFrameInfo(LocalFrame* frame,
                           std::unique_ptr<JSONObject>& message);
  static void AddFrameInfo(std::unique_ptr<ForensicFrameData> frame_data,
                           std::unique_ptr<JSONObject>& message);
  static void AddNodeInfo(std::unique_ptr<ForensicNodeData>,
                          std::unique_ptr<JSONObject>& message);
  static void AddThreadData(std::unique_ptr<ThreadData>& thread_data,
                            std::unique_ptr<JSONObject>& message);
  static String AddrToString(void* addr);
  static String GetFrameId(Frame* frame);
  static scoped_refptr<base::SequencedTaskRunner> GetTaskRunner();

  static std::unique_ptr<ForensicFrameData> copyFrameData(LocalFrame* frame);
  static std::unique_ptr<ForensicNodeData> copyNodeData(Node* node);
  static std::unique_ptr<ThreadData> GetThreadData();

 private:
  static bool recording_;
};
}  // namespace blink

#endif  // !defined(THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_RECORDER_H)
