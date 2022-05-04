#include "third_party/blink/renderer/core/inspector/inspector_forensics_agent.h"
#include <string>
#include <unordered_map>
#include <vector>
#include "base/logging.h"
#include "third_party/blink/renderer/core/inspector/forensics/animation_frame_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/crypto_random_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "third_party/blink/renderer/core/inspector/forensics/getter_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/idle_callback_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/navigation_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/performance_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/script_execution_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/v8_recorder.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forward.h"
#include "third_party/blink/renderer/core/loader/resource/css_style_sheet_resource.h"
#include "third_party/blink/renderer/platform/forensics/load_resource_event.h"
#include "third_party/blink/renderer/platform/forensics/loader_recorder.h"
#include "third_party/blink/renderer/platform/forensics/resource_data_store.h"


namespace blink {

using protocol::Response;

InspectorForensicsAgent::InspectorForensicsAgent(
    InspectedFrames* inspected_frames)
    : enabled_(&agent_state_, /*default_value = */ false),
      recording_(false),
      replaying_(false),
      inspected_frames_(inspected_frames) {
  replay_engine_ = blink::ForensicReplayEngine::GetInstance();
  replay_engine_->Initialize(inspected_frames_);
}

void InspectorForensicsAgent::Trace(Visitor* visitor) {
  visitor->Trace(inspected_frames_);
  InspectorBaseAgent::Trace(visitor);
}

Response InspectorForensicsAgent::enable() {
  if (!enabled_.Get())
    enabled_.Set(true);
  instrumenting_agents_->AddInspectorForensicsAgent(this);
  return Response::Success();
}

Response InspectorForensicsAgent::startReplay() {
  // if (recording_) {
  //   return Response::ServerError("It's recording now. Stop recording first!");
  // }
  replaying_ = true;
  ResourceDataStore::GetInstance()->setReplay(true);
  // so we read from data store
  replay_engine_->setReplay(replaying_);
  replay_engine_->StartReplay();
  return Response::Success();
}

Response InspectorForensicsAgent::stopReplay() {
  if (recording_) {
    return Response::ServerError("It's recording now. Stop recording first!");
  }
  replaying_ = false;
  ResourceDataStore::GetInstance()->setReplay(
      false);  // so we load from network
  replay_engine_->setReplay(replaying_);
  v8::V8::StopPlatformInst();
  return Response::Success();
}

Response InspectorForensicsAgent::startRecord() {
  if (replaying_) {
    return Response::ServerError(
        "It's replaying now. Stop replaying first, unless you want to "
        "forceRecord!");
  }
  ForensicRecorder::setRecord(true);  // enable logging DOM events
  LoaderRecorder::setRecord(
      true);  // enable logging Network events on platform layer
  v8::V8::StartPlatformInstRecording();
  return Response::Success();
}

Response InspectorForensicsAgent::forceRecord() {
  ForensicRecorder::setRecord(true);
  LoaderRecorder::setRecord(true);
  v8::V8::StartPlatformInstRecording();
  return Response::Success();
}

Response InspectorForensicsAgent::stopRecord() {
  if (replaying_) {
    return Response::ServerError("It's replaying now. Stop replaying first!");
  }
  recording_ = false;
  ForensicRecorder::setRecord(false);
  LoaderRecorder::setRecord(false);
  // dump logs here
  v8::V8::StopPlatformInst();
  v8::PlatformInstrumentationData v8_records =
      v8::V8::DumpPlatformInstRecording();
  V8Recorder::DumpV8Records(v8_records, __func__);
  return Response::Success();
}

Response InspectorForensicsAgent::insertDOMAttrEvent(
    std::unique_ptr<protocol::Forensics::DOMAttrEvent> event) {
  DOMAttrEvent* dom_attr_event =
      MakeGarbageCollected<DOMAttrEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertDOMAttrEvent(dom_attr_event);
  return Response::Success();
}

Response InspectorForensicsAgent::insertNavigationEvent(
    std::unique_ptr<protocol::Forensics::NavigationEvent> event) {
  NavigationEvent* navigation_event =
      MakeGarbageCollected<NavigationEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertNavigationEvent(navigation_event);
  return Response::Success();
}

Response InspectorForensicsAgent::insertPerformanceEvent(
    std::unique_ptr<protocol::Forensics::PerformanceEvent> event) {
  PerformanceEvent* performance_event =
      MakeGarbageCollected<PerformanceEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertPerformanceEvent(performance_event);
  return Response::Success();
}

Response InspectorForensicsAgent::insertLoadResourceEvent(
    std::unique_ptr<protocol::Forensics::LoadResourceEvent> event) {
  LoadResourceEvent* load_resource_event =
      new LoadResourceEvent(std::move(event));
  ResourceDataStore::GetInstance()->insertLoadResourceEvent(
      load_resource_event);
  ForensicDataStore::GetInstance()->insertLoadResourceIdentifier(
      load_resource_event->Identifier());
  return Response::Success();
}

Response InspectorForensicsAgent::insertRecordedEvent(
    std::unique_ptr<protocol::Forensics::RecordedEvent> event) {
  JSEvent* js_event = MakeGarbageCollected<JSEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertJSEvent(js_event);
  return Response::Success();
}

Response InspectorForensicsAgent::insertDOMNodeEvent(
    std::unique_ptr<protocol::Forensics::DOMNodeEvent> event) {
  DOMNodeEvent* dom_event =
      MakeGarbageCollected<DOMNodeEvent>(std::move(event));

  if (dom_event->GetEventType() == "DOMCreateNode") {
    ForensicDataStore::GetInstance()->insertVerificationEvent(dom_event);
  }

  if (dom_event->GetNodeType() == "iframe" && dom_event->WasJSExecuting())
    ForensicDataStore::GetInstance()->insertJSInsertedIframeEvent(dom_event);
  if (dom_event->Reason() == "parser")
    ForensicDataStore::GetInstance()->insertDOMNodeEvent(dom_event);

  return Response::Success();
}

Response InspectorForensicsAgent::insertTimerEvent(
    std::unique_ptr<protocol::Forensics::TimerEvent> event) {
  TimerEvent* timer_event = MakeGarbageCollected<TimerEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertTimerEvent(timer_event);
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertIdleCallbackEvent(
    std::unique_ptr<protocol::Forensics::IdleCallbackEvent> event) {
  IdleCallbackEvent* idle_event =
      MakeGarbageCollected<IdleCallbackEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertIdleCallbackEvent(idle_event);
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertAnimationFrameEvent(
    std::unique_ptr<protocol::Forensics::AnimationFrameEvent> event) {
  AnimationFrameEvent* animation_event =
      MakeGarbageCollected<AnimationFrameEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertAnimationFrameEvent(animation_event);
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertGetterEvent(
    std::unique_ptr<protocol::Forensics::GetterEvent> event) {
  GetterEvent* getter_event =
      MakeGarbageCollected<GetterEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertGetterEvent(getter_event);
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertStorageEvent(
    std::unique_ptr<protocol::Forensics::StorageEvent> event) {
  ForensicStorageEvent* storage_event =
      MakeGarbageCollected<ForensicStorageEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertStorageEvent(storage_event);
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertV8Record(
    std::unique_ptr<protocol::Forensics::V8Record> record) {
  ForensicDataStore::GetInstance()->insertV8Records(
      JSONArrayToMap(record->getMathRandomNumbers()),
      JSONArrayToMap(record->getCurrentTimestamps()));
  return Response::Success();
}

protocol::Response InspectorForensicsAgent::insertCryptoRandomEvent(
    std::unique_ptr<protocol::Forensics::CryptoRandomEvent> event) {
  CryptoRandomEvent* crypto_event =
      MakeGarbageCollected<CryptoRandomEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertCryptoRandomEvent(crypto_event);
  return Response::Success();
}

std::unordered_map<std::string, std::vector<double>>
InspectorForensicsAgent::JSONArrayToMap(
    protocol::Array<protocol::Forensics::v8RecordMap>* array) {
  std::unordered_map<std::string, std::vector<double>> map;
  for (auto& it : *array) {
    std::vector<double> numbers;
    for (auto n : *it->getValues()) {
      numbers.push_back(n.ToDouble(nullptr));
    }
    map.insert({it->getSecurityToken().Latin1(), numbers});
  }
  return map;
}

protocol::Response InspectorForensicsAgent::insertScriptExecutionEvent(
    std::unique_ptr<protocol::Forensics::ScriptExecutionEvent> event) {
  ScriptExecutionEvent* script_execution_event =
      MakeGarbageCollected<ScriptExecutionEvent>(std::move(event));
  ForensicDataStore::GetInstance()->insertScriptExecutionEvent(
      script_execution_event);
  return Response::Success();
}

void InspectorForensicsAgent::DidExecuteJSEvent(
    const String& event_type,
    const String& event_interface,
    const String& event_target_interface) {
  VLOG(7) << __func__ << " " << event_type << " " << event_interface << " "
          << event_target_interface;
  GetFrontend()->JSEventExecuted(event_type, event_interface,
                                 event_target_interface);
  base::PlatformThread::Sleep(base::TimeDelta::FromSeconds(3)); // to allow screenshot

}

}  // namespace blink
