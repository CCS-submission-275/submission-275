#include "third_party/blink/renderer/core/inspector/forensics/event_replayer.h"
#include "base/time/time.h"
#include "third_party/blink/public/common/messaging/message_port_channel.h"
#include "third_party/blink/renderer/bindings/core/v8/serialization/serialized_script_value.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_progress_event_init.h"
#include "third_party/blink/renderer/core/dom/abort_controller.h"
#include "third_party/blink/renderer/core/dom/events/event_dispatch_result.h"
#include "third_party/blink/renderer/core/dom/events/event_dispatcher.h"
#include "third_party/blink/renderer/core/dom/events/event_init.h"
#include "third_party/blink/renderer/core/dom/events/simulated_click_options.h"
#include "third_party/blink/renderer/core/editing/frame_selection.h"
#include "third_party/blink/renderer/core/event_interface_names.h"
#include "third_party/blink/renderer/core/events/before_unload_event.h"
#include "third_party/blink/renderer/core/events/focus_event.h"
#include "third_party/blink/renderer/core/events/focus_event_init.h"
#include "third_party/blink/renderer/core/events/input_event.h"
#include "third_party/blink/renderer/core/events/keyboard_event_init.h"
#include "third_party/blink/renderer/core/events/message_event.h"
#include "third_party/blink/renderer/core/events/mouse_event_init.h"
#include "third_party/blink/renderer/core/events/page_transition_event.h"
#include "third_party/blink/renderer/core/events/progress_event.h"
#include "third_party/blink/renderer/core/events/text_event.h"
#include "third_party/blink/renderer/core/events/web_input_event_conversion.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/user_activation.h"
#include "third_party/blink/renderer/core/html/forms/html_form_control_element.h"
#include "third_party/blink/renderer/core/html/forms/html_input_element.h"
#include "third_party/blink/renderer/core/input/input_device_capabilities.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_xml_http_request.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/modules/event_interface_modules_names.h"
#include "third_party/blink/renderer/modules/event_modules.h"
#include "third_party/blink/renderer/modules/storage/dom_window_storage.h"
#include "third_party/blink/renderer/modules/storage/storage_event.h"
#include "third_party/blink/renderer/modules/storage/storage_event_init.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"

namespace blink {

void EventReplayer::ReconstructEventBase(JSEvent* js_event, EventInit* init) {
  init->setBubbles(js_event->log_event->getBubbles());
  init->setCancelable(js_event->log_event->getCancelable());
  init->setComposed(js_event->log_event->getComposed());
}

void EventReplayer::ReconstructUIEventBase(ReplayFrame* frame,
                                           JSEvent* js_event,
                                           UIEventInit* init) {
  init->setDetail(js_event->log_event->getDetail(0));
  init->setView(frame->GetFrame()->DomWindow()->window());
  InputDeviceCapabilities* input = InputDeviceCapabilities::Create(
      js_event->log_event->getFireTouchEvents(false));
  init->setSourceCapabilities(input);
}

void EventReplayer::ReconstructEventModifierInit(JSEvent* js_event,
                                                 EventModifierInit* init) {
  init->setAltKey(js_event->log_event->getAltKey(false));
  init->setCtrlKey(js_event->log_event->getCtrlKey(false));
  init->setMetaKey(js_event->log_event->getMetaKey(false));
  init->setShiftKey(js_event->log_event->getShiftKey(false));
}

Event* EventReplayer::ReconstructEvent(JSEvent* js_event) {
  EventInit* init = EventInit::Create();
  ReconstructEventBase(js_event, init);
  return Event::Create(js_event->log_event->getJsEventType().Latin1().c_str(),
                       init);
}

ProgressEvent* EventReplayer::ReconstructProgressEvent(ReplayFrame* frame,
                                                       JSEvent* js_event) {
  ProgressEventInit* init = ProgressEventInit::Create();
  ReconstructEventBase(js_event, init);
  init->setLoaded(js_event->log_event->getLoaded(0));
  init->setTotal(js_event->log_event->getTotal(0));
  init->setLengthComputable(js_event->log_event->getLengthComputable(true));
  ProgressEvent* event = MakeGarbageCollected<ProgressEvent>(
      js_event->log_event->getJsEventType().Latin1().c_str(), init);
  return event;
}

BeforeUnloadEvent* EventReplayer::ReconstructBeforeUnloadEvent(
    ReplayFrame* frame,
    JSEvent* js_event) {
  auto* event = BeforeUnloadEvent::Create();
  event->initEvent(js_event->log_event->getJsEventType().Latin1().c_str(),
                   js_event->log_event->getBubbles(),
                   js_event->log_event->getCancelable());
  event->setReturnValue(
      js_event->log_event->getReturnValue("Forensic<Invalid>").IsolatedCopy());
  return event;
}

FocusEvent* EventReplayer::ReconstructFocusEvent(ReplayFrame* frame,
                                                 JSEvent* js_event) {
  FocusEventInit* init = FocusEventInit::Create();
  ReconstructEventBase(js_event, init);
  ReconstructUIEventBase(frame, js_event, init);
  if (js_event->log_event->getRelatedTargetId(-1) != -1) {
    Node* node =
        frame->GetNodeById(js_event->log_event->getRelatedTargetId(-1));
    if (node)
      init->setRelatedTarget(node);
    else
      VLOG(7) << "Failed to find related target: " << js_event->ToString();
  }
  return FocusEvent::Create(
      js_event->log_event->getJsEventType().Latin1().c_str(), init);
}

MouseEvent* EventReplayer::ReconstructMouseEvent(ReplayFrame* frame,
                                                 JSEvent* js_event) {
  MouseEventInit* init = MouseEventInit::Create();
  ReconstructEventBase(js_event, init);
  ReconstructUIEventBase(frame, js_event, init);
  ReconstructEventModifierInit(js_event, init);
  init->setButton(js_event->log_event->getButton(0));
  init->setButtons(js_event->log_event->getButtons(0));
  init->setClientX(js_event->log_event->getClientX(0));
  init->setClientY(js_event->log_event->getClientY(0));
  init->setScreenX(js_event->log_event->getScreenX(0));
  init->setScreenY(js_event->log_event->getScreenY(0));
  init->setMovementX(js_event->log_event->getMovementX(0));
  init->setMovementY(js_event->log_event->getMovementY(0));
  init->setRegion(js_event->log_event->getRegion("DefaultRegion"));

  if (js_event->log_event->getRelatedTargetId(-1) != -1) {
    Node* node =
        frame->GetNodeById(js_event->log_event->getRelatedTargetId(-1));
    if (node)
      init->setRelatedTarget(node);
    else
      VLOG(7) << "Failed to find related target: " << js_event->ToString();
  }

  // TODO(): We need to save the timestamp before we can create MouseEvent
  // object.
  // Currently, we are using the current time.
  MouseEvent* event = MouseEvent::Create(
      js_event->log_event->getJsEventType().Latin1().c_str(), init,
      base::TimeTicks::Now(),
      MouseEvent::SyntheticEventType(
          js_event->log_event->getSyntheticEventType(0)),
      WebMenuSourceType(js_event->log_event->getMenuSourceType(0)));

  return event;
}

KeyboardEvent* EventReplayer::ReconstructKeyboardEvent(ReplayFrame* frame,
                                                       JSEvent* js_event) {
  KeyboardEventInit* init = KeyboardEventInit::Create();
  ReconstructEventBase(js_event, init);
  ReconstructUIEventBase(frame, js_event, init);
  ReconstructEventModifierInit(js_event, init);
  init->setCharCode(js_event->log_event->getCharCode(0));
  init->setCode(js_event->log_event->getCode("Invalid"));
  init->setIsComposing(js_event->log_event->getIsComposing(false));
  init->setKey(js_event->log_event->getKey("Invalid"));
  init->setKeyCode(js_event->log_event->getKeyCode(0));
  init->setLocation(js_event->log_event->getLocation(0));
  init->setRepeat(js_event->log_event->getRepeat(false));

  KeyboardEvent* event = MakeGarbageCollected<KeyboardEvent>(
      js_event->log_event->getJsEventType().Latin1().c_str(), init);
  return event;
}

MessageEvent* EventReplayer::ReconstructMessageEvent(ReplayFrame* frame,
                                                     JSEvent* js_event) {
  // TODO(): Implement UserActivation.
  VLOG(7) << "Reconstructing message event: " << js_event->ToString()
          << " Message Data: "
          << js_event->log_event->getMessage("ForensicInvalid");

  // Setup the message port array.
  auto* message_port_array = MakeGarbageCollected<MessagePortArray>();
  for (int idx = 0; idx < js_event->log_event->getPortCnt(0); idx++) {
    // int message_port_id =
    // js_event->log_event->getMessagePortIds(nullptr)[idx][0];
    auto* port = MakeGarbageCollected<MessagePort>(
        *frame->GetFrame()->DomWindow()->GetExecutionContext());
    message_port_array->push_back(port);
  }

  auto* message_event = MessageEvent::Create(
      message_port_array,
      SerializedScriptValue::Create(
          js_event->log_event->getMessage("ForensicsInvalid")),
      js_event->log_event->getSourceOrigin("ForensicsInvalid"),
      js_event->log_event->getLastEventId("ForensicsInvalid"));

  return message_event;
}

PageTransitionEvent* EventReplayer::ReconstructPageTransitionEvent(
    ReplayFrame* frame,
    JSEvent* js_event) {
  return PageTransitionEvent::Create(
      js_event->log_event->getJsEventType().Latin1().c_str(), false);
}

StorageEvent* EventReplayer::ReconstructStorageEvent(ReplayFrame* frame,
                                                     JSEvent* js_event) {
  VLOG(7) << "Reconstructing storage event: " << js_event->ToString();
  StorageEventInit* init = StorageEventInit::Create();
  ReconstructEventBase(js_event, init);
  init->setKey(js_event->log_event->getKey("Invalid"));
  init->setOldValue(js_event->log_event->getOldValue("Invalid"));
  init->setNewValue(js_event->log_event->getNewValue("Invalid"));
  init->setUrl(js_event->log_event->getUrl("Invalid"));
  String storage_type = js_event->log_event->getStorageType("Invalid");

  // Build exception
  const ExceptionState::ContextType exception_state_context_type =
      ExceptionState::kGetterContext;
  ExceptionState exception_state(frame->GetFrame()->DomWindow()->GetIsolate(),
                                 exception_state_context_type, "Window",
                                 storage_type.Latin1().c_str());

  // Get the storage area.
  StorageArea* storage_area = nullptr;
  if (storage_type == "localStorage")
    storage_area = DOMWindowStorage::localStorage(
        *frame->GetFrame()->DomWindow(), exception_state);
  else if (storage_type == "sessionStorage")
    storage_area = DOMWindowStorage::sessionStorage(
        *frame->GetFrame()->DomWindow(), exception_state);
  else
    NOTREACHED() << "Invalid storage event: " << js_event->ToString();

  init->setStorageArea(storage_area);
  StorageEvent* event = MakeGarbageCollected<StorageEvent>(
      js_event->log_event->getJsEventType().Latin1().c_str(), init);
  return event;
}

base::TimeTicks EventReplayer::FireEvent(ReplayFrame* frame,
                                         JSEvent* js_event) {
  Event* event = nullptr;
  String interface = js_event->Interface();
  if (interface == event_interface_names::kMouseEvent)
    event = ReconstructMouseEvent(frame, js_event);
  else if (interface == event_interface_names::kKeyboardEvent)
    event = ReconstructKeyboardEvent(frame, js_event);
  else if (interface == event_interface_names::kEvent)
    event = ReconstructEvent(js_event);
  else if (interface == event_interface_names::kStorageEvent)
    event = ReconstructStorageEvent(frame, js_event);
  else if (interface == event_interface_names::kProgressEvent)
    event = ReconstructProgressEvent(frame, js_event);
  else if (interface == event_interface_names::kMessageEvent)
    event = ReconstructMessageEvent(frame, js_event);
  else if (interface == event_interface_names::kBeforeUnloadEvent)
    event = ReconstructBeforeUnloadEvent(frame, js_event);
  else if (interface == event_interface_names::kFocusEvent)
    event = ReconstructFocusEvent(frame, js_event);
  else if (interface == event_interface_names::kPageTransitionEvent)
    event = ReconstructPageTransitionEvent(frame, js_event);
  else {
    frame->DivergenceDetected(
        "Interface type not supported (unsuported interface): " +
        js_event->ToString());
    return base::TimeTicks::Now();
  }


  if (!event) {
    frame->DivergenceDetected(
        "Failed to reconstruct Event: " + js_event->ToString());
    return base::TimeTicks::Now();
  }

  if (js_event->log_event->getJsEventType() == "unload")
    return base::TimeTicks::Now();

  VLOG(7) << "Dispatching event: " << js_event->ToString()
          << " event_interface_name: " << event->InterfaceName()
          << " event_type: " << event->type() << " js_event_interface: "
          << interface << " js_event_target_interface: "
          << js_event->EventTargetInterface();
  event->SetReplayEvent(true);
  event->SetTrusted(true);

  if (js_event->EventTargetInterface() == "Node") {
    // Handle firing events at DOM nodes.
    // The #document is also a node here.
    // events related to #document fire here.
    Node* node = frame->GetNodeById(js_event->TargetId());
    if (node) {
      VLOG(7) << "Node found: " << js_event->ToString();
      VLOG(7) << " New node id: " << IdentifiersFactory::IntIdForNode(node);
      VLOG(7) << " Node: " << node->ToString();
      node->DispatchEvent(*event);
    } else {
      frame->DivergenceDetected("Failed to find eventTarget(node) for event: " +
                                js_event->ToString());
    }
  } else if (js_event->EventTargetInterface() == "XMLHttpRequest") {
    // Handle at firing events at XML objects.
    auto* replay_xml = frame->GetReplayXMLHttpRequestById(js_event->TargetId());
    if (replay_xml) {
      VLOG(7) << "Dispatching XMLHTTPRequest event: " << js_event->ToString();
      replay_xml->Dispatch(event);
    } else {
      VLOG(7)
          << "XMLHTTPRequest Event Failed to replay, event never registered: "
          << js_event->ToString();
      ForensicRecorder::ReplayError(
          frame->GetFrame(),
          "XMLHTTPRequest Event Failed to replay, event never registered: " +
              js_event->ToString());
    }
  } else if (js_event->EventTargetInterface() == "DOMWindow") {
    // Handle firing events at dom window.
    frame->GetFrame()->DomWindow()->DispatchEvent(*event);
  } else if (js_event->EventTargetInterface() == "MessagePort") {
    if (auto* message_port = frame->GetMessagePortById(js_event->TargetId()))
      message_port->DispatchEvent(*event);
    else
      frame->DivergenceDetected("Failed to Replay Event: " +
                                js_event->ToString());
  } else if (js_event->EventTargetInterface() == "AbortSignal") {
    auto* abort =
        AbortController::Create(frame->GetDocument()->ToExecutionContext());
    abort->abort();
  } else {
    frame->DivergenceDetected("Failed to find targetInterface: " +
                              js_event->ToString());
  }
  return base::TimeTicks::Now();
}

void EventReplayer::Trace(Visitor* visitor) {
  visitor->Trace(node_map_);
  visitor->Trace(js_replayer_);
}
}  // namespace blink
