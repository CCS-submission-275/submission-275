#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_AGENT_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_AGENT_H

#include <unordered_map>
#include "base/macros.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"
#include "third_party/blink/renderer/core/inspector/inspected_frames.h"
#include "third_party/blink/renderer/core/inspector/inspector_base_agent.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/heap/handle.h"

namespace blink {

class Resource;

class CORE_EXPORT InspectorForensicsAgent final
    : public InspectorBaseAgent<protocol::Forensics::Metainfo> {
 public:
  // Needed for using garbage collected members
  void Trace(Visitor*) override;

  explicit InspectorForensicsAgent(InspectedFrames*);
  // Inspector APIs.
  protocol::Response enable() override;

  protocol::Response insertDOMNodeEvent(
      std::unique_ptr<protocol::Forensics::DOMNodeEvent> event) override;
  protocol::Response insertNavigationEvent(
      std::unique_ptr<protocol::Forensics::NavigationEvent> event) override;
  protocol::Response insertLoadResourceEvent(
      std::unique_ptr<protocol::Forensics::LoadResourceEvent> event) override;
  protocol::Response insertDOMAttrEvent(
      std::unique_ptr<protocol::Forensics::DOMAttrEvent> event) override;
  protocol::Response insertScriptExecutionEvent(
      std::unique_ptr<protocol::Forensics::ScriptExecutionEvent> event)
      override;
  protocol::Response insertStorageEvent(
      std::unique_ptr<protocol::Forensics::StorageEvent> event) override;

  protocol::Response insertCryptoRandomEvent(
      std::unique_ptr<protocol::Forensics::CryptoRandomEvent> event) override;

  protocol::Response startReplay() override;
  protocol::Response stopReplay() override;

  protocol::Response startRecord() override;
  protocol::Response stopRecord() override;

  protocol::Response forceRecord() override;

  protocol::Response insertRecordedEvent(
      std::unique_ptr<protocol::Forensics::RecordedEvent> event) override;
  protocol::Response insertTimerEvent(
      std::unique_ptr<protocol::Forensics::TimerEvent> event) override;
  protocol::Response insertIdleCallbackEvent(
      std::unique_ptr<protocol::Forensics::IdleCallbackEvent> event) override;
  protocol::Response insertAnimationFrameEvent(
      std::unique_ptr<protocol::Forensics::AnimationFrameEvent> event) override;
  protocol::Response insertGetterEvent(
      std::unique_ptr<protocol::Forensics::GetterEvent> event) override;
  protocol::Response insertV8Record(
      std::unique_ptr<protocol::Forensics::V8Record> record) override;
  protocol::Response insertPerformanceEvent(
      std::unique_ptr<protocol::Forensics::PerformanceEvent> event) override;

  void DidExecuteJSEvent(const String&, const String&, const String&);

 private:
  // we don't need a template yet
  std::unordered_map<std::string, std::vector<double>> JSONArrayToMap(
      protocol::Array<protocol::Forensics::v8RecordMap>* array);
  InspectorAgentState::Boolean enabled_;
  bool recording_;
  bool replaying_;
  ForensicReplayEngine* replay_engine_;
  Member<InspectedFrames> inspected_frames_;
};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSIC_AGENT_H
