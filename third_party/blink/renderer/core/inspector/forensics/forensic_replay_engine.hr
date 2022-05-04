#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_ENGINE_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_ENGINE_H_

#include "third_party/blink/renderer/core/frame/frame.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_node_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/dom_attr_event.h"
#include "third_party/blink/renderer/core/inspector/forensics/replay_frame.h"
#include "third_party/blink/renderer/core/inspector/forensics/navigation_event.h"
#include "third_party/blink/renderer/modules/storage/storage_area.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_data_store.h"
#include "third_party/blink/renderer/core/inspector/inspected_frames.h"
#include "third_party/blink/renderer/core/dom/dom_node_ids.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "base/memory/singleton.h"
#include "third_party/blink/renderer/core/script/pending_script.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"
#include "v8/include/v8.h"

namespace blink {

class JSEvent;

class XMLHttpRequest;

class KURL;
class JSReplayer;
class ReplayFrame;
class InspectedFrames;
class ClassicPendingScript;
class PendingScript;
class ForensicDataStore;
class MessagePort;

class CORE_EXPORT ForensicReplayEngine {
  USING_FAST_MALLOC(ForensicReplayEngine);

  public:
    static ForensicReplayEngine* GetInstance();
    bool replay() {return replaying_;}
    void setReplay(bool v) {replaying_ = v;}
    void pauseDispatcher(){dispatcher_paused_ = true;}
    void unpauseDispatcher(){dispatcher_paused_ = false;}
    void VerifyDOMEvent(Node *node, ForensicDOMEventType modification_type);
    void VerifyDOMAttrEvent(Node* node, const QualifiedName& name,
                            const AtomicString& value, ForensicDOMAttrEventType type);
    ReplayFrame* CreateReplayFrame(String frame_id, LocalFrame* frame);
    void Initialize(InspectedFrames* inspected_frames);
    ReplayFrame* MainFrame() {return main_frame_;}
    void StartReplay();


    // Register Iframe creation.
    void RegisterFrameCreation(LocalFrame* frame, HTMLFrameOwnerElement* owner);
    // Register Timer & IdleCallback Events
    void RegisterTimerEvent(ExecutionContext* context, DOMTimer* dom_timer, bool is_one_shot);
    void RegisterIdleCallbackEvent(ExecutionContext* context, int callback_id);

    void RegisterXMLHttpRequestTarget(ExecutionContext* context,
                                      int unique_id,
                                      XMLHttpRequest* xml_http_request);
    void RegisterMessagePort(ExecutionContext* context,
                             MessagePort* message_port);

    void RegisterAnimationFrameEvent(
        ExecutionContext* context,
        FrameRequestCallbackCollection::FrameCallback* callback);
    // Register pending scripts
    void RegisterClassicPendingScript(ClassicPendingScript* script);
    void ExecutePendingInSyncScript(String script_source,
                                    KURL script_url,
                                    PendingScript* script);

    double ReplayPerformanceEvent(ExecutionContext* context);
    // Replay Storage Events (LocalStorage/SessionStorage)
    String ReplayStorageGetItem(LocalFrame* frame,
                              const String& key,
                              StorageArea::StorageType storage_type);
    String ReplayStorageKey(LocalFrame* frame,
                            String key,
                            StorageArea::StorageType storage_type);
    String ReplayCryptoRandomValues();
    // Replay Getter Events (non-deterministic property values).
    std::pair<bool, String> ReplayGetterEventString(ExecutionContext* context,
                                                    String interface,
                                                    String attribute);
    std::pair<bool, uint16_t> ReplayGetterEventuint16_t(ExecutionContext* context,
                                                        String interface,
                                                        String attribute);
    std::pair<bool, uint32_t> ReplayGetterEventuint32_t(ExecutionContext* context,
                                                        String interface,
                                                        String attribute);
    std::pair<bool, uint64_t> ReplayGetterEventuint64_t(ExecutionContext* context,
                                                        String interface,
                                                        String attribute);
    std::pair<bool, int64_t> ReplayGetterEventint64_t(ExecutionContext* context,
                                                       String interface,
                                                       String attribute);
    std::pair<bool, double> ReplayGetterEventdouble(ExecutionContext* context,
                                                    String interface,
                                                    String attribute);

    std::pair<bool, int32_t> ReplayGetterEventint32_t(ExecutionContext* context,
                                                      String interface,
                                                      String attribute);

    // Utility Methods
    bool IsJSExecuting(ExecutionContext* context);

    // Frame getters
    ReplayFrame* CurrentFrame(WTF::String frame_id);
    ReplayFrame* CurrentFrame(Node* node);
    ReplayFrame* CurrentFrame(LocalFrame* frame);
    ReplayFrame* CurrentFrame(ExecutionContext* context);

  private:
    ForensicReplayEngine();
    bool initialized_;
    bool is_replay_finished_;
    Persistent<ReplayFrame> main_frame_;
    ForensicDataStore* datastore_;
    Persistent<InspectedFrames> inspected_frames_;
    HashMap<DOMNodeId, Persistent<Node>> node_map_;
    HashMap<WTF::String, Persistent<ReplayFrame>> frame_map_;
    bool replaying_;

    bool dispatcher_paused_;
    Persistent<DOMNodeEvent> current_event_;

    Document* GetDocument();
    void UpdateFrameMap(Element* element);

    // Responsible for dispatching events to the correct replay frame.
    void HandleNavigationEvent(NavigationEvent* event);
    void HandleReplayJSEvent(JSEvent* event);
    void HandleDOMNodeEvent(DOMNodeEvent* event);
    void HandleTimerEvent(TimerEvent* event);
    void HandleIdleCallbackEvent(IdleCallbackEvent* event);
    void HandleXMLHttpRequestEvent(JSEvent* event);
    void HandleScriptExecutionEvent(ScriptExecutionEvent* event);
    void HandleAnimationFrameEvent(AnimationFrameEvent* event);

    void Dispatch();
    void DispatchAsync();



    // Replay non-deterministic attribute getters.
    std::pair<bool, String> ReplayGetterEventInternal(
        ExecutionContext* context,
        String interface,
        String attribute);


    friend base::DefaultSingletonTraits<ForensicReplayEngine>;
};

} // blink

#endif
