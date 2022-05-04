#include "third_party/blink/renderer/core/inspector/forensics/event_recorder.h"
#include <memory>
#include "base/bind.h"
#include "third_party/blink/renderer/bindings/core/v8/js_based_event_listener.h"
#include "third_party/blink/renderer/bindings/core/v8/source_location.h"
#include "third_party/blink/renderer/core/core_probes_inl.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/event_interface_names.h"
#include "third_party/blink/renderer/core/events/before_unload_event.h"
#include "third_party/blink/renderer/core/events/focus_event.h"
#include "third_party/blink/renderer/core/events/keyboard_event.h"
#include "third_party/blink/renderer/core/events/message_event.h"
#include "third_party/blink/renderer/core/events/mouse_event.h"
#include "third_party/blink/renderer/core/events/pointer_event.h"
#include "third_party/blink/renderer/core/events/progress_event.h"
#include "third_party/blink/renderer/core/events/wheel_event.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/html/forms/submit_event.h"
#include "third_party/blink/renderer/core/input/input_device_capabilities.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/modules/event_modules.h"
#include "third_party/blink/renderer/modules/storage/storage_area.h"
#include "third_party/blink/renderer/modules/storage/storage_event.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"

#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"

#include "third_party/blink/renderer/bindings/core/v8/serialization/serialized_script_value.h"
#include "third_party/blink/renderer/core/frame/user_activation.h"

#include "base/time/time.h"

namespace blink {

// Post Message is a corner case, because we have to record the  vent
// before the actual event is created.
void EventRecorder::RecordPostMessageEvent(
    LocalDOMWindow* window,
    int message_id,
    // MessagePortArray* ports,
    scoped_refptr<SerializedScriptValue> message,
    String source_origin,
    EventTarget* source,
    UserActivation* user_activation,
    bool transfer_user_activation,
    bool allow_autoplay) {
  _IFA_DISABLE_
  auto event_data = std::make_unique<ForensicEventData>();
  event_data->eventTimestamp = base::Time::Now().ToDoubleT();
  event_data->eventType = event_type_names::kMessage;
  event_data->targetInterfacename = event_interface_names::kMessageEvent;
  // event base
  event_data->event_base.bubbles = false;
  event_data->event_base.cancelable = false;
  event_data->event_base.composed = false;
  // message data
  event_data->message_event.message = message->ToWireString().IsolatedCopy();
  event_data->message_event.source_origin = source_origin.IsolatedCopy();
  event_data->message_event.transfer_user_activation = transfer_user_activation;
  event_data->message_event.allow_autoplay = allow_autoplay;
  // user activation.
  if (user_activation) {
    event_data->message_event.has_been_active =
        user_activation->hasBeenActive();
    event_data->message_event.is_active = user_activation->isActive();
    event_data->message_event.has_user_active = true;
  } else {
    event_data->message_event.has_been_active = false;
    event_data->message_event.is_active = false;
    event_data->message_event.has_user_active = false;
  }
  // message ports
  // event_data->message_event.port_cnt = ports.size();
  // Get source's frame id.
  String source_frameId;
  ExecutionContext* context = source->GetExecutionContext();
  if (LocalDOMWindow* source_window = context->ExecutingWindow()) {
    event_data->message_event.source_frame_id =
        IdentifiersFactory::FrameId(source_window->GetFrame());
  } else
    NOTREACHED();

  VLOG(7) << "message Data: " << message->Data();
  VLOG(7) << "message length: " << message->DataLengthInBytes();
  VLOG(7) << "packed: " << message->HasPackedContents();

  // Get the receiver's frameId.
  int targetId;
  String frameId, recordId;
  targetId = source->TargetId();
  LocalFrame* frame = window->GetFrame();
  recordId = window->GetFrame()->GetReplayId().IsolatedCopy();

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&EventRecorder::RecordPostMessageEventInternal,
                     GetThreadData(), copyFrameData(frame),
                     recordId.IsolatedCopy(), targetId, std::move(event_data)));
}

void EventRecorder::RecordPostMessageEventInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    String record_id,
    int target_id,
    std::unique_ptr<ForensicEventData> event_data) {
  VLOG(7) << "MessageEventInterval";
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("recordId", record_id);
  message->SetInteger("targetId", target_id);
  message->SetString("targetInterfaceName", event_data->targetInterfacename);
  // Add event base information.
  message->SetString("eventType", event_data->eventType);
  message->SetString("interfaceName", event_data->interfaceName);
  message->SetBoolean("bubbles", event_data->event_base.bubbles);
  message->SetBoolean("cancelable", event_data->event_base.cancelable);
  message->SetBoolean("composed", event_data->event_base.composed);
  AddThreadData(thread_data, message);
  // Add message info.
  message->SetString("message", event_data->message_event.message);
  message->SetInteger("messageId", event_data->message_event.message_id);
  message->SetString("sourceOrigin", event_data->message_event.source_origin);
  message->SetString("sourceFrameId",
                     event_data->message_event.source_frame_id);
  message->SetBoolean("allowAutoplay",
                      event_data->message_event.allow_autoplay);
  message->SetBoolean("hasUserActive",
                      event_data->message_event.has_user_active);
  message->SetBoolean("isActive", event_data->message_event.is_active);
  message->SetBoolean("hasBeenActive",
                      event_data->message_event.has_been_active);
  message->SetBoolean("transferUserActivation",
                      event_data->message_event.transfer_user_activation);

  // Log message.
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void EventRecorder::RecordDispatchEventStartInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    String record_id,
    int target_id,
    String node_str,
    std::unique_ptr<ForensicEventData> event_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  // NOTE: We can reduce this to only include the Frame's ID which is all that
  // we need for event recordings.
  message->SetString("recordId", record_id);
  message->SetInteger("targetId", target_id);
  message->SetString("nodeStr", node_str);
  message->SetString("targetInterfaceName", event_data->targetInterfacename);
  // This node represents the event's target.
  // Add event base information.
  message->SetString("eventType", event_data->eventType);
  message->SetString("interfaceName", event_data->interfaceName);
  message->SetBoolean("bubbles", event_data->event_base.bubbles);
  message->SetBoolean("cancelable", event_data->event_base.cancelable);
  message->SetBoolean("composed", event_data->event_base.composed);
  message->SetBoolean("isDefaultHandler", event_data->isDefaultHandler);
  AddThreadData(thread_data, message);

  // Add additional event information based on event type.
  if (event_data->interfaceName == "MouseEvent")
    AddMouseEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "PointerEvent")
    AddPointerEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "KeyboardEvent")
    AddKeyEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "FocusEvent")
    AddFocusEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "StorageEvent")
    AddStorageEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "ProgressEvent")
    AddProgressEventInfoToMessage(event_data, message);
  // else if (event_data->interfaceName == "InputEvent")
  //   AddInputEventInfoToMessage(event_data, message);
  // else if (event_data->interfaceName == "TextEvent")
  //   AddTextEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "MessageEvent")
    AddMessageEventInfoToMessage(event_data, message);
  else if (event_data->interfaceName == "BeforeUnloadEvent")
    AddBeforeUnloadEventInfoToMessage(event_data, message);

  // Log message.
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void EventRecorder::RecordDispatchEventEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    String record_id,
    int target_id,
    std::unique_ptr<ForensicEventData> event_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  // NOTE: We can reduce this to only include the Frame's ID which is all that
  // we need for event recordings.
  message->SetString("recordId", record_id);
  message->SetInteger("targetId", target_id);
  message->SetString("targetInterfaceName", event_data->targetInterfacename);
  // This node represents the event's target.
  message->SetString("eventType", event_data->eventType);
  message->SetString("interfaceName", event_data->interfaceName);
  message->SetBoolean("bubbles", event_data->event_base.bubbles);
  message->SetBoolean("cancelable", event_data->event_base.cancelable);
  message->SetBoolean("composed", event_data->event_base.composed);
  message->SetBoolean("isDefaultHandler", event_data->isDefaultHandler);
  AddThreadData(thread_data, message);

  // Log message.
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void EventRecorder::RecordDispatchEventStart(Event* event,
                                             bool is_default_handler) {
  TRACE_EVENT0("blink.forensic_recording", __func__);

  _IFA_DISABLE_
  // we add a property into event object to prevent duplicate recording
  VLOG(7) << __func__ << " " << event->type() << " "
          << event->target()->InterfaceName();
  if (event->wasRecorded())
    return;

  int targetId;
  String recordId;
  String node_str = "";
  LocalFrame* frame = nullptr;
  // Node* node = event->target()->ToNode();
  if (Node* node = event->target()->ToNode()) {
    node_str = node->ToString();
    targetId = IdentifiersFactory::IntIdForNode(node);
    frame = node->GetDocument().GetFrame();
    recordId = frame ? frame->GetReplayId() : "RecordId";
  } else if (MessagePort* port = event->target()->ToMessagePort()) {
    targetId = port->GetMessagePortId();
    frame = port->GetExecutionContext()->ExecutingWindow()->GetFrame();
    recordId = frame->GetReplayId();
    node_str = "<Forensics>MessagePort";
  } else {
    ExecutionContext* context = event->target()->GetExecutionContext();
    if (LocalDOMWindow* window = context->ExecutingWindow()) {
      targetId = event->target()->TargetId();
      frame = window->GetFrame();
      recordId = window->GetFrame()->GetReplayId();
      node_str = "<Forensics>WINDOW";
    }
  }

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&EventRecorder::RecordDispatchEventStartInternal,
                     GetThreadData(), copyFrameData(frame),
                     recordId.IsolatedCopy(), targetId, node_str.IsolatedCopy(),
                     copyEventData(event, is_default_handler)));
}

void EventRecorder::RecordDispatchEventEnd(Event* event,
                                           bool is_default_handler) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  // we add a property into event object to prevent duplicate recording
  if (event->wasRecorded())
    return;
  event->SetRecorded(true);
  int targetId;
  String recordId;
  LocalFrame* frame = nullptr;
  Node* node = event->target()->ToNode();
  if (node) {
    targetId = IdentifiersFactory::IntIdForNode(node);
    frame = node->GetDocument().GetFrame();
    recordId = frame ? frame->GetReplayId() : "RecordId";
  } else if (MessagePort* port = event->target()->ToMessagePort()) {
    targetId = port->GetMessagePortId();
    frame = port->GetExecutionContext()->ExecutingWindow()->GetFrame();
    recordId = frame->GetReplayId();
  } else {
    ExecutionContext* context = event->target()->GetExecutionContext();
    if (LocalDOMWindow* window = context->ExecutingWindow()) {
      targetId = event->target()->TargetId();
      frame = window->GetFrame();
      recordId = window->GetFrame()->GetReplayId().IsolatedCopy();
    }
  }

  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&EventRecorder::RecordDispatchEventEndInternal,
                                GetThreadData(), copyFrameData(frame),
                                recordId.IsolatedCopy(), targetId,
                                copyEventData(event, is_default_handler)));
 probe::DidExecuteJSEvent(frame, event->type(), event->InterfaceName(),
                          event->target()->InterfaceName());
}

void EventRecorder::RecordUIEvent(Event* event, ForensicEventData* message) {
  UIEvent* ui_event = DynamicTo<UIEvent>(event);
  RecordUIEventBase(ui_event, std::move(message));
  if (event->IsPointerEvent()) {
    message->interfaceName =
        event_interface_names::kPointerEvent.GetString().IsolatedCopy();
    RecordPointerEvent(event, std::move(message));
  } else if (event->IsWheelEvent()) {
    message->interfaceName =
        event_interface_names::kWheelEvent.GetString().IsolatedCopy();
    RecordWheelEvent(event, std::move(message));
  } else if (event->IsMouseEvent()) {
    message->interfaceName =
        event_interface_names::kMouseEvent.GetString().IsolatedCopy();
    RecordMouseEvent(event, std::move(message));
  } else if (event->IsKeyboardEvent()) {
    message->interfaceName =
        event_interface_names::kKeyboardEvent.GetString().IsolatedCopy();
    RecordKeyboardEvent(event, std::move(message));
  } else if (event->IsFocusEvent()) {
    message->interfaceName =
        event_interface_names::kFocusEvent.GetString().IsolatedCopy();
    RecordFocusEvent(event, std::move(message));
  }
  // else if (event->IsInputEvent()) {
  //   message->interfaceName =
  //   event_interface_names::kInputEvent.GetString().IsolatedCopy();
  //   RecordInputEvent(event, std::move(message));
  // } else if (IsTextEvent(*event)) {
  //     message->interfaceName =
  //     event_interface_names::kTextEvent.GetString().IsolatedCopy();
  //     RecordTextEvent(event, std::move(message));
  // }
}

void EventRecorder::RecordStorageEvent(Event* event,
                                       ForensicEventData* message) {
  if (!event)
    return;
  auto* storage_event = DynamicTo<StorageEvent>(event);
  message->interfaceName = event->InterfaceName().GetString().IsolatedCopy();
  message->storage_event.key = storage_event->key().IsolatedCopy();
  message->storage_event.oldValue = storage_event->oldValue().IsolatedCopy();
  message->storage_event.newValue = storage_event->newValue().IsolatedCopy();
  message->storage_event.url = storage_event->url().IsolatedCopy();
  message->storage_event.storageType =
      (storage_event->storageArea()->GetStorageType() ==
               StorageArea::StorageType::kLocalStorage
           ? "localStorage"
           : "sessionStorage");
}

void EventRecorder::AddStorageEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetString("key", event->storage_event.key);
  message->SetString("oldValue", event->storage_event.oldValue);
  message->SetString("newValue", event->storage_event.newValue);
  message->SetString("url", event->storage_event.url);
  message->SetString("storageType", event->storage_event.storageType);
}

void EventRecorder::RecordProgressEvent(Event* event,
                                        ForensicEventData* message) {
  if (!event)
    return;
  auto* progress_event = DynamicTo<ProgressEvent>(event);
  message->interfaceName =
      progress_event->InterfaceName().GetString().IsolatedCopy();
  message->progress_event.loaded = progress_event->loaded();
  message->progress_event.total = progress_event->total();
  message->progress_event.lengthComputable = progress_event->lengthComputable();
}

void EventRecorder::AddProgressEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetInteger("loaded", event->progress_event.loaded);
  message->SetInteger("total", event->progress_event.total);
  message->SetBoolean("lengthComputable",
                      event->progress_event.lengthComputable);
}

void EventRecorder::RecordMouseEvent(Event* event, ForensicEventData* message) {
  if (!event)
    return;
  auto* mouse_event = DynamicTo<MouseEvent>(event);
  if (!mouse_event)
    return;
  RecordModifierInfo(mouse_event, std::move(message));
  message->mouse_event.screenX = mouse_event->screenX();
  message->mouse_event.screenY = mouse_event->screenY();
  message->mouse_event.clientX = mouse_event->clientX();
  message->mouse_event.clientY = mouse_event->clientY();
  message->mouse_event.movementX = mouse_event->movementX();
  message->mouse_event.movementY = mouse_event->movementY();
  message->mouse_event.region = mouse_event->region().IsolatedCopy();
  message->mouse_event.button = mouse_event->button();
  message->mouse_event.buttons = mouse_event->buttons();
  message->mouse_event.menuSourceType =
      static_cast<int>(mouse_event->GetMenuSourceType());
  message->mouse_event.syntheticEventType =
      static_cast<int>(mouse_event->GetSyntheticEventType());

  EventTarget* target = mouse_event->target();
  if (target)
    message->mouse_event.relatedTargetId =
        DOMNodeIds::IdForNode(target->ToNode());
  else
    message->mouse_event.relatedTargetId = -1;
}

void EventRecorder::AddMouseEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  AddUIEventBaseInfoToMessage(event, message);
  AddModifierEventInfoToMessage(event, message);
  message->SetDouble("screenX", event->mouse_event.screenX);
  message->SetDouble("screenY", event->mouse_event.screenY);
  message->SetDouble("clientX", event->mouse_event.clientX);
  message->SetDouble("clientY", event->mouse_event.clientY);
  message->SetInteger("movementX", event->mouse_event.movementX);
  message->SetInteger("movementY", event->mouse_event.movementY);
  message->SetString("region", event->mouse_event.region);
  message->SetInteger("button", event->mouse_event.button);
  message->SetInteger("buttons", event->mouse_event.buttons);
  message->SetInteger("menuSourceType", event->mouse_event.menuSourceType);
  message->SetInteger("syntheticEventType",
                      event->mouse_event.syntheticEventType);
  message->SetInteger("relatedTargetId", event->mouse_event.relatedTargetId);
}

void EventRecorder::RecordPointerEvent(Event* event,
                                       ForensicEventData* message) {
  // NOTE: We cannot dyn. cast to mouseEvent
  auto* pointer_event = DynamicTo<PointerEvent>(event);
  RecordModifierInfo(pointer_event, std::move(message));
  message->pointer_event.screenX = pointer_event->screenX();
  message->pointer_event.screenY = pointer_event->screenY();
  message->pointer_event.clientX = pointer_event->clientX();
  message->pointer_event.clientY = pointer_event->clientY();
  message->pointer_event.movementX = pointer_event->movementX();
  message->pointer_event.movementY = pointer_event->movementY();
  message->pointer_event.region = pointer_event->region().IsolatedCopy();
  message->pointer_event.button = pointer_event->button();
  message->pointer_event.buttons = pointer_event->buttons();
  message->pointer_event.menuSourceType =
      static_cast<int>(pointer_event->GetMenuSourceType());
  message->pointer_event.syntheticEventType =
      static_cast<int>(pointer_event->GetSyntheticEventType());

  EventTarget* target = pointer_event->relatedTarget();
  if (target)
    message->pointer_event.relatedTargetId =
        DOMNodeIds::IdForNode(target->ToNode());
  else
    message->pointer_event.relatedTargetId = -1;

  // message->width = pointer_event->width();
  // message->height = pointer_event->height();
  // message->pressure = pointer_event->pressure();
  // message->tiltX = pointer_event->tiltX();
  // message->tiltY = pointer_event->tiltY();
  // message->isPrimary = pointer_event->isPrimary();
  // message->tangentialPressure = pointer_event->tangentialPressure();
  // message->twist = pointer_event->twist();
  // message->pointerType = pointer_event->pointerType();

  // TODO():  coalesced events
  // TODO(): predicted events
}

void EventRecorder::AddPointerEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  AddUIEventBaseInfoToMessage(event, message);
  AddModifierEventInfoToMessage(event, message);
  message->SetDouble("screenX", event->pointer_event.screenX);
  message->SetDouble("screenY", event->pointer_event.screenY);
  message->SetDouble("clientX", event->pointer_event.clientX);
  message->SetDouble("clientY", event->pointer_event.clientY);
  message->SetInteger("movementX", event->pointer_event.movementX);
  message->SetInteger("movementY", event->pointer_event.movementY);
  message->SetString("region", event->pointer_event.region);
  message->SetInteger("button", event->pointer_event.button);
  message->SetInteger("buttons", event->pointer_event.buttons);
  message->SetInteger("menuSourceType", event->pointer_event.menuSourceType);
  message->SetInteger("syntheticEventType",
                      event->pointer_event.syntheticEventType);
  message->SetInteger("relatedTargetId", event->pointer_event.relatedTargetId);
}

void EventRecorder::RecordWheelEvent(Event* event, ForensicEventData* message) {
  auto* wheel_event = DynamicTo<WheelEvent>(event);
  RecordMouseEvent(wheel_event, std::move(message));
  // message->deltaMode = wheel_event->deltaMode();
  // message->deltaX = wheel_event->deltaX();
  // message->deltaY = wheel_event->deltaY();
  // message->deltaZ = wheel_event->deltaZ();
  // message->wheelDeltaX = wheel_event->wheelDeltaX();
  // message->wheelDeltaY = wheel_event->wheelDeltaY();
}

void EventRecorder::RecordEventBase(Event* event, ForensicEventData* message) {
  message->event_base.bubbles = event->bubbles();
  message->event_base.cancelable = event->cancelable();
  message->event_base.composed = event->composed();
}

void EventRecorder::RecordUIEventBase(UIEvent* event,
                                      ForensicEventData* message) {
  message->ui_event_base.detail = event->detail();
  InputDeviceCapabilities* source_capabilities = event->sourceCapabilities();
  if (source_capabilities) {
    message->ui_event_base.hasSourceCapabilities = true;
    message->ui_event_base.firesTouchEvents =
        source_capabilities->firesTouchEvents();
  } else {
    message->ui_event_base.firesTouchEvents = false;
    message->ui_event_base.hasSourceCapabilities = false;
  }
}

void EventRecorder::AddUIEventBaseInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetInteger("detail", event->ui_event_base.detail);
  message->SetBoolean("hasSourceCapabilities",
                      event->ui_event_base.hasSourceCapabilities);
  message->SetBoolean("firesTouchEvents",
                      event->ui_event_base.firesTouchEvents);
}

void EventRecorder::RecordKeyboardEvent(Event* event,
                                        ForensicEventData* message) {
  auto* keyboard_event = DynamicTo<KeyboardEvent>(event);
  DCHECK(keyboard_event);
  RecordModifierInfo(keyboard_event, std::move(message));
  message->key_event.code = keyboard_event->code().IsolatedCopy();
  message->key_event.key = keyboard_event->key().IsolatedCopy();
  message->key_event.location = keyboard_event->location();
  message->key_event.isComposing = keyboard_event->isComposing();
  message->key_event.charCode = keyboard_event->charCode();
  message->key_event.keyCode = keyboard_event->keyCode();
  message->key_event.repeat = keyboard_event->repeat();
}

void EventRecorder::AddKeyEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  AddUIEventBaseInfoToMessage(event, message);
  AddModifierEventInfoToMessage(event, message);
  message->SetString("code", event->key_event.code);
  message->SetString("key", event->key_event.key);
  message->SetInteger("location", event->key_event.location);
  message->SetBoolean("isComposing", event->key_event.isComposing);
  message->SetInteger("charCode", event->key_event.charCode);
  message->SetInteger("keyCode", event->key_event.keyCode);
  message->SetBoolean("repeat", event->key_event.repeat);
}

// void EventRecorder::AddInputEventInfoToMessage(
//         std::unique_ptr<ForensicEventData>& event,
//         std::unique_ptr<JSONObject>& message) {
//   AddUIEventBaseInfoToMessage(event, message);
//   message->SetString("inputType", event->input_event.inputType);
//   message->SetString("data", event->input_event.data);
//   message->SetBoolean("isComposing", event->input_event.isComposing);
// }

// void EventRecorder::AddTextEventInfoToMessage(
//         std::unique_ptr<ForensicEventData>& event,
//         std::unique_ptr<JSONObject>& message) {
//   AddUIEventBaseInfoToMessage(event, message);
//   message->SetString("data", event->input_event.data);
// }

void EventRecorder::RecordModifierInfo(UIEventWithKeyState* event,
                                       ForensicEventData* message) {
  message->modifier_info.altKey = event->altKey();
  message->modifier_info.ctrlKey = event->ctrlKey();
  message->modifier_info.metaKey = event->metaKey();
  message->modifier_info.shiftKey = event->shiftKey();
}

void EventRecorder::AddModifierEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetBoolean("altKey", event->modifier_info.altKey);
  message->SetBoolean("ctrlKey", event->modifier_info.ctrlKey);
  message->SetBoolean("metaKey", event->modifier_info.metaKey);
  message->SetBoolean("shiftKey", event->modifier_info.shiftKey);
}

/*
 *   String message;
  String source_origin;
  String source_frame_id;
  bool transfer_user_activation;
  bool allow_autoplay;
  // UserActivation
  bool has_user_active;
  bool has_been_active;
  bool is_active;
  // MessagePorts
  int port_cnt;
 */

void EventRecorder::RecordMessageEvent(Event* event,
                                       ForensicEventData* message) {
  message->interfaceName = event->InterfaceName().GetString().IsolatedCopy();
  auto* message_event = DynamicTo<MessageEvent>(event);
  message->message_event.source_origin = message_event->origin().IsolatedCopy();
  message->message_event.last_event_id =
      message_event->lastEventId().IsolatedCopy();
  auto* value = message_event->DataAsSerializedScriptValue();

    message->message_event.message = value ? value->ToWireString()
                                       .IsolatedCopy() : "";



  // If any ports are being transferred, add their ids.
  message->message_event.port_cnt = message_event->ports().size();
  for (unsigned int idx = 0; idx < message_event->ports().size(); idx++) {
    message->message_event.message_port_ids.push_back(
        message_event->ports()[idx]->GetMessagePortId());
  }

  // handle user activation.
  UserActivation* user_activation = message_event->userActivation();
  if (user_activation) {
    message->message_event.has_user_active = true;
    message->message_event.has_been_active = user_activation->hasBeenActive();
    message->message_event.is_active = user_activation->isActive();
  } else {
    message->message_event.has_user_active = true;
    message->message_event.has_been_active = false;
    message->message_event.is_active = false;
  }

  // TOOD(): Transferables and
}

void EventRecorder::AddMessageEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetString("sourceOrigin", event->message_event.source_origin);
  message->SetString("lastEventId", event->message_event.last_event_id);
  message->SetString("message", event->message_event.message);
  message->SetBoolean("hasUserActive", event->message_event.has_user_active);
  message->SetBoolean("hasBeenActive", event->message_event.has_been_active);
  message->SetBoolean("isActive", event->message_event.is_active);
  message->SetInteger("portCnt", event->message_event.port_cnt);
  // Add message port ids if any ports are being sent through this message.
  auto message_port_ids = std::make_unique<JSONArray>();
  for (auto message_port_id : event->message_event.message_port_ids) {
    message_port_ids->PushInteger(message_port_id);
  }
  message->SetArray("messagePortIds", std::move(message_port_ids));
}

void EventRecorder::RecordFocusEvent(Event* event, ForensicEventData* message) {
  if (!event)
    return;
  auto* focus_event = DynamicTo<FocusEvent>(event);
  EventTarget* target = focus_event->relatedTarget();
  if (target)
    message->focus_event.relatedTargetId =
        DOMNodeIds::IdForNode(target->ToNode());
  else
    message->focus_event.relatedTargetId = -1;
}

void EventRecorder::AddFocusEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  AddUIEventBaseInfoToMessage(event, message);

  message->SetInteger("relatedTargetId", event->focus_event.relatedTargetId);
}

void EventRecorder::RecordBeforeUnloadEvent(Event* event,
                                            ForensicEventData* message) {
  auto* unload_event = DynamicTo<BeforeUnloadEvent>(event);
  message->interfaceName = event->InterfaceName().GetString().IsolatedCopy();
  message->before_unload_event.returnValue =
      unload_event->returnValue().IsolatedCopy();
}

void EventRecorder::AddBeforeUnloadEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message) {
  message->SetString("returnValue", event->before_unload_event.returnValue);
}

void EventRecorder::RecordSubmitEvent(Event* event,
                                      ForensicEventData* message) {
  //  auto* submit_event = DynamicTo<SubmitEvent>(event);
  //  if (submit_event->hasSubmitterNonNull())
  //    message->submitter =
  //    DOMNodeIds::IdForNode(submit_event->submitter());
  //  else
  //    message->submitter = -1;
}

void EventRecorder::RecordInputEvent(Event* event, ForensicEventData* message) {

}
void EventRecorder::RecordCompositionEvent(Event* event,
                                           ForensicEventData* message) {}
void EventRecorder::RecordTextEvent(Event* event, ForensicEventData* message) {}
void EventRecorder::RecordGestureEvent(Event* event,
                                       ForensicEventData* message) {}
void EventRecorder::RecordTouchEvent(Event* event, ForensicEventData* message) {
}
void EventRecorder::RecordErrorEvent(Event* event, ForensicEventData* message) {
}
void EventRecorder::RecordDragEvent(Event* event, ForensicEventData* message) {}
void EventRecorder::RecordBeforeTextInsertedEvent(Event* event,
                                                  ForensicEventData* message) {}
void EventRecorder::RecordClipboardEvent(Event* event,
                                         ForensicEventData* message) {}
void EventRecorder::RecordBeforeCreatePolicyEvent(Event* event,
                                                  ForensicEventData* message) {}

void EventRecorder::RecordFormDataEvent(Event* event,
                                        ForensicEventData* message) {}

std::unique_ptr<ForensicEventData> EventRecorder::copyEventData(
    Event* event,
    bool is_default_handler) {
  auto event_data = std::make_unique<ForensicEventData>();
  event_data->targetInterfacename =
      event->target()->InterfaceName().GetString().IsolatedCopy();
  event_data->eventType = event->type().GetString().IsolatedCopy();
  event_data->eventTimestamp = base::Time::Now().ToDoubleT();
  event_data->isDefaultHandler = is_default_handler;

  RecordEventBase(event, event_data.get());
  // Dispatch event based on event type.
  if (event->IsUIEvent())
    RecordUIEvent(event, event_data.get());
  else if (event->IsBeforeUnloadEvent())
    RecordBeforeUnloadEvent(event, event_data.get());
  else if (event->IsStorageEvent())
    RecordStorageEvent(event, event_data.get());
  else if (event->IsProgressEvent())
    RecordProgressEvent(event, event_data.get());
  else if (event->IsMessageEvent())
    RecordMessageEvent(event, event_data.get());
  else if (event->IsBeforeUnloadEvent())
    RecordBeforeUnloadEvent(event, event_data.get());
  else
    event_data->interfaceName =
        event->InterfaceName().GetString().IsolatedCopy();
  return event_data;
}

// UIEvents: composition, focus, input, text, UIEventWithKeyState
// UIEventWithKeyState: gesture, keyboard, mouse, touch
// MouseEvent: Drag, Pointer, Wheel

}  // namespace blink
