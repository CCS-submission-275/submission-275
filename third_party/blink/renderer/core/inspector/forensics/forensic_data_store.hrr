#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DATA_STORE_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DATA_STORE_H_

#include <unordered_map>
#include "base/memory/singleton.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_attr_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/getter_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/idle_callback_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/animation_frame_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/navigation_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/performance_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/script_execution_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/storage_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/crypto_random_event.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"

#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"

#include "third_party/blink/renderer/platform/wtf/hash_map.h"

namespace blink {

enum ForensicEventType {
  kEmptyQueue,
  kDOMNode,
  kLoadPage,
  kJSEvent,
  kTimerEvent,
  kIdleCallbackEvent,
  kAnimationFrameEvent,
  kLoadResource,
  kScriptExecutionEvent,
  kCryptoRandom
};
using v8RecordMap = std::unordered_map<std::string, std::vector<double>>;


class GetterQueue : public GarbageCollected<GetterQueue> {

  public:
    void insert(GetterEvent* event);
    GetterEvent* NextGetterEvent(String interface, String attribute);
    void Trace(Visitor* visitor);

  private:
    HeapHashMap<String, HeapDeque<Member<GetterEvent>>> getter_map_;
};

class CORE_EXPORT ForensicDataStore final {
  USING_FAST_MALLOC(ForensicDataStore);

 public:
  static ForensicDataStore* GetInstance();
  ForensicDataStore();
  void insertDOMNodeEvent(DOMNodeEvent* event);
  void insertJSInsertedIframeEvent(DOMNodeEvent* event);
  void insertVerificationEvent(DOMNodeEvent* event);
  void insertNavigationEvent(NavigationEvent* event);
  void insertDOMAttrEvent(DOMAttrEvent* event);
  void insertJSEvent(JSEvent* event);
  void insertTimerEvent(TimerEvent* event);
  void insertIdleCallbackEvent(IdleCallbackEvent* event);
  void insertAnimationFrameEvent(AnimationFrameEvent* event);
  void insertPerformanceEvent(PerformanceEvent* event);
  void insertGetterEvent(GetterEvent* event);
  void insertLoadResourceIdentifier(String identifier);
  void insertV8Records(const std::unordered_map<std::string, std::vector<double>>& rnd,
                       const std::unordered_map<std::string, std::vector<double>>& ts);
  void insertCryptoRandomEvent(CryptoRandomEvent* event);
  void insertScriptExecutionEvent(ScriptExecutionEvent* event);


  DOMNodeEvent* NextDOMNodePeek() { return dom_node_list_.front(); }
  // Replay Queues.
  DOMNodeEvent* NextDOMNodeEvent();
  JSEvent* NextJSEvent();
  NavigationEvent* NextNavigationEvent();
  ForensicEventType NextForensicEventType();
  IdleCallbackEvent* NextIdleCallbackEvent();
  AnimationFrameEvent* NextAnimationFrameEvent();
  String NextLoadResourceIdentifier();
  ScriptExecutionEvent* NextScriptExecutionEvent();
  DOMNodeEvent* NextJSInsertedIframeEvent(ReplayFrame* frame);
  // Verification Queues.
  DOMNodeEvent* NextNodeVerificationEvent(ReplayFrame* frame);
  DOMAttrEvent* NextAttrVerificationEvent(ReplayFrame* frame);
  PerformanceEvent* NextPerformanceEvent(ReplayFrame* frame);
  GetterEvent* NextGetterEvent(ReplayFrame* frame,
                               String interface, String Attribute);
  TimerEvent* NextTimerEvent();
  String NextCryptoRandomValues(ReplayFrame* frame);
  // Peek at the next timer event's sequence number, but doens't pop it off
  // queue.
  int PeekTimerEventSequenceNumber(ReplayFrame* frame);
  v8::PlatformInstrumentationData getV8InstrumentData();

  // Storage API Data
  void insertStorageEvent(ForensicStorageEvent* event);
  ForensicStorageEvent* NextStorageEvent(ReplayFrame* frame, String api);

 private:
  Deque<Persistent<DOMNodeEvent>> verification_list_;
  // The set of DOM modifications that need to be verified. (keyed on frameId).
  Deque<Persistent<TimerEvent>> timer_list_;
  Deque<Persistent<IdleCallbackEvent>> idle_callback_list_;
  //AnimationFrameEvent are callbacks registered w/ window.requestAnimationFrame()
  Deque<Persistent<AnimationFrameEvent>> animation_frame_list_;
  Deque<Persistent<DOMNodeEvent>> dom_node_list_;
  Deque<Persistent<DOMAttrEvent>> dom_attr_list_;
  Deque<Persistent<NavigationEvent>> navigation_list_;
  Deque<Persistent<JSEvent>> js_event_list_;
  Deque<String> load_resource_identifier_list_;
  Deque<Persistent<ScriptExecutionEvent>> script_execution_list_;
  Deque<ForensicEventType> event_queue_;

  // TODO(): Remove timer_map_.
  using FrameId = String;
  HashMap<FrameId, Deque<Persistent<CryptoRandomEvent>>> crypto_random_values_map_;
  HashMap<FrameId, Deque<Persistent<TimerEvent>>> timer_map_;
  HashMap<FrameId, HashMap<String, Deque<Persistent<DOMNodeEvent>>>> dom_node_verification_map_;
  HashMap<FrameId, Deque<Persistent<DOMNodeEvent>>> js_inserted_iframe_list_;
  // The set of DOM attribution modifications that need to be verified. (keyed
  // on frameId).
  HashMap<FrameId, Deque<Persistent<DOMAttrEvent>>> dom_attr_verification_map_;
  HashMap<FrameId, Persistent<GetterQueue>> getter_map_;
  HashMap<FrameId, Deque<Persistent<PerformanceEvent>>> performance_map_;
  v8RecordMap v8_random_number_map_;
  v8RecordMap v8_date_time_map_;


  // Storage API data.
  HashMap<FrameId, Deque<Persistent<ForensicStorageEvent>>> storage_key_map_;
  HashMap<FrameId, Deque<Persistent<ForensicStorageEvent>>>
      storage_get_item_map_;

  // Insert into Map
  template <typename T1, typename T2>
  void insertIntoMap(T1& map, T2* event) {
    String frame_id = event->FrameId();
    auto it = map.find(frame_id);
    if (it == map.end()) {
      Deque<Persistent<T2>> api_queue;
      api_queue.push_back(event);
      map.insert(frame_id, api_queue);
    } else
      it->value.push_back(event);
  }

  // Get next value in map.
  template <typename T1, typename T2>
  T1* getNextEventInMap(ReplayFrame* frame, T2& map) {
    String frame_id = frame->GetFrameId();
    auto it = map.find(frame_id);
    if (it == map.end() || it->value.empty())
      return nullptr;

    T1* event = it->value.front();
    it->value.pop_front();
    return event;
  }

  // Get next value in deque.
  template <typename T1, typename T2>
  T1* getNextEvent(T2& dequeue) {
    if (dequeue.empty())
      return nullptr;

    T1* event = dequeue.front();
    dequeue.pop_front();
    return event;
  }

  void inline mergeMaps(v8RecordMap&, const v8RecordMap&);

  friend base::DefaultSingletonTraits<ForensicDataStore>;

};

}  // namespace blink

#endif
