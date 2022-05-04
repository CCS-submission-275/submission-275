#include "third_party/blink/renderer/core/inspector/forensics/timer_recorder.h"
#include "base/bind.h"
#include "base/containers/span.h"
#include "base/hash/hash.h"
#include "third_party/blink/renderer/bindings/core/v8/scheduled_action.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_function.h"
#include "third_party/blink/renderer/core/dom/idle_deadline.h"
#include "third_party/blink/renderer/core/dom/scripted_idle_task_controller.h"
#include "third_party/blink/renderer/core/frame/dom_timer.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/dom/dom_high_res_time_stamp.h"

namespace blink {

void TimerRecorder::RecordTimerEventBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordIdleCallbackEventBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    int callback_id,
    int callback_type,
    double allotted_time) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetInteger("callbackId", callback_id);
  message->SetInteger("callbackType", callback_type);
  message->SetDouble("allottedTime", allotted_time);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordIdleCallbackEventEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    int callback_id,
    int callback_type) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetInteger("callbackId", callback_id);
  message->SetInteger("callbackType", callback_type);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);

}

void TimerRecorder::RecordRequestAnimationFrameEventBegin(
    ExecutionContext* context,
    int callback_id,
    double high_res_now_ms,
    double high_res_now_ms_legacy,
    bool is_legacy_call) {

  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordRequestAnimationFrameEventBeginInternal,
      GetThreadData(),
      copyFrameData(frame),
      callback_id,
      high_res_now_ms,
      high_res_now_ms_legacy,
      is_legacy_call
  ));
}

void TimerRecorder::RecordRequestAnimationFrameEventBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    int callback_id,
    double high_res_now_ms,
    double high_res_now_ms_legacy,
    bool is_legacy_call) {

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetInteger("callbackId", callback_id);
  message->SetDouble("highResNowMs", high_res_now_ms);
  message->SetDouble("highResNowMsLegacy", high_res_now_ms_legacy);
  message->SetBoolean("isLegacyCall", is_legacy_call);


  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordRequestAnimationFrameEventEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    int callback_id,
    double high_res_now_ms,
    double high_res_now_ms_legacy,
    bool is_legacy_call){

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetInteger("callbackId", callback_id);
  message->SetDouble("highResNowMs", high_res_now_ms);
  message->SetDouble("highResNowMsLegacy", high_res_now_ms_legacy);
  message->SetBoolean("isLegacyCall", is_legacy_call);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordRequestAnimationFrameEventEnd(
    ExecutionContext* context,
    int callback_id,
    double high_res_now_ms,
    double high_res_now_ms_legacy,
    bool is_legacy_call) {

  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordRequestAnimationFrameEventEndInternal,
      GetThreadData(),
      copyFrameData(frame),
      callback_id,
      high_res_now_ms,
      high_res_now_ms_legacy,
      is_legacy_call
  ));
}


void TimerRecorder::RecordIdleCallbackEventBegin(
    ExecutionContext* context,
    int callback_id,
    IdleDeadline::CallbackType callback_type,
    base::TimeDelta allotted_time) {
  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;
  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordIdleCallbackEventBeginInternal,
      GetThreadData(),
      copyFrameData(frame),
      callback_id,
      static_cast<int>(callback_type),
      allotted_time.InMicrosecondsF()
  ));
}

void TimerRecorder::RecordIdleCallbackEventEnd(
    ExecutionContext* context,
    int callback_id,
    IdleDeadline::CallbackType callback_type) {
  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordIdleCallbackEventEndInternal,
      GetThreadData(),
      copyFrameData(frame),
      callback_id,
      static_cast<int>(callback_type)
  ));
}

void TimerRecorder::RecordTimerEventBegin(LocalFrame* frame,
                                          ScheduledAction* action) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
              &TimerRecorder::RecordTimerEventBeginInternal,
              GetThreadData(),
              copyFrameData(frame)));
}
void TimerRecorder::RecordTimerEventEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordTimerEventEnd(LocalFrame* frame,
                                        ScheduledAction* action) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  GetTaskRunner()->PostTask(FROM_HERE,base::BindOnce(
          &TimerRecorder::RecordTimerEventEndInternal,
          GetThreadData(),
          copyFrameData(frame)));
}

void TimerRecorder::RecordTimerEventIDBeginInternal(
        std::unique_ptr<ThreadData> thread_data,
        std::unique_ptr<ForensicFrameData> frame_data,
        int timer_id) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  AddThreadData(thread_data, message);
  message->SetInteger("timerID", timer_id);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordTimerEventIDEndInternal(
        std::unique_ptr<ThreadData> thread_data,
        std::unique_ptr<ForensicFrameData> frame_data,
        int timer_id) {
  auto message = std::make_unique<JSONObject>();
  AddThreadData(thread_data, message);
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("timerID", timer_id);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void TimerRecorder::RecordTimerEventIDBegin(ExecutionContext* context,
                                            DOMTimer* timer) {
  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordTimerEventIDBeginInternal,
      GetThreadData(),
      copyFrameData(frame),
      timer->TimerId()
  ));
}

void TimerRecorder::RecordTimerEventIDEnd(ExecutionContext* context,
                                          DOMTimer* timer) {
  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(FROM_HERE, base::BindOnce(
      &TimerRecorder::RecordTimerEventIDEndInternal,
      GetThreadData(),
      copyFrameData(frame),
      timer->TimerId()
  ));
}





}  // namespace blink
