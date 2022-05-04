#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_RECORDER_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_RECORDER_H

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"

namespace blink {

class CORE_EXPORT TimerRecorder : ForensicRecorder {
  DISALLOW_COPY(TimerRecorder);

 public:
  static void RecordTimerEventBegin(LocalFrame* frame, ScheduledAction* action);
  static void RecordTimerEventEnd(LocalFrame* frame, ScheduledAction* action);
  static void RecordTimerEventIDBegin(ExecutionContext* context,
                                      DOMTimer* timer);
  static void RecordTimerEventIDEnd(ExecutionContext* context, DOMTimer* timer);
  static void RecordIdleCallbackEventBegin(
      ExecutionContext* context,
      int callback_id,
      IdleDeadline::CallbackType callback_type,
      base::TimeDelta alotted_time);
  static void RecordIdleCallbackEventEnd(
      ExecutionContext* context,
      int callback_id,
      IdleDeadline::CallbackType callback_type);

  static void RecordRequestAnimationFrameEventBegin(
      ExecutionContext* context,
      int callback_id,
      double high_res_now_ms,
      double high_res_now_ms_legacy,
      bool is_legacy_call);

  static void RecordRequestAnimationFrameEventEnd(
      ExecutionContext* context,
      int callback_id,
      double high_res_now_ms,
      double high_res_now_ms_legacy,
      bool is_legacy_call);

  static void RecordPerformanceNowEvent(
      ExecutionContext* context,
      const DOMHighResTimeStamp& now);


private:
  static void RecordTimerEventIDBeginInternal(
          std::unique_ptr<ThreadData> thread_data,
          std::unique_ptr<ForensicFrameData> frame_data,
          int timer_id);
  static void RecordTimerEventIDEndInternal(
          std::unique_ptr<ThreadData> thread_data,
          std::unique_ptr<ForensicFrameData> frame_data,
          int timer_id);
  static void RecordIdleCallbackEventBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      int callback_id,
      int callback_type,
      double allotted_time);
  static void RecordIdleCallbackEventEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      int callback_id,
      int callback_type);
  static void RecordTimerEventBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data);
  static void RecordTimerEventEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data);
  static void RecordRequestAnimationFrameEventBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      int callback_id,
      double high_res_now_ms,
      double high_res_now_ms_legacy,
      bool is_legacy_call);
  static void RecordRequestAnimationFrameEventEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      int callback_id,
      double high_res_now_ms,
      double high_res_now_ms_legacy,
      bool is_legacy_call);
  static void RecordPerformanceNowEventInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    double now);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_TIMER_RECORDER_H
