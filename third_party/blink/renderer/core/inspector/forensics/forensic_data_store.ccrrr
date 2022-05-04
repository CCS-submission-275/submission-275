#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"
#include "base/logging.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"

namespace blink {

ForensicDataStore::ForensicDataStore() {}

ForensicDataStore* ForensicDataStore::GetInstance() {
  return base::Singleton<ForensicDataStore>::get();
}

void ForensicDataStore::insertStorageEvent(ForensicStorageEvent* event) {
  if (event->ApiName() == "key")
    insertIntoMap<>(storage_key_map_, event);
  else if (event->ApiName() == "getItem")
    insertIntoMap<>(storage_get_item_map_, event);
  else
    NOTREACHED();
}

ForensicStorageEvent* ForensicDataStore::NextStorageEvent(ReplayFrame* frame,
                                                          String api) {
  if (api == "key")
    return getNextEventInMap<ForensicStorageEvent>(frame, storage_key_map_);
  else  // api == "getItem"
    return getNextEventInMap<ForensicStorageEvent>(frame,
                                                   storage_get_item_map_);
}

int ForensicDataStore::PeekTimerEventSequenceNumber(ReplayFrame* frame) {
  String frame_id = frame->GetFrameId();
  auto iter = timer_map_.find(frame_id);
  if (iter == timer_map_.end() || iter->value.empty())
    return -1;
  else
    return iter->value.front()->SequenceNumber();
}


void ForensicDataStore::insertLoadResourceIdentifier(String identifier) {
  load_resource_identifier_list_.push_back(identifier);
  event_queue_.push_back(ForensicEventType::kLoadResource);
}

void ForensicDataStore::insertCryptoRandomEvent(CryptoRandomEvent* event) {
  insertIntoMap<>(crypto_random_values_map_, event);
}

void ForensicDataStore::insertScriptExecutionEvent(
    ScriptExecutionEvent* event) {
  script_execution_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kScriptExecutionEvent);
}

void ForensicDataStore::insertDOMNodeEvent(DOMNodeEvent* event) {
  dom_node_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kDOMNode);
}

void ForensicDataStore::insertNavigationEvent(NavigationEvent* event) {
  navigation_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kLoadPage);
}

void ForensicDataStore::insertJSEvent(JSEvent* event) {
  js_event_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kJSEvent);
}

void ForensicDataStore::insertIdleCallbackEvent(IdleCallbackEvent* event) {
  idle_callback_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kIdleCallbackEvent);
}

void ForensicDataStore::insertAnimationFrameEvent(AnimationFrameEvent* event) {
  animation_frame_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kAnimationFrameEvent);
}

void ForensicDataStore::insertTimerEvent(TimerEvent* event) {
  timer_list_.push_back(event);
  event_queue_.push_back(ForensicEventType::kTimerEvent);
}

void ForensicDataStore::insertDOMAttrEvent(DOMAttrEvent* event) {
  insertIntoMap<>(dom_attr_verification_map_, event);
}

void ForensicDataStore::insertJSInsertedIframeEvent(DOMNodeEvent* event) {
  insertIntoMap<>(js_inserted_iframe_list_, event);
}

GetterEvent* ForensicDataStore::NextGetterEvent(ReplayFrame* frame,
                                                String interface,
                                                String attribute) {
    String frame_id = frame->GetFrameId();
    auto it = getter_map_.find(frame_id);
    if (it == getter_map_.end())
      return nullptr;

    return it->value->NextGetterEvent(interface, attribute);
}

void GetterQueue::insert(GetterEvent* event) {
  String name = event->Interface() + "." + event->Attribute();
  auto it = getter_map_.find(name);
  if (it == getter_map_.end()) {
    HeapDeque<Member<GetterEvent>> queue;
    queue.push_back(event);
    getter_map_.insert(name, queue);
  } else
    it->value.push_back(event);
}

GetterEvent* GetterQueue::NextGetterEvent(String interface, String attribute) {
  String name = interface + "." + attribute;
  auto it = getter_map_.find(name);
  if (it == getter_map_.end() || it->value.empty())
    return nullptr;

  GetterEvent* event = it->value.front();
  it->value.pop_front();
  return event;
}

void GetterQueue::Trace(Visitor* visitor) {
  visitor->Trace(getter_map_);
}

void ForensicDataStore::insertGetterEvent(GetterEvent* event) {
  String frame_id = event->FrameId();
  auto it = getter_map_.find(frame_id);
  if (it == getter_map_.end()) {
    GetterQueue* getter_queue = MakeGarbageCollected<GetterQueue>();
    getter_queue->insert(event);
    getter_map_.insert(frame_id, getter_queue);
  } else
    it->value->insert(event);
}

void ForensicDataStore::insertPerformanceEvent(PerformanceEvent* event) {
  insertIntoMap<>(performance_map_, event);
}

ScriptExecutionEvent* ForensicDataStore::NextScriptExecutionEvent() {
  if (script_execution_list_.empty())
    return nullptr;

  ScriptExecutionEvent* event = script_execution_list_.front();
  script_execution_list_.pop_front();
  return event;
}

JSEvent* ForensicDataStore::NextJSEvent() {
  return getNextEvent<JSEvent>(js_event_list_);
}

TimerEvent* ForensicDataStore::NextTimerEvent() {
  return getNextEvent<TimerEvent>(timer_list_);
}

NavigationEvent* ForensicDataStore::NextNavigationEvent() {
  return getNextEvent<NavigationEvent>(navigation_list_);
}

IdleCallbackEvent* ForensicDataStore::NextIdleCallbackEvent() {
  return getNextEvent<IdleCallbackEvent>(idle_callback_list_);
}

AnimationFrameEvent* ForensicDataStore::NextAnimationFrameEvent() {
  return getNextEvent<AnimationFrameEvent>(animation_frame_list_);
}

DOMNodeEvent* ForensicDataStore::NextDOMNodeEvent() {
  return getNextEvent<DOMNodeEvent>(dom_node_list_);
}

DOMNodeEvent* ForensicDataStore::NextJSInsertedIframeEvent(ReplayFrame* frame) {
  return getNextEventInMap<DOMNodeEvent>(frame, js_inserted_iframe_list_);
}

void ForensicDataStore::insertVerificationEvent(DOMNodeEvent* event) {
  String frame_id = event->FrameId();
  String executing_event = event->ExecutingEvent();
  VLOG(7) << __func__ << " for frame: " << frame_id
          << " event: " << executing_event << " , " << event->ToString();
  auto it = dom_node_verification_map_.find(frame_id);
  if (it == dom_node_verification_map_.end()) {
    HashMap<String, Deque<Persistent<DOMNodeEvent>>> api_map;
    Deque<Persistent<DOMNodeEvent>> api_queue;
    api_queue.push_back(event);
    api_map.insert(executing_event, api_queue);
    dom_node_verification_map_.insert(frame_id, api_map);
  } else {
    auto api_queue_it = it->value.find(executing_event);
    if (api_queue_it == it->value.end()) {
      Deque<Persistent<DOMNodeEvent>> api_queue;
      api_queue.push_back(event);
      it->value.insert(executing_event, api_queue);
    } else {
      api_queue_it->value.push_back(event);
    }
  }
}

DOMNodeEvent* ForensicDataStore::NextNodeVerificationEvent(ReplayFrame* frame) {
  String frame_id = frame->GetFrameId();
  auto it = dom_node_verification_map_.find(frame_id);
  if (it == dom_node_verification_map_.end() || it->value.IsEmpty())
    return nullptr;
  auto* js_replayer = frame->GetJSReplayer();
  auto api_map = it->value.find(js_replayer->CurrentExecutingEvent());
  if (api_map == it->value.end() || api_map->value.IsEmpty())
    return nullptr;
  VLOG(7) << __func__ << " found node event for " << frame_id << " during "
          << js_replayer->CurrentExecutingEvent();
  return api_map->value.TakeFirst();
}


PerformanceEvent* ForensicDataStore::NextPerformanceEvent(ReplayFrame* frame) {
  return getNextEventInMap<PerformanceEvent>(frame, performance_map_);
}

DOMAttrEvent* ForensicDataStore::NextAttrVerificationEvent(ReplayFrame* frame) {
  return getNextEventInMap<DOMAttrEvent>(frame, dom_attr_verification_map_);
}

String ForensicDataStore::NextLoadResourceIdentifier() {
  if (load_resource_identifier_list_.empty())
    return "";
  return load_resource_identifier_list_.TakeFirst();
}

String ForensicDataStore::NextCryptoRandomValues(ReplayFrame* frame) {
  auto* event =
      getNextEventInMap<CryptoRandomEvent>(frame, crypto_random_values_map_);
  if (event)
    return event->Values();
  return "";
}

ForensicEventType ForensicDataStore::NextForensicEventType() {
  if (event_queue_.empty())
    return ForensicEventType::kEmptyQueue;
  ForensicEventType eventType = event_queue_.front();
  event_queue_.pop_front();
  return eventType;
}

void ForensicDataStore::mergeMaps(v8RecordMap& dst, const v8RecordMap& src) {
  for (auto it : src) {
    if (dst.find(it.first) != dst.end()) {  // merge
      dst.at(it.first).insert(dst.at(it.first).end(), it.second.begin(),
                              it.second.end());
    } else {  // insert
      dst.insert({it.first, {it.second.begin(), it.second.end()}});
    }
  }
}

void ForensicDataStore::insertV8Records(
    const std::unordered_map<std::string, std::vector<double>>& rnd,
    const std::unordered_map<std::string, std::vector<double>>& ts) {
  mergeMaps(v8_date_time_map_, ts);
  mergeMaps(v8_random_number_map_, rnd);
}

v8::PlatformInstrumentationData ForensicDataStore::getV8InstrumentData() {
  v8::PlatformInstrumentationData data;
  data.current_timestamps_ = v8_date_time_map_;
  data.math_random_values_ = v8_random_number_map_;
  return data;
}
}  // namespace blink
