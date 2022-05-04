#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"

#include "base/containers/span.h"
#include "base/logging.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_layout_constraints.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "base/memory/scoped_refptr.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/web/web_frame_load_type.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "third_party/blink/public/web/web_window_features.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/events/event.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/editing/serializers/serialization.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/html/html_script_element.h"
#include "third_party/blink/renderer/core/html_names.h"
#include "third_party/blink/renderer/core/page/page.h"
#include "third_party/blink/renderer/platform/wtf/text/base64.h"
#include "third_party/blink/renderer/platform/wtf/text/text_position.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/core/geometry/dom_rect.h"
#include "third_party/blink/renderer/bindings/core/v8/script_source_code.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/platform/forensics/forensic_file_logger.h"

#include "base/process/process.h"
#include "base/threading/platform_thread.h"
#include <sstream>

namespace blink {

using std::string;

bool ForensicRecorder::recording_ = true;

String ForensicRecorder::EncodeData(std::string const& input) {
  String output;
  output = Base64Encode(base::as_bytes(base::make_span(input)));
  return output;
}

String ForensicRecorder::EncodeData(String const& input) {
  String output;
  output = Base64Encode(base::as_bytes(base::make_span(input.Utf8())));
  return output;
}

// static
Document* ForensicRecorder::GetDocument(LocalFrame* frame) {
  if (!frame)
    return nullptr;

  return frame->GetDocument();
}

// static
String ForensicRecorder::GetDocumentURL(LocalFrame* frame) {
  if (!frame || !frame->GetDocument())
    return KURL().GetString();

  return frame->GetDocument()->Url().GetString();
}

// static
LocalFrame* ForensicRecorder::GetFrameRoot(LocalFrame* frame) {
  if (!frame)
    return nullptr;

  return &(frame->LocalFrameRoot());
}

// static
String ForensicRecorder::GetFrameRootURL(LocalFrame* frame) {
  if (!frame)
    return KURL().GetString();

  return GetDocumentURL(GetFrameRoot(frame));
}

// static
Frame* ForensicRecorder::GetMainFrame(LocalFrame* frame) {
  if (!frame || !frame->GetDocument() || !frame->GetDocument()->GetPage())
    return nullptr;

  return frame->GetDocument()->GetPage()->MainFrame();
}

// static
ChromeClient* ForensicRecorder::GetChromeClient(LocalFrame* frame) {
  if (!frame || !frame->GetDocument() || !frame->GetDocument()->GetPage())
    return nullptr;

  return &(frame->GetDocument()->GetPage()->GetChromeClient());
}

// static
void ForensicRecorder::LogInfo(std::string const& log_str) {
   blink::ForensicFileLogger::GetInstance()->log(
       std::to_string(base::Time::Now().ToDoubleT()) + " || " + log_str);
  // VLOG(7) << log_str;
}

void ForensicRecorder::LogInfo(const std::string& log_str, double timestamp) {
  ForensicFileLogger::GetInstance()->log(
          std::to_string(base::Process::Current().Pid()) + ":" +
          std::to_string(base::PlatformThread::CurrentId()) + " || " +
          std::to_string(timestamp) + " || " + log_str);
  // VLOG(7) << log_str;
}



void ForensicRecorder::AddFrameInfo(LocalFrame* frame,
                                    std::unique_ptr<JSONObject>& message) {
  message->SetString("frame", GetFrameId(frame));
  message->SetString("frameAddr", AddrToString(frame));
  message->SetString("frameUrl", GetDocumentURL(frame));
  message->SetString("mainFrame", GetFrameId(GetMainFrame(frame)));
  message->SetString("localFrameRoot", GetFrameId(GetFrameRoot(frame)));
  message->SetString("localFrameRootUrl", GetFrameRootURL(frame));
  message->SetString("document", AddrToString(GetDocument(frame)));
  //NOTE(): Only need to occur in debugging.
  if (ForensicReplayEngine::GetInstance()->replay())
    message->SetString("recordId", frame->GetReplayId());
}

void ForensicRecorder::AddFrameInfo(
    std::unique_ptr<ForensicFrameData> frame_data,
    std::unique_ptr<JSONObject>& message) {
  message->SetString("frame", frame_data->frame);
  message->SetString("frameUrl", frame_data->frameUrl);
  message->SetString("mainFrame", frame_data->mainFrame);
  message->SetString("localFrameRoot", frame_data->localFrameRoot);
  message->SetString("localFrameRootUrl", frame_data->localFrameRootUrl);
  message->SetString("recordId", frame_data->recordId);
}

std::unique_ptr<ForensicFrameData> ForensicRecorder::copyFrameData(
    LocalFrame* frame) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  std::unique_ptr<ForensicFrameData> frame_data =
      std::make_unique<ForensicFrameData>();

  if (!frame)
    return frame_data;

  frame_data->frame = GetFrameId(frame);
  frame_data->frameUrl = GetDocumentURL(frame).IsolatedCopy();
  frame_data->mainFrame = GetFrameId(GetMainFrame(frame));
  frame_data->localFrameRoot = GetFrameId(GetFrameRoot(frame));
  frame_data->localFrameRootUrl = GetFrameRootURL(frame).IsolatedCopy();

  //NOTE(): Only need to occur in debugging.
  if (ForensicReplayEngine::GetInstance()->replay())
    frame_data->recordId = frame->GetReplayId().IsolatedCopy();
  else
    frame_data->recordId = "recordId";

  return frame_data;
}

String ForensicRecorder::AddrToString(void* addr) {
  std::stringstream ss;
  ss << static_cast<void*>(addr);
  return String(ss.str().c_str());
}

String ForensicRecorder::GetFrameId(Frame* frame) {
  return IdentifiersFactory::FrameId(frame);
}

scoped_refptr<base::SequencedTaskRunner> ForensicRecorder::GetTaskRunner() {
  return ForensicFileLogger::GetInstance()->GetOrCreateFileThread();
}

std::unique_ptr<ForensicNodeData> ForensicRecorder::copyNodeData(Node* node) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  std::unique_ptr<ForensicNodeData> node_data =
      std::make_unique<ForensicNodeData>();
  node_data->nodeId = IdentifiersFactory::IntIdForNode(node);
  node_data->nodeType = node->ToString().IsolatedCopy();
  node_data->nodeName = node->nodeName().IsolatedCopy();
  node_data->prevSiblingId = IdentifiersFactory::IntIdForNode(node->previousSibling());
  node_data->nextSiblingId = IdentifiersFactory::IntIdForNode(node->nextSibling());
  node_data->parentId = IdentifiersFactory::IntIdForNode(node->parentNode());
  node_data->markup = CreateMarkup(node).IsolatedCopy();
  node_data->isElementNode = node->IsElementNode();


  return node_data;
}

void ForensicRecorder::AddNodeInfo(std::unique_ptr<ForensicNodeData> node_data,
                                   std::unique_ptr<JSONObject>& message) {
  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("nodeName", node_data->nodeName);
  message->SetDouble("top", node_data->top);
  message->SetDouble("bottom", node_data->bottom);
  message->SetDouble("left", node_data->left);
  message->SetDouble("right", node_data->right);
}

// Used for recording replay issues.
void ForensicRecorder::ReplayError(LocalFrame* frame, WTF::String error_str) {
  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &ForensicRecorder::ReplayErrorInternal,
      GetThreadData(),
      copyFrameData(frame),
      error_str.IsolatedCopy()));
}

void ForensicRecorder::ReplayErrorInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    WTF::String error_str) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("Error", error_str);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}



void ForensicRecorder::AddThreadData(std::unique_ptr<ThreadData>& thread_data,
                                     std::unique_ptr<JSONObject>& message) {
  _IFA_DISABLE_
  message->SetInteger("tid", thread_data->tid);
  message->SetString("threadName", thread_data->name);
  message->SetInteger("pid", thread_data->pid);
}

std::unique_ptr<ThreadData> ForensicRecorder::GetThreadData() {
  std::unique_ptr<ThreadData> res = std::make_unique<ThreadData>();
  res->timestamp = base::Time::Now().ToDoubleT();
  res->tid = base::PlatformThread::CurrentId();
  res->pid = base::Process::Current().Pid();
  res->name = base::PlatformThread::GetName();
  return res;
}

}  // namespace blink
