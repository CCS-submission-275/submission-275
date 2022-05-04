#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/html_names.h"
#include "third_party/blink/renderer/core/html/html_frame_element_base.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#include "third_party/blink/renderer/platform/wtf/text/string_statics.h"
#include "third_party/blink/renderer/core/inspector/protocol/Protocol.h"
#include "third_party/blink/renderer/core/inspector/protocol/Forensics.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"

namespace blink {

//DOMNode Event
DOMNodeEvent::DOMNodeEvent() {}


DOMNodeEvent::DOMNodeEvent(std::unique_ptr<protocol::Forensics::DOMNodeEvent> event) {
  // Store the event so we can access it later.
  log_event_ = std::move(event);
  node_id_ = log_event_->getNodeId();
}

void DOMNodeEvent::AddAttributes(Element* element) {
  if (log_event_->hasAttributes()) {
    protocol::DictionaryValue* attrs = log_event_->getAttributes(nullptr);
    DCHECK(attrs);

    for (size_t i = 0; i < attrs->size(); i++) {
      auto entry = attrs->at(i);
      WTF::String attr_name = entry.first;
      WTF::String value;
      entry.second->asString(&value);

      element->setAttribute(AtomicString(entry.first), AtomicString(value));
      VLOG(7) << __func__ << "name: " << attr_name.Utf8().data()
              << " value: " << value.Utf8().data() << "\n";
    }
  }
}

bool DOMNodeEvent::WasJSExecuting() {
  DCHECK(GetNodeType() == "iframe")
  << "WasJSExecuting can only be called for iframe nodes.";
  return log_event_->getIsJSExecuting(false);
}


ForensicDOMEventType DOMNodeEvent::GetDOMEventType() {
  String type = log_event_->getEventType();
  if (type == "DOMRemoveNode")
    return ForensicDOMEventType::kRemoveNode;
  else if (type == "DOMInsertNode")
    return ForensicDOMEventType::kInsertNode;
  else
    return ForensicDOMEventType::kInvalidNode;
}

Element* DOMNodeEvent::EventToNode(Document* document) {
  Element* element = document->CreateRawElement(GetQName());
  element->SetReplayId(node_id_);
  element->setReplayParserInserted(true);
  DCHECK(element);
  AddAttributes(element);
  return element;
}

bool DOMNodeEvent::IsFrameEvent(Document* document) {
  Element* element = EventToNode(document);
  return IsA<HTMLFrameElementBase, Element>(element);
}


QualifiedName DOMNodeEvent::GetQName() {
  AtomicString ns_uri("http://www.w3.org/1999/xhtml");
  AtomicString tag(log_event_->getNodeType());
  return QualifiedName(g_null_atom, tag, ns_uri);
}


String DOMNodeEvent::ToString() {
  std::string tmp = " NodeID: " + std::to_string(node_id_) + ", "
    + " ParentID: " + std::to_string(log_event_->getParentId(0)) +
    + " NextSiblingId: " + std::to_string(log_event_->getNextSiblingId(0));

  WTF::String out(tmp.c_str(), tmp.size());

  out = out
    + " NodeType: " + log_event_->getNodeType()
    + " EventType: " + log_event_->getEventType()
    + " FrameId: " + FrameId()
    + " Reason: " + Reason();


  return out ;
}

}
