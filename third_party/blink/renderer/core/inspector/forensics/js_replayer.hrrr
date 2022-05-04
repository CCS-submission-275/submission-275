#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_REPLAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_REPLAYER_H_

#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/deque.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_dom_timer.h"
#include "third_party/blink/renderer/core/dom/frame_request_callback_collection.h"

namespace blink {

class ScriptExecutionEvent;
class AnimationFrameEvent;

class ClassicPendingScript;
class PendingScript;
class ReplayFrame;
class EventReplayer;
class JSEvent;
class KURL;
class TimerEvent;
class IdleCallbackEvent;

struct JSReplayerExecutionEvent {
  String id_;
  String type_;
  JSReplayerExecutionEvent(String id, String type) {id_ = id; type_ = type;}
  String ToString() {return type_ + "-" + id_;}
};

/*
 * The JSReplayer is responsible for scheduling executing events during the
 * replay. The Execution Events we currently support.
 *  1. Scripts
 *  2. Events
 *  3. Callbacks registered with setInterval & setTimeout.
 *  4. Callbacks registered with window.requestIdleCallback()
 *  5. Callbacks registered with window.requestAnimationFrame()
 *
 * The JSReplayer maintains an execution stack. When an execution event occurs,
 * it will push this event onto the stack, and pop it when it is complete.
 *
 */
class JSReplayer final : public GarbageCollected<JSReplayer> {

public:
  // Register & Replaying Script Execution Events.
  explicit JSReplayer(ReplayFrame* frame);
  void ReplayScriptExecutionEvent(ScriptExecutionEvent *event);
  void RegisterClassicPendingScript(ClassicPendingScript *script);
  void ExecutePendingInSyncScript(String script_source,
                                  KURL script_url,
                                  PendingScript* script);
  // Register & Replay Timer Events.
  void ReplayTimerEvent(TimerEvent* event);
  void RegisterTimerEvent(ExecutionContext* context,
                          DOMTimer* timer,
                          bool is_one_shot);
  // Register & Replay Idle Callback Events.
  void ReplayIdleCallbackEvent(IdleCallbackEvent* event);
  void RegisterIdleCallbackEvent(int callback_id);
  // Register & Replay Animation Frame Evetns.
  void ReplayAnimationFrameEvent(AnimationFrameEvent* event);
  void RegisterAnimationFrameEvent(
      FrameRequestCallbackCollection::FrameCallback* callback);
  // Replay JS Execution
  void ReplayJSEvent(JSEvent* event);
  // Utility functions
  bool IsJSExecuting() { return !execution_stack_.empty(); }
  String CurrentExecutingEvent(){ return
    IsJSExecuting() ? execution_stack_.front().ToString() : "NoJSExecution";}
  ReplayFrame* Frame() {return replay_frame_;}
  void Trace(Visitor *visitor);


private:
  Deque<JSReplayerExecutionEvent> execution_stack_;
  Member<ReplayFrame> replay_frame_;
  Member<EventReplayer> event_replayer_;
  // Helper functions for replaying Script Execution.
  void ExecuteScript(ScriptExecutionEvent *event);
  // Helper functions for replaying timer events.
  void ReplayTimerEventAsync(TimerEvent* event);
  void ExecuteTimerEvent(TimerEvent* event);
  // Helper functions for replaying Idle Callback Events.
  void ReplayIdleCallbackEventAsync(IdleCallbackEvent* event);
  void ExecuteIdleCallback(IdleCallbackEvent* event);
  // Helper functions for replaying AnimationFrame Events.
  void ReplayAnimationFrameEventAsync(AnimationFrameEvent* event);
  void ExecuteAnimationFrameEvent(AnimationFrameEvent* event);
  // Helper functions for replaying JS events
  void ReplayJSEventAsync(JSEvent* event);
  void FireJSEvent(JSEvent* event);
  // Maintain execution stack.
  void PushExecutionEvent(String id, String type);
  void PopExecutionEvent();

  // If a script had to be executed prior to us receiving the ScriptExecutionEvent
  // the script's hash value will be stored here.
  Deque<int> executed_in_advance_;
  // The number of times that ReplayScriptExecutionEvent has been called and
  // the script has not been registered.
  int wait_for_script_registration_count_ = 0;

  // Utility functions.
  int CalculateHash(String script_source);
  base::TimeDelta CalculateEventDelay(base::TimeDelta event_delay);

  // Registration Maps.
  HeapHashMap<int, Member<ReplayDOMTimer>> timer_map_;
  HashSet<int> idle_callbacks_registered_;
  HeapHashMap<int, Member<PendingScript>> classic_pending_script_map_;
  HeapHashMap<int,
    Member<FrameRequestCallbackCollection::FrameCallback>> animation_frame_callback_map_;


  // FIXME: Remove this when possible.
  // The number of events that have been fired so far.
  int sequence_number_;
  // The timestamp at the end of execution of the prev replay event.
  base::TimeTicks prev_event_fired_time_;
};

} // blink
#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_SCRIPT_REPLAYER_H_
