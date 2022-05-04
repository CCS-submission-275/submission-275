#include "third_party/blink/renderer/core/inspector/forensics/js_replayer.h"
#include "base/hash/hash.h"
#include "third_party/blink/renderer/bindings/core/v8/script_source_code.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"
#include "third_party/blink/renderer/core/script/classic_pending_script.h"
#include "third_party/blink/renderer/core/script/classic_script.h"
#include "third_party/blink/renderer/core/script/pending_script.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"

#include "third_party/blink/renderer/core/inspector/forensics/timer_recorder.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_replayer.h"
#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/animation_frame_event.h"

namespace blink {

JSReplayer::JSReplayer(ReplayFrame* frame)
    : replay_frame_(frame), sequence_number_(0) {
  event_replayer_ = MakeGarbageCollected<EventReplayer>(this);
}

// Script Execution
void JSReplayer::ExecuteScript(ScriptExecutionEvent* event) {

  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();

  int hash = 1;
  if (event->Hash() != 0)
    hash = event->Hash();

  auto it = classic_pending_script_map_.find(hash);
  if (it != classic_pending_script_map_.end()) {
    PendingScript* script = it->value;
    if (script->GetElement() == nullptr) {
      VLOG(7) << "Script  element is null (likely disposed/already executed)"
        << event->ToString();
      return;
    }

    //DCHECK(script->IsReady());
    PushExecutionEvent(String::Number(event->Hash()), "script");
    VLOG(7) << "Begin Execution of Script: " << event->ToString();
    script->ExecuteScriptBlock(NullURL());
    PopExecutionEvent();
  } else
    Frame()->DivergenceDetected("Failed to replay script: " + event->ToString());
}

// NOTE: Hooks third_party/blink/renderer/core/script/script_loader.cc#897.
// This addresses the scenario where a script is executed synchronously by
// Blink, even if another script is already executing. In this case, we
// cannot register the script and defer the execution to another time.
// Otherwise, it would create divergence.
void JSReplayer::ExecutePendingInSyncScript(String script_source,
                                            KURL script_url,
                                            PendingScript* script) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();
  int hash = CalculateHash(script_source);

  VLOG(7) << "Executing PendingInSyncScript: " << hash << " " << script_url;

  script->SetReplayTrackingType(PendingScript::ReplayTrackingType::kScheduled);
  if (IsJSExecuting()) {

    VLOG(7) << "Executing Script InSync: " << hash << " "  << script_url;
    // This is a nested script execution, so we execute now.
    // Add this script to the executing in advance queue.
    executed_in_advance_.push_back(hash);
    // Execute the script.
    DCHECK(script->IsReady());
    PushExecutionEvent(String::Number(hash), "script");
    script->ExecuteScriptBlock(script_url);
    PopExecutionEvent();
    sequence_number_++;
  } else {
    VLOG(7) << "JS Not Executing Deferring Script Execution until later."
            << hash << " " << script_url;
    // Otherwise, just add it to the queue and execute later.
    classic_pending_script_map_.insert(hash, script);
  }
}

void JSReplayer::RegisterClassicPendingScript(ClassicPendingScript* script) {
  DCHECK(script->IsReady());
  ClassicScript* classic_script =
      script->GetSource(replay_frame_->GetDocument()->Url());

  if (!classic_script) {
    VLOG(7) << "Source for script was not found.";
    // script->SetReplayTrackingType(PendingScript::ReplayTrackingType::kScheduled);
    return;
  }

  DCHECK(classic_script);
  ScriptSourceCode source = classic_script->GetScriptSourceCode();
  int hash = CalculateHash(source.Source().ToString());
  VLOG(7) << "Registering Script: " << hash << " "
    << script->GetElement()->SourceAttributeValue();

  script->SetReplayTrackingType(PendingScript::ReplayTrackingType::kScheduled);

  if (hash == 0)
    hash = 1;

  classic_pending_script_map_.insert(hash, script);
}

void JSReplayer::ReplayScriptExecutionEvent(ScriptExecutionEvent* event) {
  VLOG(7) << "Attempting to replay: " << event->ToString() << std::endl;
  // We do not need to replay scripts executed in advance.

  if (!executed_in_advance_.IsEmpty()) {
    int last_executed_script_hash = executed_in_advance_.front();
    // If a script was executed in advance, then this event must be
    // the event in the queue for this.
    if (last_executed_script_hash != event->Hash()) {
      Frame()->DivergenceDetected(
          "last_hash: " + String::Number(last_executed_script_hash) +
          " " + event->ToString());
      return;
    }
    VLOG(7) << "Script was executed in advance: " << event->ToString();
    executed_in_advance_.pop_front();
    return;
  }

  auto* engine = ForensicReplayEngine::GetInstance();
  // Determine if script has already been registered.
  VLOG(7) << "Searching hash: " << event->Hash() << std::endl;

  int hash = 1;
  if (event->Hash() != 0)
    hash = event->Hash();
  auto it = classic_pending_script_map_.find(hash);

  if (it != classic_pending_script_map_.end()) {
    VLOG(7) << "Executing Script: " << event->ToString();
    engine->unpauseDispatcher();
    ExecuteScript(event);
    classic_pending_script_map_.erase(it);
  } else {
    // If Script has not been registered yet, the dispatcher is paused,
    // and this function is rescheduled.

    // In non-strict mode, we stop trying after 10 attempts/10 seconds.
    if (!Frame()->isStrictReplay() && wait_for_script_registration_count_ > 3) {
      Frame()->DivergenceDetected(
          "Script was never registered: " + event->ToString());
      engine->unpauseDispatcher();
      wait_for_script_registration_count_ = 0;
      return;
    } else
      wait_for_script_registration_count_++;

    // Schedule this function to be ran again. Hopefully, the script has
    // been registered.
    engine->pauseDispatcher();
    engine->MainFrame()->GetFrame()->GetTaskRunner(TaskType::kDOMManipulation)
    ->PostDelayedTask(
            FROM_HERE,
            WTF::Bind(&JSReplayer::ReplayScriptExecutionEvent,
                      WrapPersistent(this), WrapPersistent(event)),
            base::TimeDelta::FromSeconds(1));
  }
}

// Timer Execution.
void JSReplayer::ReplayTimerEvent(TimerEvent* event) {
  auto it = timer_map_.find(event->TimerId());
  if (it == timer_map_.end()) {
    Frame()->DivergenceDetected("Timer event was not registered"
                       + String::Number(event->TimerId()));
    return;
  }

  if (sequence_number_ == 0) {
    ExecuteTimerEvent(event);
    return;
  }

  base::TimeDelta delay = CalculateEventDelay(event->TimeDelta());
  if (delay <= base::TimeDelta()) {
    VLOG(7) << "Replay is lagging: Timer fired late: " << event->ToString();
    ExecuteTimerEvent(event);
    return;
  }

  // Fire Timer event async
  VLOG(7) << "Replaying TimerEvent Async: " << event->ToString()
          << "delay: " << delay << " seq num: " << sequence_number_;
  auto* engine = ForensicReplayEngine::GetInstance();
  engine->pauseDispatcher();
  auto task = WTF::Bind(&JSReplayer::ReplayTimerEventAsync,
                        WrapPersistent(this), WrapPersistent(event));
  auto task_runner = engine->MainFrame()->GetFrame()->GetTaskRunner(
      TaskType::kDOMManipulation);
  task_runner->PostDelayedTask(FROM_HERE, std::move(task), delay);
}

void JSReplayer::RegisterTimerEvent(ExecutionContext* context,
                                    DOMTimer* timer,
                                    bool is_one_shot) {
  VLOG(7) << "Registering timer event: " << timer->TimerId();
  auto it = timer_map_.find(timer->TimerId());
  DCHECK(it == timer_map_.end())
      << "Duplicate timer registration" << timer->TimerId();
  ReplayDOMTimer* replay_dom_timer = MakeGarbageCollected<ReplayDOMTimer>();
  replay_dom_timer->bindDOMTimer(context, timer);
  timer_map_.insert(timer->TimerId(), replay_dom_timer);
}

void JSReplayer::ReplayTimerEventAsync(TimerEvent* event) {
  ExecuteTimerEvent(event);
  ForensicReplayEngine::GetInstance()->unpauseDispatcher();
}

void JSReplayer::ExecuteTimerEvent(TimerEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();
  VLOG(7) << "Executing timer event: " << event->ToString();
  auto it = timer_map_.find(event->TimerId());
  if (it != timer_map_.end()) {
    PushExecutionEvent(String::Number(event->TimerId()), "timer");
    prev_event_fired_time_ = it->value->Execute();
    PopExecutionEvent();
    sequence_number_++;
  } else
    Frame()->DivergenceDetected(
        "Timer Event Failed to replay: " + event->ToString());
}

// Replay JS Events
void JSReplayer::ReplayJSEvent(JSEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();
  // If this is the first event for this frame, then just fire it synchronously.
  if (sequence_number_ == 0) {
    FireJSEvent(event);
    return;
  }

  base::TimeDelta delay = CalculateEventDelay(event->TimeDelta());
  // Replay time has lagged behind recording time, fire synchronously.
  if (delay <= base::TimeDelta()) {
    VLOG(7) << "Replay is lagging, firing late event: " << event->ToString();
    FireJSEvent(event);
    return;
  }

  // Fire JS event async
  auto* engine = ForensicReplayEngine::GetInstance();
  engine->pauseDispatcher();
  auto task = WTF::Bind(&JSReplayer::ReplayJSEventAsync, WrapPersistent(this),
                        WrapPersistent(event));
  auto task_runner = engine->MainFrame()->GetFrame()->GetTaskRunner(
      TaskType::kDOMManipulation);
  task_runner->PostDelayedTask(FROM_HERE, std::move(task), delay);
}

void JSReplayer::FireJSEvent(JSEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();

  VLOG(7) << "Firing JS event: " << event->ToString();
  sequence_number_++;
  PushExecutionEvent(event->EventType(), "event");
  prev_event_fired_time_ = event_replayer_->FireEvent(replay_frame_, event);
  PopExecutionEvent();
}

void JSReplayer::ReplayJSEventAsync(JSEvent* event) {
  FireJSEvent(event);
  ForensicReplayEngine::GetInstance()->unpauseDispatcher();
}

// IdleCallback Execution
void JSReplayer::RegisterIdleCallbackEvent(int callback_id) {
  VLOG(7) << "Registered IdleCallback: " << callback_id;
  idle_callbacks_registered_.insert(callback_id);
}


void JSReplayer::ReplayAnimationFrameEventAsync(AnimationFrameEvent* event) {
  VLOG(7) << "Replay animation frame event async";
  ExecuteAnimationFrameEvent(event);
  ForensicReplayEngine::GetInstance()->unpauseDispatcher();
}

void JSReplayer::ReplayAnimationFrameEvent(AnimationFrameEvent* event) {
  replay_frame_->UpdateV8SecurityToken();

  VLOG(7) << "Replay animation frame event.";

  // Verify callback exists and was registered, then remove it.
  int callback_id = event->CallbackId();
  auto it = animation_frame_callback_map_.find(callback_id);

  if (it == animation_frame_callback_map_.end()) {
    Frame()->DivergenceDetected("AnimationFrameCallback was not registered."
                                + String::Number(callback_id));
    return;
  }

  // If this is the first item in the sequence, we execute now.
  if (sequence_number_ == 0) {
    ExecuteAnimationFrameEvent(event);
    return;
  }

  base::TimeDelta delay = CalculateEventDelay(event->TimeDelta());
  if (delay <= base::TimeDelta()) {
    VLOG(7) << "Replay is lagging: Animation fired late: " << event->ToString();
    ExecuteAnimationFrameEvent(event);
    return;
  }

  auto* engine = ForensicReplayEngine::GetInstance();
  engine->pauseDispatcher();
  auto task = WTF::Bind(&JSReplayer::ReplayAnimationFrameEventAsync,
                        WrapPersistent(this), WrapPersistent(event));
  auto task_runner = engine->MainFrame()->GetFrame()->GetTaskRunner(
      TaskType::kDOMManipulation);
  task_runner->PostDelayedTask(FROM_HERE, std::move(task), delay);
}

void JSReplayer::ExecuteAnimationFrameEvent(AnimationFrameEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();
  int callback_id = event->CallbackId();
  auto it = animation_frame_callback_map_.find(callback_id);
  if (it != animation_frame_callback_map_.end()) {

    TimerRecorder::RecordRequestAnimationFrameEventBegin(
        replay_frame_->GetFrame()->GetDocument()->GetExecutionContext(),
        callback_id,
        event->HighResNowMs(),
        event->HighResNowMsLegacy(),
        event->isLegacyCall());

    PushExecutionEvent(String::Number(callback_id), "animation");
    if (event->isLegacyCall())
      it->value->Invoke(event->HighResNowMsLegacy());
    else
      it->value->Invoke(event->HighResNowMs());
    PopExecutionEvent();
    prev_event_fired_time_ = base::TimeTicks::Now();

    TimerRecorder::RecordRequestAnimationFrameEventEnd(
        replay_frame_->GetFrame()->GetDocument()->GetExecutionContext(),
        callback_id,
        event->HighResNowMs(),
        event->HighResNowMsLegacy(),
        event->isLegacyCall());

  } else {
    VLOG(7) << "Animation Callback ID: " << callback_id << "Not Found!";
    Frame()->DivergenceDetected(
        "Animation Frame Event Failed to replay: " + event->ToString());
  }
}

void JSReplayer::RegisterAnimationFrameEvent(
  FrameRequestCallbackCollection::FrameCallback* callback) {
  int callback_id = callback->Id();
  VLOG(7) << "Registering animation frame event: " << callback_id;
  auto it = animation_frame_callback_map_.find(callback_id);
  DCHECK(it == animation_frame_callback_map_.end())
      << "Duplicate animation frame registration" << callback_id;
  animation_frame_callback_map_.insert(callback_id, callback);
}

void JSReplayer::ReplayIdleCallbackEvent(IdleCallbackEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();
  // Verify callback exists and was registered, then remove it.
  int callback_id = event->CallbackId();
  auto it = idle_callbacks_registered_.find(callback_id);

  if (it == idle_callbacks_registered_.end()) {
    Frame()->DivergenceDetected("Idlecallback was not registered."
                                + String::Number(callback_id));
    return;
  } else
    idle_callbacks_registered_.erase(it);

  if (sequence_number_ == 0) {
    ExecuteIdleCallback(event);
    return;
  }

  base::TimeDelta delay = CalculateEventDelay(event->TimeDelta());
  if (delay <= base::TimeDelta()) {
    VLOG(7) << "Replay is lagging: Timer fired late: " << event->ToString();
    ExecuteIdleCallback(event);
    return;
  }

  auto* engine = ForensicReplayEngine::GetInstance();
  engine->pauseDispatcher();
  auto task = WTF::Bind(&JSReplayer::ReplayIdleCallbackEventAsync,
                        WrapPersistent(this), WrapPersistent(event));
  auto task_runner = engine->MainFrame()->GetFrame()->GetTaskRunner(
      TaskType::kDOMManipulation);
  task_runner->PostDelayedTask(FROM_HERE, std::move(task), delay);
}

void JSReplayer::ReplayIdleCallbackEventAsync(IdleCallbackEvent* event) {
  ExecuteIdleCallback(event);
  ForensicReplayEngine::GetInstance()->unpauseDispatcher();
}

void JSReplayer::ExecuteIdleCallback(IdleCallbackEvent* event) {
  // important for replaying v8 records
  replay_frame_->UpdateV8SecurityToken();

  int callback_id = event->CallbackId();
  // Create deadline where allotted time is the same as during recording.
  base::TimeDelta allotted_time =
      base::TimeDelta::FromMicrosecondsD(event->AllottedTime());
  base::TimeTicks deadline = allotted_time + base::TimeTicks::Now();

  // Execute idle callback.
  int callback_type = event->CallbackType();
  auto* document = replay_frame_->GetDocument();

  VLOG(7) << "Replaying Idle Callback: " << event->ToString();
  PushExecutionEvent(String::Number(callback_id), "idle");
  document->EnsureScriptedIdleTaskController().CallbackFired(
      callback_id, deadline,
      static_cast<IdleDeadline::CallbackType>(callback_type));
  PopExecutionEvent();
  prev_event_fired_time_ = base::TimeTicks::Now();
}

base::TimeDelta JSReplayer::CalculateEventDelay(base::TimeDelta event_delay) {
  // This difference between time in between this event and previous during
  // recording.
  return event_delay - (base::TimeTicks::Now() - prev_event_fired_time_);
}

// Maintain Execution Stack
void JSReplayer::PushExecutionEvent(String id, String type) {
  JSReplayerExecutionEvent execution_event(id, type);
  execution_stack_.push_front(execution_event);
}

void JSReplayer::PopExecutionEvent() {
  // NOTE(): Why do we not enforce correct id at this point?
  DCHECK(!execution_stack_.IsEmpty());
  execution_stack_.pop_front();
}


// Utility functions
int JSReplayer::CalculateHash(String source) {
  const void* bytes = source.Bytes();
  size_t size = source.length();

  int hash = 1;
  if (size != 0)
    hash = base::PersistentHash(bytes, size);

  VLOG(7) << "Registering script for execution:" << hash
          << " size: " << size;
  return hash;
}

void JSReplayer::Trace(Visitor* visitor) {
  visitor->Trace(classic_pending_script_map_);
  visitor->Trace(replay_frame_);
  visitor->Trace(event_replayer_);
  visitor->Trace(timer_map_);
  visitor->Trace(animation_frame_callback_map_);
}

}  // namespace blink
