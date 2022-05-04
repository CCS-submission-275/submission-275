#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"
#include "third_party/blink/public/web/web_navigation_params.h"
#include "third_party/blink/renderer/core/dom/container_node.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/core/html/html_frame_element_base.h"
#include "third_party/blink/renderer/core/html/html_head_element.h"
#include "third_party/blink/renderer/core/html_names.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/loader/frame_load_request.h"
#include "third_party/blink/renderer/core/messaging/message_port.h"
#include "third_party/blink/renderer/core/script/classic_pending_script.h"
#include "third_party/blink/renderer/core/script/pending_script.h"
#include "third_party/blink/renderer/platform/forensics/resource_data_store.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/shared_buffer.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

ForensicReplayEngine* ForensicReplayEngine::GetInstance() {
  return base::Singleton<ForensicReplayEngine>::get();
}

ForensicReplayEngine::ForensicReplayEngine() {
  initialized_ = false;
  replaying_ = false;
}

void ForensicReplayEngine::RegisterClassicPendingScript(
    ClassicPendingScript* script) {
  ExecutionContext* context =
      script->GetElement()->GetDocument().GetExecutionContext();
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame) {
    replay_frame->GetJSReplayer()->RegisterClassicPendingScript(script);
  }
}

void ForensicReplayEngine::ExecutePendingInSyncScript(String script_source,
                                                      KURL script_url,
                                                      PendingScript* script) {
  ExecutionContext* context =
      script->GetElement()->GetDocument().GetExecutionContext();
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame) {
    JSReplayer* js_replayer = replay_frame->GetJSReplayer();
    js_replayer->ExecutePendingInSyncScript(script_source, script_url, script);
  }
}

void ForensicReplayEngine::Initialize(InspectedFrames* inspected_frames) {
  initialized_ = true;
  is_replay_finished_ = true;
  dispatcher_paused_ = false;
  inspected_frames_ = inspected_frames;
  datastore_ = ForensicDataStore::GetInstance();
  current_event_ = nullptr;
}

ReplayFrame* ForensicReplayEngine::CurrentFrame(WTF::String frame_id) {
  auto current = frame_map_.find(frame_id);
  if (current == frame_map_.end())
    return nullptr;
  else
    return current->value;
}

ReplayFrame* ForensicReplayEngine::CurrentFrame(ExecutionContext* context) {
  LocalFrame* frame = context->ExecutingWindow()->GetFrame();
  return CurrentFrame(frame);
}

ReplayFrame* ForensicReplayEngine::CurrentFrame(Node* node) {
  LocalFrame* frame = node->GetDocument().GetFrame();
  return CurrentFrame(frame);
}

ReplayFrame* ForensicReplayEngine::CurrentFrame(LocalFrame* frame) {
  if (frame)
    return CurrentFrame(frame->GetReplayId());
  else
    return nullptr;
}

void ForensicReplayEngine::StartReplay() {
  replaying_ = true;
  is_replay_finished_ = false;
  DCHECK(initialized_) << "The replay engine has not been initialized.";
  // NOTE: We pull the first navigation event off the queue and handle it
  // manually.
  // This is so we can initialize 'main_frame_'.
  ForensicEventType event_type = datastore_->NextForensicEventType();
  DCHECK(event_type == ForensicEventType::kLoadPage);
  NavigationEvent* event = datastore_->NextNavigationEvent();
  HandleNavigationEvent(event);
  main_frame_ = CreateReplayFrame(event->FrameId(), inspected_frames_->Root());
  // Load v8 records in one process
  v8::V8::StartPlatformInstReplaying();
  v8::V8::LoadPlatformInstRecording(datastore_->getV8InstrumentData());
  // Schedule dispatcher in render thread's queue.
  DispatchAsync();
}

void ForensicReplayEngine::DispatchAsync() {
  auto task = WTF::Bind(&ForensicReplayEngine::Dispatch, WTF::Unretained(this));
  main_frame_->GetFrame()
      ->GetTaskRunner(TaskType::kDOMManipulation)
      ->PostTask(FROM_HERE, std::move(task));
}

// This is the main loop for the replay engine.
void ForensicReplayEngine::Dispatch() {
  if (dispatcher_paused_ && !is_replay_finished_) {
    DispatchAsync();
    return;
  }

  ForensicEventType event_type = datastore_->NextForensicEventType();
  switch (event_type) {
    case ForensicEventType::kLoadPage: {
      NavigationEvent* event = datastore_->NextNavigationEvent();
      HandleNavigationEvent(event);
      break;
    }
    case ForensicEventType::kDOMNode: {
      DOMNodeEvent* event = datastore_->NextDOMNodeEvent();
      current_event_ = event;
      HandleDOMNodeEvent(event);
      break;
    }
    case ForensicEventType::kJSEvent: {
      // this should also handle xmlhttprequest
      JSEvent* event = datastore_->NextJSEvent();
      HandleReplayJSEvent(event);
      break;
    }
    case ForensicEventType::kTimerEvent: {
      TimerEvent* event = datastore_->NextTimerEvent();
      HandleTimerEvent(event);
      break;
    }
    case ForensicEventType::kIdleCallbackEvent: {
      IdleCallbackEvent* event = datastore_->NextIdleCallbackEvent();
      HandleIdleCallbackEvent(event);
      break;
    }
    case ForensicEventType::kAnimationFrameEvent: {
      AnimationFrameEvent* event = datastore_->NextAnimationFrameEvent();
      HandleAnimationFrameEvent(event);
      break;
    }

    case ForensicEventType::kScriptExecutionEvent: {
      ScriptExecutionEvent* event = datastore_->NextScriptExecutionEvent();
      HandleScriptExecutionEvent(event);
      break;
    }
    case ForensicEventType::kEmptyQueue: {
      is_replay_finished_ = true;
      // replaying_ = false;
      VLOG(7) << "Replay Finished!";
      break;
    }
    case ForensicEventType::kLoadResource: {
      String key = datastore_->NextLoadResourceIdentifier();
      ResourceDataStore::GetInstance()->DispatchLoadFinish(key);
      break;
    }
    case ForensicEventType::kCryptoRandom: {
      // we don't need to anything here but
      break;
    }
    default:
      NOTREACHED() << "Invalid event type: " << event_type;
  }

  if (!is_replay_finished_)
    DispatchAsync();
}

void ForensicReplayEngine::HandleIdleCallbackEvent(IdleCallbackEvent* event) {
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame)
    current_frame->GetJSReplayer()->ReplayIdleCallbackEvent(event);
}

void ForensicReplayEngine::HandleAnimationFrameEvent(
    AnimationFrameEvent* event) {
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame)
    current_frame->GetJSReplayer()->ReplayAnimationFrameEvent(event);
}

void ForensicReplayEngine::RegisterFrameCreation(LocalFrame* frame,
                                                 HTMLFrameOwnerElement* owner) {
  auto* parent_frame = CurrentFrame(owner->GetDocument().GetFrame());
  if (!parent_frame) {
    VLOG(7) << "Did not find parent frame during registration.";
    return;
  }

  auto* event = datastore_->NextJSInsertedIframeEvent(parent_frame);
  if (!event) {
    VLOG(7) << "No remaining iframe events found for frame: "
            << frame->GetReplayId();
    return;
  }

  VLOG(7) << "Registering (owner): " << owner->GetDocument().Url()
          << " FrameEvent: " << event->ToString();

  // Create a replay frame for this local frame.
  CreateReplayFrame(event->GetSubframeId(), frame);
}

void ForensicReplayEngine::HandleReplayJSEvent(JSEvent* event) {
  VLOG(7) << "Replaying event: " << event->ToString();
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame) {
    current_frame->GetJSReplayer()->ReplayJSEvent(event);
  } else
    VLOG(7) << __func__ << "[Failed JSEvent]: Current Frame does not exist!"
            << "Cannot replay event: " << event->ToString() << std::endl;
}

void ForensicReplayEngine::HandleNavigationEvent(NavigationEvent* event) {
  VLOG(7) << __func__ << "Navigation Event: " << event->ToString();
  // Navigate this event's frame.
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame)
    current_frame->Navigate(event->Url());
  else
    VLOG(7) << __func__ << "Current Frame does not exist!" << std::endl;
}

void ForensicReplayEngine::HandleScriptExecutionEvent(
    ScriptExecutionEvent* event) {
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame)
    current_frame->GetJSReplayer()->ReplayScriptExecutionEvent(event);
  else
    VLOG(7) << __func__ << "No Frame for event: " << event->ToString();
}

void ForensicReplayEngine::HandleDOMNodeEvent(DOMNodeEvent* event) {
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (!current_frame) {
    // VLOG(7) << __func__ << " Cannot identify frame for: " <<
    // event->ToString();
    return;
  }

  // NOTE(): This code should be moved to replay frame.
  if (event->GetEventType() == "DOMInsertNode") {
    current_frame->InsertDOMNode(event);
  } else if (event->GetEventType() == "DOMInsertFrame") {
    VLOG(7) << " Inserting Iframe: " << event->GetSubframeId()
            << event->ToString();
    current_frame->InsertFrameNode(event);
  } else if (event->GetEventType() == "DOMRemoveNode") {
    // NOTREACHED() << "Parser should not be removing nodes: " <<
    // event->ToString();
    VLOG(7) << "Parser should not be removing nodes: " << event->ToString();
    // current_frame->RemoveDOMNode(event);
  } else {
    NOTREACHED() << "Unknown DOMNodeEventType" << event->ToString();
  }
}

void ForensicReplayEngine::HandleTimerEvent(TimerEvent* event) {
  ReplayFrame* current_frame = CurrentFrame(event->FrameId());
  if (current_frame)
    current_frame->GetJSReplayer()->ReplayTimerEvent(event);
  else
    VLOG(7) << __func__ << "No Frame for timer event:" << event->ToString();
}

void ForensicReplayEngine::RegisterTimerEvent(ExecutionContext* context,
                                              DOMTimer* dom_timer,
                                              bool is_one_shot) {
  ReplayFrame* frame = CurrentFrame(context);
  if (frame)
    frame->GetJSReplayer()->RegisterTimerEvent(context, dom_timer, is_one_shot);
  else
    NOTREACHED() << "No Replay Frame associated with this DOMTimer.";
}

void ForensicReplayEngine::VerifyDOMEvent(
    Node* node,
    ForensicDOMEventType modification_type) {
  ReplayFrame* replay_frame = CurrentFrame(node);
  if (replay_frame)
    replay_frame->VerifyDOMEvent(node, modification_type);
  // else
  //  VLOG(7) << "Not verifying: " << node->ToString() << " has no frame. This
  //  is usually okay.";
}

void ForensicReplayEngine::VerifyDOMAttrEvent(Node* node,
                                              const QualifiedName& name,
                                              const AtomicString& value,
                                              ForensicDOMAttrEventType type) {
  ReplayFrame* replay_frame = CurrentFrame(node);
  if (replay_frame)
    replay_frame->VerifyDOMAttrEvent(node, name, value, type);
  else
    VLOG(7) << "Not verifying attr: " << node->ToString();
}

ReplayFrame* ForensicReplayEngine::CreateReplayFrame(WTF::String frame_id,
                                                     LocalFrame* frame) {
  DCHECK(frame);
  ReplayFrame* replay_frame =
      MakeGarbageCollected<ReplayFrame>(frame_id, frame);
  frame_map_.insert(frame_id, replay_frame);
  return replay_frame;
}

Document* ForensicReplayEngine::GetDocument() {
  return inspected_frames_->Root()->GetDocument();
}

// TODO(): Update to work w/ JSReplayer.
void ForensicReplayEngine::RegisterIdleCallbackEvent(ExecutionContext* context,
                                                     int callback_id) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame)
    replay_frame->GetJSReplayer()->RegisterIdleCallbackEvent(callback_id);
  else
    NOTREACHED() << "No Replay Frame associated with this IdleCallback: "
                 << callback_id;
}

void ForensicReplayEngine::RegisterAnimationFrameEvent(
    ExecutionContext* context,
    FrameRequestCallbackCollection::FrameCallback* callback) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame)
    replay_frame->GetJSReplayer()->RegisterAnimationFrameEvent(callback);
  else
    NOTREACHED() << "No Replay Frame associated with animation frame: "
                 << callback->Id();
}

void ForensicReplayEngine::RegisterXMLHttpRequestTarget(
    ExecutionContext* context,
    int unique_id,
    XMLHttpRequest* xml_http_request) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame)
    replay_frame->RegisterXMLHttpRequestTarget(unique_id, xml_http_request);
  else
    NOTREACHED()
        << "No Replay Frame associated with this XMLHttpRequestTarget: "
        << unique_id;
}

void ForensicReplayEngine::RegisterMessagePort(ExecutionContext* context,
                                               MessagePort* message_port) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame)
    replay_frame->RegisterMessagePort(message_port->GetMessagePortId(),
                                      message_port);
  else
    NOTREACHED() << "No Replay Frame associated with this MessagePort: "
                 << message_port->GetMessagePortId();
}

// Replay Storage Events (LocalStorage/SessionStorage)
String ForensicReplayEngine::ReplayStorageGetItem(
    LocalFrame* frame,
    const String& key,
    StorageArea::StorageType storage_type) {
  ReplayFrame* replay_frame = CurrentFrame(frame);
  if (replay_frame)
    return replay_frame->GetStorageReplayer()->ReplayStorageGetItem(
        key, storage_type);

  NOTREACHED();
  return String();
}

String ForensicReplayEngine::ReplayCryptoRandomValues() {
  // we need to get current frame from the isolate
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  ScriptState* ss = ScriptState::Current(isolate);
  LocalDOMWindow* dom_window = LocalDOMWindow::From(ss);
  ReplayFrame* replay_frame = CurrentFrame(dom_window->GetFrame());
  if (replay_frame)
    return datastore_->NextCryptoRandomValues(replay_frame);
  return String();
}

String ForensicReplayEngine::ReplayStorageKey(
    LocalFrame* frame,
    String key,
    StorageArea::StorageType storage_type) {
  ReplayFrame* replay_frame = CurrentFrame(frame);
  if (replay_frame)
    return replay_frame->GetStorageReplayer()->ReplayStorageKey(key,
                                                                storage_type);

  NOTREACHED();
  return String();
}

bool ForensicReplayEngine::IsJSExecuting(ExecutionContext* context) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  if (replay_frame)
    return replay_frame->GetJSReplayer()->IsJSExecuting();
  NOTREACHED();
  return false;
}

std::pair<bool, String> ForensicReplayEngine::ReplayGetterEventString(
    ExecutionContext* context,
    String interface,
    String attribute) {
  return ReplayGetterEventInternal(context, interface, attribute);
}

std::pair<bool, uint16_t> ForensicReplayEngine::ReplayGetterEventuint16_t(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, uint16_t>(value.first, value.second.ToUInt());
}

std::pair<bool, uint32_t> ForensicReplayEngine::ReplayGetterEventuint32_t(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, uint32_t>(value.first, value.second.ToUInt());
}

std::pair<bool, int32_t> ForensicReplayEngine::ReplayGetterEventint32_t(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, int32_t>(value.first, value.second.ToInt());
}

std::pair<bool, int64_t> ForensicReplayEngine::ReplayGetterEventint64_t(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, int64_t>(value.first, value.second.ToInt64Strict());
}

std::pair<bool, uint64_t> ForensicReplayEngine::ReplayGetterEventuint64_t(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, uint64_t>(value.first, value.second.ToUInt64Strict());
}

std::pair<bool, double> ForensicReplayEngine::ReplayGetterEventdouble(
    ExecutionContext* context,
    String interface,
    String attribute) {
  auto value = ReplayGetterEventInternal(context, interface, attribute);
  return std::pair<bool, double>(value.first, value.second.ToDouble());
}

double ForensicReplayEngine::ReplayPerformanceEvent(ExecutionContext* context) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  PerformanceEvent* event = datastore_->NextPerformanceEvent(replay_frame);

  if (!event) {
    replay_frame->DivergenceDetected(
        "Requestor: " + replay_frame->GetJSReplayer()->CurrentExecutingEvent() +
        "No performance now event found.");
    return 999999999;
  }
  VLOG(7) << "Performance now recorder called!" << event->now();
  return event->now();
}

std::pair<bool, String> ForensicReplayEngine::ReplayGetterEventInternal(
    ExecutionContext* context,
    String interface,
    String attribute) {
  ReplayFrame* replay_frame = CurrentFrame(context);
  GetterEvent* event =
      datastore_->NextGetterEvent(replay_frame, interface, attribute);

  if (!event) {
    replay_frame->DivergenceDetected(
        "Requestor: " + replay_frame->GetJSReplayer()->CurrentExecutingEvent() +
        "No Getter event found: " + interface + "." + attribute);
    return std::pair<bool, String>(false, "0");
  }

  if (interface != event->Interface() || attribute != event->Attribute()) {
    replay_frame->DivergenceDetected(
        "Expected: " + event->ToString() + " Actual: " + interface + "." +
        attribute + " Requestor: " +
        replay_frame->GetJSReplayer()->CurrentExecutingEvent());
    return std::pair<bool, String>(false, "0");
  }

  VLOG(7) << " Injecting GetterEvent: " << event->ToString();
  return std::pair<bool, String>(true, event->ReturnValue());
}

}  // namespace blink
