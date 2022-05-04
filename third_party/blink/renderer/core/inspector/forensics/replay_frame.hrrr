#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_FRAME_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_FRAME_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_attr_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/event_replayer.h"
#include "third_party/blink/renderer/core/inspector/forensics/idle_callback_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_replayer.h"
#include "third_party/blink/renderer/core/inspector/forensics/script_execution_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/storage_replayer.h"
#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"
#include "third_party/blink/renderer/core/xmlhttprequest/xml_http_request.h"
#include "third_party/blink/renderer/platform/bindings/dom_wrapper_world.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/weborigin/security_origin.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"


#include "v8/include/v8.h"

namespace blink {

class Resource;
class JSEvent;
class ReplayXMLHttpRequest;
class XMLHttpRequest;
class PendingScript;
class KURL;
class ClassicPendingScript;
class MessagePort;

class ReplayFrame final : public GarbageCollected<ReplayFrame> {
 public:
  ReplayFrame(WTF::String frame_id, LocalFrame* frame);
  // DOM Replay Events
  void InsertDOMNode(DOMNodeEvent* event);
  void RemoveDOMNode(DOMNodeEvent* event);
  void InsertFrameNode(DOMNodeEvent* event);
  // Replay JS Events
  void ReplayJSEvent(JSEvent* event);
  // Register and replay xmlhttprequest events using ReplayJSEvent
  void RegisterXMLHttpRequestTarget(int unique_id, XMLHttpRequest* req);
  void RegisterMessagePort(int message_port_id, MessagePort* port);

  //void RegisterIdleCallbackEvent(ExecutionContext*)
  // Replaying Script Execution Events.
  JSReplayer* GetJSReplayer() const {return js_replayer_;}

  StorageReplayer* GetStorageReplayer() const {return storage_replayer_;}
  // Replay Navigation
  void Navigate(WTF::String url);
  // Helpers
  LocalFrame* GetFrame() {return frame_;}
  WTF::String GetReplayId() {return frame_->GetReplayId();}
  WTF::String GetFrameId() {return GetReplayId();}
  Document* GetDocument() {return frame_->GetDocument();}
  Node* GetNodeById(int node_id);
  ReplayXMLHttpRequest* GetReplayXMLHttpRequestById(int target_id);
  MessagePort* GetMessagePortById(int target_id);
  void Trace(Visitor* visitor);
  // Verify DOM modifications.
  void VerifyDOMEvent(Node* node, ForensicDOMEventType modification_type);
  void VerifyDOMAttrEvent(Node* node, const QualifiedName& name,
                          const AtomicString& value, ForensicDOMAttrEventType type);


  // Methods for replaying V8:
  void UpdateV8SecurityToken();
  // Util functions
  void DivergenceDetected(WTF::String info);
  bool isStrictReplay() {return strict_replay;}

  private:
    // If false, the replay will continue if a timer, idle callback, or script fails
    // to replay correctly.
    // Replay frame manages the message port ids.
    int next_message_port_id;
    // how many time to wait for object related to iframe.
    int waiting_ = 0;
    bool strict_replay = false;
    Member<LocalFrame> frame_;
    Member<JSReplayer> js_replayer_;
    Member<StorageReplayer> storage_replayer_;
    HeapHashMap<DOMNodeId, Member<HeapVector<Member<DOMNodeEvent>>>> awaiting_insert_event_;
    HeapHashMap<DOMNodeId, Member<Node>> node_map_;
    HeapHashMap<int, Member<ReplayXMLHttpRequest>> xml_http_request_map_;
    HeapHashMap<int, Member<MessagePort>> message_port_map_;

  // Helper functions for replaying DOM events.
  void InsertToNodeMap(DOMNodeId id, Node* node);
  void WaitForObjectNodeInsertion(DOMNodeEvent* event,
                                  HTMLFrameOwnerElement* owner);
  void AddChild(DOMNodeEvent* event, Node* node);
  Node* GetParent(DOMNodeEvent* event);
  Node* GetNextSibling(DOMNodeEvent* event);
};

}  // namespace blink

#endif
