#include "third_party/blink/renderer/core/inspector/forensics/dom_modification_recorder.h"
#include "third_party/blink/renderer/core/dom/attribute_collection.h"
#include "third_party/blink/renderer/core/dom/character_data.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/editing/serializers/serialization.h"
#include "third_party/blink/renderer/core/frame/frame.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_frame_element_base.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/core/loader/frame_loader_types.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"

#include <memory>
#include <sstream>

namespace blink {

bool DOMModificationRecorder::IsWhitespace(Node* node) {
  // TODO: pull ignoreWhitespace setting from the frontend and use here.
  return node && node->getNodeType() == Node::kTextNode &&
         node->nodeValue().StripWhiteSpace().length() == 0;
}
void DOMModificationRecorder::FrameInitializedInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    bool is_top_frame) {
  auto message = std::make_unique<JSONObject>();
  message->SetBoolean("isTopFrame", is_top_frame);
  AddThreadData(thread_data, message);
  AddFrameInfo(std::move(frame), message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}
// TODO: Remove after debugging
void DOMModificationRecorder::FrameInitialized(Frame* frame) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!frame || frame->IsRemoteFrame())
    return;

  LocalFrame* local_frame = DynamicTo<LocalFrame>(frame);
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::FrameInitializedInternal,
                     GetThreadData(), copyFrameData(local_frame),
                     frame->Owner() ? false : true));
}
void DOMModificationRecorder::WillRemoveDOMNodeInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  AddNodeInfo(std::move(node_data), message);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::WillRemoveDOMNode(Node* node) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!node || !node->GetDocument().GetFrame())
    return;

  LocalFrame* frame = node->GetDocument().GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::WillRemoveDOMNodeInternal,
                     GetThreadData(), copyFrameData(frame),
                     copyNodeData(node)));
}

void DOMModificationRecorder::DidInsertDOMFrameInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data,
    const String& sub_frame_id) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  AddNodeInfo(std::move(node_data), message);
  message->SetString("subframeId", sub_frame_id);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidInsertDOMFrame(HTMLFrameOwnerElement* owner) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!owner || !owner->GetDocument().GetFrame()) {
    return;
  }
  LocalFrame* frame = owner->GetDocument().GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidInsertDOMFrameInternal,
                     GetThreadData(), copyFrameData(frame), copyNodeData(owner),
                     GetFrameId(owner->ContentFrame()).IsolatedCopy()));
}

/*
void DOMModificationRecorder::WillInsertDOMNode(Node *node) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!node || !node->GetDocument().GetFrame())
    return;

  if (IsWhitespace(node))
    return;

  LocalFrame *frame = node->GetDocument().GetFrame();


  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(frame, message);
  message->SetInteger("parentId", DOMNodeIds::IdForNode(node->parentNode()));
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("markup", CreateMarkup(node));
  message->SetString("nodeName", node_data->nodeName);
  message->SetString("isElementNode", node_data->isElementNode);
  message->SetObject("attributes", GetAttributes(node));

  std::string token = frame->ToTraceValue();
  message->SetString("FrameToken", String(token.c_str(), token.size()));
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}
*/

void DOMModificationRecorder::DidInsertDOMNodeInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    std::unique_ptr<ForensicNodeData> node_data,
    std::unique_ptr<std::vector<NodeAttrValPair>> attributes,
    const String& reason) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("markup", node_data->markup);
  message->SetString("nodeName", node_data->nodeName);
  message->SetBoolean("isElementNode", node_data->isElementNode);
  message->SetObject("attributes", GetAttributes(std::move(attributes)));
  message->SetString("reason", reason);
  message->SetDouble("top", node_data->top);
  message->SetDouble("bottom", node_data->bottom);
  message->SetDouble("left", node_data->left);
  message->SetDouble("right", node_data->right);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidInsertDOMNode(
    Node* node,
    ContainerNode::ChildrenChangeSource reason) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  if (!node || !node->GetDocument().GetFrame())
    return;

  LocalFrame* frame = node->GetDocument().GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &DOMModificationRecorder::DidInsertDOMNodeInternal, GetThreadData(),
          copyFrameData(frame), copyNodeData(node), copyAttributes(node),
          reason == ContainerNode::ChildrenChangeSource::kAPI ? "api"
                                                              : "parser"));
}

void DOMModificationRecorder::DidCreateDOMNode(Node* node) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  if (!node || !node->GetDocument().GetFrame())
    return;

  LocalFrame* frame = node->GetDocument().GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidCreateDOMNodeInternal,
                     GetThreadData(), copyFrameData(frame),
                     IdentifiersFactory::IntIdForNode(node),
                     node->nodeName().IsolatedCopy()));
}

void DOMModificationRecorder::DidCreateDOMNodeInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    int node_id,
    const String& node_type) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("nodeId", node_id);
  message->SetString("nodeType", node_type);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}
/*
void DOMModificationRecorder::WillModifyDOMAttr(Element* element,
                                                const AtomicString& oldValue,
                                                const AtomicString& newValue) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!element || !element->GetDocument().GetFrame())
    return;

  LocalFrame* frame = element->GetDocument().GetFrame();

  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(frame, message);
  message->SetInteger("parentId", DOMNodeIds::IdForNode(element->parentNode()));
  message->SetInteger("prevSiblingId",
                      DOMNodeIds::IdForNode(element->previousSibling()));
  message->SetInteger("nextSiblingId",
                      DOMNodeIds::IdForNode(element->nextSibling()));
  message->SetInteger("nodeId", DOMNodeIds::IdForNode(element));
  message->SetString("nodeType", element->ToString());
  message->SetString("oldValue", oldValue.GetString());
  message->SetString("newValue", newValue.GetString());
  message->SetString("nodeName", element->nodeName());

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}
*/

void DOMModificationRecorder::DidModifyDOMAttrInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data,
    const String& name,
    const String& value) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);

  message->SetString("attrName", name);
  message->SetString("newValue", value);

  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("nodeName", node_data->nodeName);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidModifyDOMAttr(Element* element,
                                               const QualifiedName& name,
                                               const AtomicString& newValue) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!element || !element->GetDocument().GetFrame())
    return;

  LocalFrame* frame = element->GetDocument().GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidModifyDOMAttrInternal,
                     GetThreadData(), copyFrameData(frame),
                     copyNodeData(element), name.ToString().IsolatedCopy(),
                     newValue.GetString().IsolatedCopy()));
}
void DOMModificationRecorder::DidRemoveDOMAttrInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data,
    const String& name) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("nodeName", node_data->nodeName);
  message->SetString("removedAttr", name);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidRemoveDOMAttr(Element* element,
                                               const QualifiedName& name) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!element || !element->GetDocument().GetFrame())
    return;

  LocalFrame* frame = element->GetDocument().GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidRemoveDOMAttrInternal,
                     GetThreadData(), copyFrameData(frame),
                     copyNodeData(element), name.ToString().IsolatedCopy()));
}
void DOMModificationRecorder::DidInvalidateStyleAttrInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("nodeName", node_data->nodeName);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidInvalidateStyleAttr(Element* element) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!element || !element->GetDocument().GetFrame())
    return;

  LocalFrame* frame = element->GetDocument().GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidInvalidateStyleAttrInternal,
                     GetThreadData(), copyFrameData(frame),
                     copyNodeData(element)));
}

void DOMModificationRecorder::CharacterDataModified(CharacterData* data) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!data || !data->GetDocument().GetFrame())
    return;

  LocalFrame* frame = data->GetDocument().GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::CharacterDataModifiedInternal,
                     GetThreadData(), copyFrameData(frame),
                     copyNodeData(data)));
}

void DOMModificationRecorder::CharacterDataModifiedInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetInteger("parentId", node_data->parentId);
  message->SetInteger("prevSiblingId", node_data->prevSiblingId);
  message->SetInteger("nextSiblingId", node_data->nextSiblingId);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType", node_data->nodeType);
  message->SetString("nodeName", node_data->nodeName);
  AddThreadData(thread_data, message);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void DOMModificationRecorder::DidCreateDocument(Document* doc,
                                                bool forced_xhtml) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  LocalFrame* frame = doc->GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&DOMModificationRecorder::DidCreateDocumentInternal,
                     GetThreadData(), copyFrameData(frame), copyNodeData(doc),
                     forced_xhtml));
}

void DOMModificationRecorder::DidCreateDocumentInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame,
    std::unique_ptr<ForensicNodeData> node_data,
    bool forced_xhtml) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame), message);
  message->SetInteger("nodeId", node_data->nodeId);
  message->SetString("nodeType",
                     node_data->nodeType);  // it should be #document
  message->SetBoolean("forcedXHTML", forced_xhtml);
  AddThreadData(thread_data, message);
  // the markup is empty for a #document node

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

std::unique_ptr<std::vector<NodeAttrValPair>>
DOMModificationRecorder::copyAttributes(Node* node) {
  auto attributes = std::make_unique<std::vector<NodeAttrValPair>>();
  Element* element = DynamicTo<Element, Node>(node);
  if (element) {
    for (auto attribute : element->Attributes()) {
      attributes->emplace_back(attribute.LocalName().GetString().IsolatedCopy(),
                               attribute.Value().GetString().IsolatedCopy());
    }
  }
  return attributes;
}

std::unique_ptr<JSONObject> DOMModificationRecorder::GetAttributes(
    std::unique_ptr<std::vector<NodeAttrValPair>> attrs) {
  std::unique_ptr<JSONObject> attributes = std::make_unique<JSONObject>();

  for (auto attr : *attrs) {
    attributes->SetString(attr.first, attr.second);
  }

  return attributes;
}
}  // namespace blink
