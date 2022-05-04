#include "third_party/blink/renderer/core/inspector/forensics/navigation_recorder.h"
#include "base/bind.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "third_party/blink/public/web/web_window_features.h"
#include "third_party/blink/renderer/core/dom/events/event.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/core/inspector/inspector_page_agent.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"

#include <sstream>

namespace blink {

namespace {
String ClientNavigationReasonToProtocol(ClientNavigationReason reason) {
  namespace ReasonEnum = protocol::Page::ClientNavigationReasonEnum;
  switch (reason) {
    case ClientNavigationReason::kAnchorClick:return ReasonEnum::AnchorClick;
    case ClientNavigationReason::kFormSubmissionGet:return ReasonEnum::FormSubmissionGet;
    case ClientNavigationReason::kFormSubmissionPost:return ReasonEnum::FormSubmissionPost;
    case ClientNavigationReason::kHttpHeaderRefresh:return ReasonEnum::HttpHeaderRefresh;
    case ClientNavigationReason::kFrameNavigation:return ReasonEnum::ScriptInitiated;
    case ClientNavigationReason::kMetaTagRefresh:return ReasonEnum::MetaTagRefresh;
    case ClientNavigationReason::kPageBlock:return ReasonEnum::PageBlockInterstitial;
    case ClientNavigationReason::kReload:return ReasonEnum::Reload;
    default:NOTREACHED();
  }
  return ReasonEnum::Reload;
}
}  // namespace

void NavigationRecorder::WindowOpenInternal(
        std::unique_ptr<ThreadData> thread_data,
        std::unique_ptr<ForensicFrameData> frame_data,
        const String& url,
        const String& window_name,
        bool user_gesture) {
  auto message = std::make_unique<JSONObject>();

  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetString("url", url);
  message->SetString("windowName", window_name);
  message->SetBoolean("userGesture", user_gesture);


  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::WindowOpen(Document* document,
                                    const KURL& url,
                                    const AtomicString& window_name,
                                    const WebWindowFeatures& window_features,
                                    bool user_gesture) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  LocalFrame* frame = document->GetFrame();

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &NavigationRecorder::WindowOpenInternal,
      GetThreadData(),
      copyFrameData(frame),
      url.GetString().IsolatedCopy(),
      window_name.GetString().IsolatedCopy(),
      user_gesture
  ));
}

void NavigationRecorder::WindowCreatedInternal(
        std::unique_ptr<ThreadData> thread_data,
        std::unique_ptr<ForensicFrameData> frame_data,
        const String& created_frame) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetString("createdFrame", created_frame);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::WindowCreated(LocalFrame* frame, LocalFrame* opener_frame) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &NavigationRecorder::WindowCreatedInternal,
      GetThreadData(),
      copyFrameData(opener_frame),
      GetFrameId(frame)
  ));
}

void NavigationRecorder::WillDownloadURL(LocalFrame* frame,
                                         const KURL& url,
                                         const String& suggested_name) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
//  auto message = std::make_unique<JSONObject>();
//
//  AddFrameInfo(frame, message);
//  message->SetString("url", url.GetString());
//  message->SetString("suggestedName", suggested_name);
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}
void NavigationRecorder::WillLoadFrame(LocalFrame* frame,
                                       const KURL& url,
                                       int navigation_policy_enum,
                                       int request_context_enum,
                                       int frame_load_type_enum,
                                       Event* triggering_event,
                                       std::string event_type) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
//
//  auto message = std::make_unique<JSONObject>();
//
//  AddFrameInfo(frame, message);
//  message->SetString("url", url.GetString());
//  message->SetInteger("navigationPolicy", navigation_policy_enum);
//  message->SetInteger("requestContext", request_context_enum);
//  message->SetInteger("frameLoadTypeEnum", frame_load_type_enum);
//  message->SetString("triggeringEvent", triggering_event->type().GetString());
//  message->SetString("eventType", event_type.c_str());
//
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::FrameRequestedNavigationInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const String& url,
    ClientNavigationReason reason,
    bool replace_current_item) {
  auto message = std::make_unique<JSONObject>();

  AddFrameInfo(std::move(frame_data), message);
  message->SetString("url", url);
  message->SetString("navigationReason",
                     ClientNavigationReasonToProtocol(reason));
  message->SetBoolean("replaceCurrentItem", replace_current_item);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::FrameRequestedNavigation(LocalFrame* frame,
                                                  Frame* target_frame,
                                                  const KURL& url,
                                                  ClientNavigationReason reason,
                                                  bool replace_current_item) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &NavigationRecorder::FrameRequestedNavigationInternal,
      GetThreadData(),
      copyFrameData(frame),
      url.GetString().IsolatedCopy(),
      reason,
      replace_current_item
  ));
}

void NavigationRecorder::FrameScheduledNavigation(LocalFrame* frame,
                                                  const KURL& url,
                                                  ClientNavigationReason reason,
                                                  bool replace_current_item) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

//  auto message = std::make_unique<JSONObject>();
//
//  AddFrameInfo(frame, message);
//
//  message->SetString("url", url.GetString());
//  message->SetString("navigationReason",
//                     ClientNavigationReasonToProtocol(reason));
//  message->SetBoolean("replaceCurrentItem", replace_current_item);
//
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::FrameClearedScheduledNavigation(LocalFrame* frame) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

//  auto message = std::make_unique<JSONObject>();
//
//  AddFrameInfo(frame, message);
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::DidCommitLoad(LocalFrame* frame,
                                       DocumentLoader* document_loader) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

//  auto message = std::make_unique<JSONObject>();
//
//  AddFrameInfo(frame, message);
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::BeginNavigation(LocalFrame* frame, const KURL& url) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

//  auto message = std::make_unique<JSONObject>();
//
//  message->SetString("url", url.GetString());
//  message->SetString("frameId", GetFrameId(frame));
//
//  std::ostringstream log_str;
//  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
//
//  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::DidCreateDocumentLoaderInternal(
        std::unique_ptr<ThreadData> thread_data,
        std::unique_ptr<ForensicFrameData> frame_data,
        const String& url,
        int navigation_type) {

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetString("url", url);
  message->SetInteger("WebNavigationType", navigation_type);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void NavigationRecorder::DidCreateDocumentLoader(
    LocalFrame* frame,
    const KURL& url,
    WebFrameLoadType load_type,
    WebNavigationType navigation_type) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&NavigationRecorder::DidCreateDocumentLoaderInternal,
                     GetThreadData(), copyFrameData(frame),
                     url.GetString().IsolatedCopy(), navigation_type));
}

}  // namespace blink
