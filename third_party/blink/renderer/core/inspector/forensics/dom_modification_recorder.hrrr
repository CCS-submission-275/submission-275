#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_MODIFICATION_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_DOM_MODIFICATION_RECORDER_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/frame.h"
#include "third_party/blink/renderer/core/frame/frame_owner.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_frame_owner_element.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"

#include "base/logging.h"

#include "v8/include/v8.h"

namespace blink {

static int init_count = 0;

using NodeAttrValPair = std::pair<String, String>;

class CORE_EXPORT DOMModificationRecorder : ForensicRecorder {
  DISALLOW_COPY(DOMModificationRecorder);

 public:
  // Methods called for inserting and removing DOM nodes.
  static void DidCreateDocument(Document* doc, bool forced_xhtml);
  static bool IsWhitespace(Node* node);
  static void DidInsertDOMNode(Node* node,
                               ContainerNode::ChildrenChangeSource source);
  static void DidCreateDOMNode(Node* node);
  // static void WillInsertDOMNode(Node *node);
  static void WillRemoveDOMNode(Node* node);
  // static void WillModifyDOMAttr(Element* element,
  //                               const AtomicString& old_value,
  //                               const AtomicString& new_value);
  static void DidModifyDOMAttr(Element* element,
                               const QualifiedName& name,
                               const AtomicString& new_value);
  static void DidRemoveDOMAttr(Element* element, const QualifiedName& name);
  static void DidInvalidateStyleAttr(Element*);
  static void CharacterDataModified(CharacterData*);
  static void DidInsertDOMFrame(HTMLFrameOwnerElement* owner);
  static void FrameInitialized(Frame* frame);

 private:
  static std::unique_ptr<std::vector<NodeAttrValPair>> copyAttributes(
      Node* node);
  static std::unique_ptr<JSONObject> GetAttributes(
      std::unique_ptr<std::vector<NodeAttrValPair>>);
  static void FrameInitializedInternal(std::unique_ptr<ThreadData> thread_data,
                                       std::unique_ptr<ForensicFrameData> frame,
                                       bool is_top_frame);
  static void DidCreateDocumentInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data,
      bool forced_xhtml);
  static void DidInsertDOMNodeInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data,
      std::unique_ptr<std::vector<NodeAttrValPair>> attributes,
      const String& reason);
  static void DidCreateDOMNodeInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      int node_id, const String& node_type);
  static void CharacterDataModifiedInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data);
  static void DidInvalidateStyleAttrInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data);
  static void DidRemoveDOMAttrInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data,
      const String& name);
  static void DidModifyDOMAttrInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data,
      const String& name,
      const String& value);
  static void WillRemoveDOMNodeInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data);
  static void DidInsertDOMFrameInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame,
      std::unique_ptr<ForensicNodeData> node_data,
      const String& sub_frame_id);
};

}  // namespace blink

#endif
