#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_DOM_TIMER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_DOM_TIMER_H_

#include "base/time/time.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/core/frame/dom_timer.h"
#include "third_party/blink/renderer/core/inspector/forensics/timer_event.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"

namespace blink {

class ReplayDOMTimer : public GarbageCollected<ReplayDOMTimer>{

  public:
    void Trace(Visitor* visitor);
    void bindDOMTimer(ExecutionContext* context, DOMTimer* dom_timer) {context_ = context; dom_timer_ = dom_timer;}
    // Synchronously executes the callback registered with SetTimeout or SetInterval.
    base::TimeTicks Execute();

  private:
    Member<DOMTimer> dom_timer_;
    Member<TimerEvent>  timer_event_;
    Member<ExecutionContext> context_;
};

} // blink

#endif //THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_REPLAY_DOM_TIMER_H_
