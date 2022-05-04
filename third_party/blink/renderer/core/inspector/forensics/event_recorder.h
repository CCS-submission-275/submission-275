//
// Created by  on 11/24/20.
//
#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_RECORDER_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/events/event.h"
#include "third_party/blink/renderer/core/dom/events/event_dispatcher.h"
#include "third_party/blink/renderer/core/dom/events/event_listener.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "third_party/blink/renderer/platform/json/json_values.h"
#include "third_party/blink/renderer/bindings/core/v8/serialization/transferables.h"

namespace blink {
class UIEventWithKeyState;
class UIEvent;
class SerializedScriptValue;
class UserActivation;
class LocalDOMWindow;


struct ForensicJSEventData {
  String eventType;
  int line;
  int column;
  int scriptId;
};

struct ForensicEventBase {
  bool bubbles;
  bool cancelable;
  bool composed;
};

struct ForensicUIEventBaseData {
  int detail;
  bool hasSourceCapabilities;
  bool firesTouchEvents;
};

struct ForensicModifierInfo {
  bool altKey;
  bool ctrlKey;
  bool metaKey;
  bool shiftKey;
};

struct ForensicProgressEventData {
  uint64_t loaded;
  uint64_t total;
  bool lengthComputable;
};

struct ForensicMouseEvent {
  double screenX;
  double screenY;
  double clientX;
  double clientY;
  int movementX;
  int movementY;
  String region;
  int16_t button;
  uint16_t buttons;
  int menuSourceType;
  int syntheticEventType;
  int relatedTargetId;
};

using ForensicPointerEvent = ForensicMouseEvent;

struct ForensicKeyEvent {
  String code;
  String key;
  int location;
  bool isComposing;
  int charCode;
  int keyCode;
  bool repeat;
};

struct ForensicFocusData {
  int relatedTargetId;
};


struct ForensicStorageData {
  String key;
  String oldValue;
  String newValue;
  String url;
  const char* storageType;
};

struct ForensicBeforeUnloadData {
  String returnValue;
};

struct ForensicMessageEvent {
  String message;
  String source_origin;
  String last_event_id;
  String source_frame_id;
  bool transfer_user_activation;
  bool allow_autoplay;
  // UserActivation
  bool has_user_active;
  bool has_been_active;
  bool is_active;
  int message_id;
  // MessagePorts
  int port_cnt;
  Vector<int> message_port_ids;
};

struct ForensicEventData {
  String eventType;
  double eventTimestamp;
  String interfaceName;
  String targetInterfacename;
  bool isDefaultHandler;
  ForensicEventBase event_base;
  ForensicUIEventBaseData ui_event_base;
  ForensicModifierInfo modifier_info;
  ForensicMouseEvent mouse_event;
  ForensicKeyEvent key_event;
  ForensicFocusData focus_event;
  ForensicPointerEvent pointer_event;
  ForensicStorageData storage_event;
  ForensicProgressEventData progress_event;
  ForensicMessageEvent message_event;
  ForensicBeforeUnloadData before_unload_event;
};

class CORE_EXPORT EventRecorder : ForensicRecorder {
  DISALLOW_COPY(EventRecorder);

 public:
  static void RecordDispatchEventStart(Event* event,
                                       bool is_default_handler=false);
  static void RecordDispatchEventEnd(Event* event,
                                     bool is_default_handler=false);

  static void RecordPostMessageEvent(
      LocalDOMWindow* window,
      int message_id,
      //MessagePortArray* ports,
      scoped_refptr<SerializedScriptValue> message,
      String source_origin,
      EventTarget* source,
      UserActivation* user_activation,
      bool transfer_user_activation,
      bool allow_autoplay);

 private:
  // static void AddNodeInfo(Node* node, std::unique_ptr<JSONObject>& message);
  static void RecordMouseEvent(Event* event,
                               ForensicEventData* message);
  static void RecordPointerEvent(Event* event,
                                 ForensicEventData* message);
  static void RecordKeyboardEvent(Event* event,
                                  ForensicEventData* message);
  static void RecordFocusEvent(Event* event,
                               ForensicEventData* message);
  static void RecordWheelEvent(Event* event,
                               ForensicEventData* message);
  static void RecordUIEvent(Event* event,
                            ForensicEventData* message);
  static void RecordSubmitEvent(Event* event,
                                ForensicEventData* message);
  static inline void RecordModifierInfo(
      UIEventWithKeyState* event,
      ForensicEventData* message);
  static inline void RecordEventBase(
      Event* event,
      ForensicEventData* message);
  static inline void RecordUIEventBase(
      UIEvent* event,
      ForensicEventData* message);
  // not implemented
  static void RecordInputEvent(Event* event,
                               ForensicEventData* message);
  static void RecordTextEvent(Event* event,
                              ForensicEventData* message);
  static void RecordDragEvent(Event* event,
                              ForensicEventData* message);
  static void RecordGestureEvent(Event* event,
                                 ForensicEventData* message);
  static void RecordTouchEvent(Event* event,
                               ForensicEventData* message);
  static void RecordCompositionEvent(
      Event* event,
      ForensicEventData* message);
  static void RecordErrorEvent(Event* event,
                               ForensicEventData* message);
  static void RecordBeforeTextInsertedEvent(
      Event* event,
      ForensicEventData* message);
  static void RecordClipboardEvent(Event* event,
                                   ForensicEventData* message);
  static void RecordBeforeCreatePolicyEvent(
      Event* event,
      ForensicEventData* message);
  static void RecordBeforeUnloadEvent(
      Event* event,
      ForensicEventData* message);
  static void RecordFormDataEvent(Event* event,
                                  ForensicEventData* message);
  static void RecordMessageEvent(Event* event,
                                 ForensicEventData* message);
  static void RecordStorageEvent(Event* event,
                                 ForensicEventData* message);
  static void RecordProgressEvent(Event* event,
                                  ForensicEventData* message);

  static void RecordDispatchEventStartInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      String record_id,
      int target_id,
      String node_str,
      std::unique_ptr<ForensicEventData> event_data);

  static void RecordDispatchEventEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      String record_id,
      int target_id,
      std::unique_ptr<ForensicEventData> event_data);

  static void RecordPostMessageEventInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    String record_id,
    int target_id,
    std::unique_ptr<ForensicEventData> event_data);

  static void AddUIEventBaseInfoToMessage(
      std::unique_ptr<ForensicEventData>& event,
      std::unique_ptr<JSONObject>& message);

  static void AddMouseEventInfoToMessage(
          std::unique_ptr<ForensicEventData>& event,
          std::unique_ptr<JSONObject>& message);

  static void AddKeyEventInfoToMessage(
          std::unique_ptr<ForensicEventData>& event,
          std::unique_ptr<JSONObject>& message);

  static void AddPointerEventInfoToMessage(
          std::unique_ptr<ForensicEventData>& event,
          std::unique_ptr<JSONObject>& message);

  static void AddFocusEventInfoToMessage(
          std::unique_ptr<ForensicEventData>& event,
          std::unique_ptr<JSONObject>& message);

  static void AddModifierEventInfoToMessage(
          std::unique_ptr<ForensicEventData>& event,
          std::unique_ptr<JSONObject>& message);

  static void AddStorageEventInfoToMessage(
      std::unique_ptr<ForensicEventData>& event,
      std::unique_ptr<JSONObject>& message);

  static void AddProgressEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message);

  static void AddMessageEventInfoToMessage(
    std::unique_ptr<ForensicEventData>& event,
    std::unique_ptr<JSONObject>& message);

  static void AddBeforeUnloadEventInfoToMessage(
      std::unique_ptr<ForensicEventData>& event,
      std::unique_ptr<JSONObject>& message);

  static std::unique_ptr<ForensicEventData> copyEventData(Event* event,
                                                          bool is_default_handler);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_RECORDER_H_
