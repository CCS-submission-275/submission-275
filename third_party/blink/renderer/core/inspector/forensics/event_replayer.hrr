#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_REPLAYER_H
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_REPLAYER_H

#include "third_party/blink/renderer/core/events/page_transition_event.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/core/events/mouse_event.h"
#include "third_party/blink/renderer/core/events/keyboard_event.h"


namespace blink {

class ReplayFrame;
class EventInit;
class UIEventInit;
class EventModifierInit;
class JSEvent;
class ReplayFrame;
class JSReplayer;
class StorageEvent;
class ProgressEvent;
class MessageEvent;
class FocusEvent;
class BeforeUnloadEvent;
class FocusEvent;
class PageTransitionEvent;

class EventReplayer final: public GarbageCollected<EventReplayer> {
  public:
    explicit EventReplayer(JSReplayer* replayer): js_replayer_(replayer) {}
    void Trace(Visitor* visitor);
    base::TimeTicks FireEvent(ReplayFrame* frame, JSEvent* log_event);

  private:
    Member<JSReplayer> js_replayer_;
    HeapHashMap<DOMNodeId, Member<Node>> node_map_;
    MouseEvent* ReconstructMouseEvent(ReplayFrame* frame, JSEvent* js_event);
    KeyboardEvent* ReconstructKeyboardEvent(ReplayFrame* frame, JSEvent* js_event);
    StorageEvent* ReconstructStorageEvent(ReplayFrame* frame, JSEvent* js_event);
    ProgressEvent* ReconstructProgressEvent(ReplayFrame* frame, JSEvent* js_event);
    MessageEvent* ReconstructMessageEvent(ReplayFrame* frame, JSEvent* js_event);
    FocusEvent* ReconstructFocusEvent(ReplayFrame* frame, JSEvent* js_event);
    BeforeUnloadEvent* ReconstructBeforeUnloadEvent(ReplayFrame* frame, JSEvent* js_event);
    PageTransitionEvent* ReconstructPageTransitionEvent(ReplayFrame* frame, JSEvent* js_event);
    Event* ReconstructEvent(JSEvent* js_event);
    // Helpers
    void ReconstructEventBase(JSEvent* js_event, EventInit* init);
    void ReconstructUIEventBase(ReplayFrame* frame, JSEvent* js_event, UIEventInit* init);
    void ReconstructEventModifierInit(JSEvent* js_event, EventModifierInit* init);
};

}

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_EVENT_REPLAYER_H
