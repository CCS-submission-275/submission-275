#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_NODE_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_NODE_EVENT_H_

#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/qualified_name.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_event.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

enum ForensicDOMEventType {
  kRemoveNode,
  kInsertNode,
  kCreateNode,
  kInvalidNode
};

class DOMNodeEvent final : public ForensicEvent {
 public:
  DOMNodeEvent();
  explicit DOMNodeEvent(std::unique_ptr<protocol::Forensics::DOMNodeEvent> event);
  Element* EventToNode(Document* document);
  void AddAttributes(Element* element);
  String ToString() override;
  // Getters
  DOMNodeId NodeId() { return node_id_; }
  DOMNodeId ParentNodeId() { return log_event_->getParentId(0); }
  DOMNodeId NextSiblingId() { return log_event_->getNextSiblingId(0); }
  DOMNodeId PrevSiblingId() { return log_event_->getPrevSiblingId(0); }

  WTF::String FrameId() { return log_event_->getFrameId(); }
  WTF::String GetNodeType() { return log_event_->getNodeType(); }
  WTF::String Reason() { return log_event_->getReason(""); }
  WTF::String GetEventType() { return log_event_->getEventType(); }
  WTF::String NodeString() { return log_event_->getNodeString(""); }
  WTF::String FrameUrl() { return log_event_->getFrameUrl(""); }
  String ExecutingEvent() {
    return log_event_->getExecutingEvent("NoJSExecution");
  }
  // NOTE: WasJSExecuting should only be called on DOMNodeEvents that are frame
  //  events. Otherwise, it will call false.
  bool WasJSExecuting();
  ForensicDOMEventType GetDOMEventType();
  WTF::String GetText() { return log_event_->getText("NoTextFound"); }
  WTF::String GetSubframeId() {
    return log_event_->getSubframeId("notSubframe");
  }
  QualifiedName GetQName();
  bool IsFrameEvent(Document* document);

 private:
  int node_id_;
  // The "audit" log related to this event.
  std::unique_ptr<protocol::Forensics::DOMNodeEvent> log_event_;
};

}  // namespace blink

#endif
