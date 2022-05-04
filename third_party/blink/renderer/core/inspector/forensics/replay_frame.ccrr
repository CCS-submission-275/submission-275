#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"
#include "third_party/blink/public/web/web_frame_load_type.h"
#include "third_party/blink/public/web/web_navigation_params.h"
#include "third_party/blink/renderer/core/dom/comment.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/document_type.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/dom/scripted_idle_task_controller.h"
#include "third_party/blink/renderer/core/dom/text.h"
#include "third_party/blink/renderer/core/execution_context/agent.h"
#include "third_party/blink/renderer/core/execution_context/security_context.h"
#include "third_party/blink/renderer/core/frame/frame.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/core/html/html_frame_element_base.h"
#include "third_party/blink/renderer/core/html/html_frame_owner_element.h"
#include "third_party/blink/renderer/core/html/html_head_element.h"
#include "third_party/blink/renderer/core/html/html_object_element.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"
#include "third_party/blink/renderer/core/inspector/forensics/js_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_xml_http_request.h"
#include "third_party/blink/renderer/core/inspector/identifiers_factory.h"
#include "third_party/blink/renderer/core/loader/document_loader.h"
#include "third_party/blink/renderer/core/loader/frame_load_request.h"
#include "third_party/blink/renderer/core/messaging/message_port.h"
#include "third_party/blink/renderer/platform/heap/heap.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/wtf/functional.h"

namespace blink {

ReplayFrame::ReplayFrame(WTF::String frame_id, LocalFrame* frame) {
  DCHECK(frame);
  frame_ = frame;
  frame_->SetReplayId(frame_id);
  next_message_port_id = 1;
  VLOG(7) << "Creating new replay frame: " << frame_id;
  storage_replayer_ = MakeGarbageCollected<StorageReplayer>(this);
  js_replayer_ = MakeGarbageCollected<JSReplayer>(this);
}

void ReplayFrame::InsertDOMNode(DOMNodeEvent* event) {
  // Get current document

  // VLOG(7) << "Inserting: " << event->ToString();
  Document* document = frame_->GetDocument();
  WTF::String tag_name = event->GetNodeType();
  if (tag_name == String("html")) {
    InsertToNodeMap(event->NodeId(), document->documentElement());
    event->AddAttributes(document->documentElement());
  } else if (tag_name == String("#document")) {
    /*
    // NOTE: this is a hacky way to update the security origin because for some
    websites when a navigation
    // occurs, the eventual URL changes, e.g. www.wikipedia.org ->
    en.wikipedia.org.
    // the request URL starts with www.wikipedia.org and the eventual response
    becomes en.wikipedia.org/...
    // the logs after the document being created have the latter frame URL
    associated
    */
    InsertToNodeMap(event->NodeId(), document);
    KURL url(event->FrameUrl());
    scoped_refptr<SecurityOrigin> security_origin = SecurityOrigin::Create(url);
    security_origin = security_origin->GetOriginForAgentCluster(
        frame_->DomWindow()->GetAgent()->cluster_id());
    document->GetExecutionContext()
        ->GetSecurityContext()
        .SetSecurityOriginForTesting(security_origin);
    document->SetBaseURLOverride(url);
  } else if (tag_name == String("doctype")) {
    // DocumentType* document_type = MakeGarbageCollected<DocumentType>(
    //    document, WTF::String(event->GetText()), WTF::String(),
    //    WTF::String());
    // NOTE: This doesn't actually insert the node.
    // document->SetDoctype(document_type);
  } else if (tag_name == String("body")) {
    InsertToNodeMap(event->NodeId(), document->body());
    event->AddAttributes(document->body());
  } else if (tag_name == String("head")) {
    InsertToNodeMap(event->NodeId(), document->head());
    event->AddAttributes(document->head());
  } else if (tag_name == String("#text")) {
    Text* text_node = document->createTextNode(event->GetText());
    AddChild(event, text_node);
  } else if (tag_name == String("#comment")) {
    Comment* comment = document->createComment(event->GetText());
    AddChild(event, comment);
  } else {
    // we need to handle existing node, instead of creating of new one.
    Node* node = GetNodeById(event->NodeId());
    if (!node) {
      Element* element = event->EventToNode(document);
      AddChild(event, element);
    } else {
      AddChild(event, node);
    }
  }
}

void ReplayFrame::InsertToNodeMap(DOMNodeId id, Node* node) {
  if (node_map_.Contains(id))
    node_map_.erase(id);
  // VLOG(7) << __func__ << " inserting: " << node << ", id:" << id
  // << " document:" << node->GetDocument().Url();
  node_map_.Set(id, node);
}

void ReplayFrame::InsertFrameNode(DOMNodeEvent* event) {
  // VLOG(7) << "Inserting Frame Node: " << event->ToString()
  //         << event->GetSubframeId() << std::endl;

  Element* element = event->EventToNode(GetDocument());
  AddChild(event, element);

  // NOTE: iframe, frame, object, and embed tags all have an underlying local
  // frame that we need to store.
  if (element->tagName() == "OBJECT") {

    VLOG(7) << "inserting object!!";
    auto* frame_node = DynamicTo<HTMLFrameOwnerElement, Element>(element);
    auto task = WTF::Bind(&ReplayFrame::WaitForObjectNodeInsertion,
                          WrapPersistent(this), WrapPersistent(event),
                          WrapPersistent(frame_node));
    auto* engine = ForensicReplayEngine::GetInstance();
    engine->MainFrame()
        ->GetFrame()
        ->GetTaskRunner(TaskType::kDOMManipulation)
        ->PostTask(FROM_HERE, std::move(task));
  } else {
    auto* frame_node = DynamicTo<HTMLFrameOwnerElement, Element>(element);
    if (frame_node->ContentFrame()) {
      LocalFrame* local_frame =
          DynamicTo<LocalFrame>(frame_node->ContentFrame());
      ForensicReplayEngine::GetInstance()->CreateReplayFrame(
          event->GetSubframeId(), local_frame);
    } else {
      // TODO(): I am adding this else statement for now, because I am not
      // sure why frame_node->ContentFrame()
      // returns null for this case. This isn't good, because if any events are
      // attributed to this frame, they will not get dispatched.
      VLOG(7) << "Failed to get content frame for " << event->ToString()
              << event->GetSubframeId();
    }

    VLOG(7) << "Finished insertinf rame!";
  }
}

void ReplayFrame::AddChild(DOMNodeEvent* event, Node* node) {
  DCHECK(node);
  Node* parent = GetParent(event);
  InsertToNodeMap(event->NodeId(), node);
  node->setReplayParserInserted(true);

  if (!parent) {
    // put the current node in to waitinglist
    // VLOG(7) << __func__ << "Didn't find parent for " << node;
    if (awaiting_insert_event_.Contains(event->ParentNodeId())) {
      awaiting_insert_event_.at(event->ParentNodeId())->push_back(event);
    } else {
      auto* init = MakeGarbageCollected<HeapVector<Member<DOMNodeEvent>>>();
      init->push_back(event);
      awaiting_insert_event_.Set(event->ParentNodeId(), init);
    }

    return;
  }

  if (event->NextSiblingId() != 0) {
    Node* refNode = GetNextSibling(event);
    // VLOG(7) << "Insert before" << event->ToString() << std::endl;
    parent->insertBefore(node, refNode);
  } else if (event->ParentNodeId() != 1) {
    // VLOG(7) << "Inserting[AppendChild]: " << event->ToString()
    //         << " as child of " << parent->ToString();
    DynamicTo<ContainerNode>(parent)->AppendChild(node);
  }

  // let's handle the waiting list here
  if (awaiting_insert_event_.Contains(event->NodeId())) {
    auto awaiting_events = awaiting_insert_event_.Take(event->NodeId());
    for (auto awaiting_event : *awaiting_events) {
      AddChild(awaiting_event, GetNodeById(awaiting_event->NodeId()));
    }
  }
}

Node* ReplayFrame::GetParent(DOMNodeEvent* event) {
  return GetNodeById(event->ParentNodeId());
}

Node* ReplayFrame::GetNextSibling(DOMNodeEvent* event) {
  return GetNodeById(event->NextSiblingId());
}

void ReplayFrame::Trace(Visitor* visitor) {
  visitor->Trace(frame_);
  visitor->Trace(node_map_);
  visitor->Trace(awaiting_insert_event_);
  visitor->Trace(js_replayer_);
  visitor->Trace(storage_replayer_);
  visitor->Trace(xml_http_request_map_);
  visitor->Trace(message_port_map_);
}

void ReplayFrame::RemoveDOMNode(DOMNodeEvent* event) {
  Node* parent = GetParent(event);
  // VLOG(7) << __func__ << " get parent node id " << event->ParentNodeId()
  //         << " node to remove: " << event->NodeId();
  if (!parent)
    return;

  Node* node = GetNodeById(event->NodeId());
  if (parent == node->parentNode())
    parent->removeChild(node);
}

Node* ReplayFrame::GetNodeById(int node_id) {
  if (node_id <= 0)
    return nullptr;
  auto node = node_map_.find(node_id);
  if (node != node_map_.end())
    return node->value;
  else
    return nullptr;
}

void ReplayFrame::Navigate(WTF::String url) {
  Document* document = frame_->GetDocument();

  // FrameLoadRequest null_url(document, ResourceRequest(NullURL()));
  // We create a new JSReplayer for this frame when it navigates. This way,
  // the state of the old replayer (in the previous page we had loaded in this
  // frame) does not interfere with this one.
  FrameLoadRequest request(document, ResourceRequest(KURL(url)));

  js_replayer_ = MakeGarbageCollected<JSReplayer>(this);
  node_map_.clear();
  xml_http_request_map_.clear();
  awaiting_insert_event_.clear();
  message_port_map_.clear();
  frame_->Navigate(request, WebFrameLoadType::kStandard);
}

void ReplayFrame::VerifyDOMAttrEvent(Node* node,
                                     const QualifiedName& name,
                                     const AtomicString& value,
                                     ForensicDOMAttrEventType type) {
  // NOTE(): This code works, but is disabled for now.
  return;
  //  if (!isScriptExecuting())
  //    return;
  //
  //
  //  auto* datastore = ForensicDataStore::GetInstance();
  //  auto* event = datastore->NextAttrVerificationEvent(this);
  //  if (event && event->GetEventAttrType() == type)
  //    VLOG(7) << "Replay Pass: " << "mod type: " << type << node->ToString()
  //            << " " << name << "Event name: " << event->GetAttrName();
  //  else if (event)
  //    VLOG(7) << "Replay Diverged: " << "mod type: " << type <<
  //    node->ToString()
  //            << " " << name << event->GetAttrName();
  //  else
  //    VLOG(7) << "Empty queue: " << node->ToString() << " " << name;
}

void ReplayFrame::DivergenceDetected(WTF::String info) {
  ForensicRecorder::ReplayError(this->GetFrame(), info);

  if (strict_replay)
    NOTREACHED() << info;
  else
    VLOG(7) << info;
}

void ReplayFrame::VerifyDOMEvent(Node* node,
                                 ForensicDOMEventType modification_type) {
  // We only need to register nodes when js is executing.
  // NOTE(): This code works, but is disabled for now.
  // NOTE(): the DOM events here are about NODE modifications made by API
  // we need to make sure all the node inserted by API are correctly inserted
  // into the node map such that event dispatcher can find the correct target
  if (!js_replayer_->IsJSExecuting())
    return;

  auto* datastore = ForensicDataStore::GetInstance();
  auto* event = datastore->NextNodeVerificationEvent(this);

  VLOG(7) << __func__ << " JS " << modification_type
          << "nodes: " << node->ToString();
  if (event) {
    VLOG(7) << __func__ << "Node: " << node->nodeName()
            << " Event Node: " << event->GetNodeType();
    // presumably, the node inserted here is made by API calls.
    switch (modification_type) {
      case ForensicDOMEventType::kCreateNode:
        // case ForensicDOMEventType::kInsertNode:
        InsertToNodeMap(event->NodeId(), node);
        break;
      // case ForensicDOMEventType::kRemoveNode:
      //   // remove the node from node map
      //   if (node_map_.Contains(event->NodeId()))
      //     node_map_.erase(event->NodeId());
      //   break;
      default:
        break;
    }
  } else {
    DivergenceDetected(
        "Failed to get event for: " + js_replayer_->CurrentExecutingEvent() +
        " insert " + node->ToString());
    return;
  }

  VLOG(7) << " the corresponding event is " << event->ToString();

  //
  //  if (event && event->GetDOMEventType() == modification_type) {
  //    VLOG(7) << "Replay Pass: " << "mod type: " << modification_type
  //            << event->GetNodeType() << "," << node->ToString();
  //  } else if (event)
  //    VLOG(7) << "Replay Diverged: " << "mod type: " << modification_type
  //            << event->GetNodeType() << "," << node->ToString();
  //  else
  //    VLOG(7) << "Empty queue: " << node->ToString();
}

// NOTE: When <object> nodes are inserted into the DOM, the underlying
// LocalFrame
// is not created during the process of inserting the node (which is the case
// for <iframe> nodes). Instead, the creation of the frame object is delayed
// until after layout when void
// LocalFrameView::FlushAnyPendingPostLayoutTasks() is called. Since this is
// the case, we actually have to delay dispatching anymore events during the
// replay until the <object> node's LocalFrame is created. This callback is
// responsible for pausing the dispatcher and waiting for the creation of the
// object tag's localFrame.
void ReplayFrame::WaitForObjectNodeInsertion(DOMNodeEvent* event,
                                             HTMLFrameOwnerElement* owner) {

  auto* engine = ForensicReplayEngine::GetInstance();
  if (waiting_ > 50) {
    engine->unpauseDispatcher();
    return;
  } else {

    // Probably an SVG.
    waiting_++;
  }

  VLOG(7) << "Waiting for object node insertion."
    << event->ToString() << "Waiting: " << waiting_;

  if (owner->ContentFrame()) {
    engine->unpauseDispatcher();
    engine->CreateReplayFrame(event->GetSubframeId(),
                              DynamicTo<LocalFrame>(owner->ContentFrame()));
  } else {

    engine->pauseDispatcher();
    auto task = WTF::Bind(&ReplayFrame::WaitForObjectNodeInsertion,
                          WrapPersistent(this), WrapPersistent(event),
                          WrapPersistent(owner));
    engine->MainFrame()
        ->GetFrame()
        ->GetTaskRunner(TaskType::kDOMManipulation)
        ->PostTask(FROM_HERE, std::move(task));
  }
}

void ReplayFrame::RegisterMessagePort(int message_port_id, MessagePort* port) {
  // VLOG(7) << "Registering message port: " << message_port_id;
  port->SetMessagePortId(message_port_id);
  message_port_map_.insert(port->GetMessagePortId(), port);
}

MessagePort* ReplayFrame::GetMessagePortById(int target_id) {
  auto it = message_port_map_.find(target_id);
  if (it != message_port_map_.end())
    return message_port_map_.at(target_id);
  else
    return nullptr;
}

void ReplayFrame::RegisterXMLHttpRequestTarget(int target_id,
                                               XMLHttpRequest* req) {
  // VLOG(7) << __func__ << "Registering xmlhttprequest target: " << target_id;
  auto it = xml_http_request_map_.find(target_id);
  DCHECK(it == xml_http_request_map_.end())
      << "Duplicate xmlhttprequest registration" << target_id;
  ReplayXMLHttpRequest* replay_request =
      MakeGarbageCollected<ReplayXMLHttpRequest>();
  replay_request->BindTarget(req);
  xml_http_request_map_.insert(target_id, replay_request);
}

ReplayXMLHttpRequest* ReplayFrame::GetReplayXMLHttpRequestById(int target_id) {
  auto it = xml_http_request_map_.find(target_id);
  if (it != xml_http_request_map_.end()) {
    return xml_http_request_map_.at(target_id);
  } else {
    DivergenceDetected("XMLhttpRequest not found for id" +
                       String::Number(target_id));
    return nullptr;
  }
}

void ReplayFrame::UpdateV8SecurityToken() {
  String token = frame_->GetSecurityContext()->GetSecurityOrigin()->ToString() +
                 GetReplayId();
  v8::V8::SetPlatformInstrumentationSecurityToken(token.Ascii());
}

}  // namespace blink
