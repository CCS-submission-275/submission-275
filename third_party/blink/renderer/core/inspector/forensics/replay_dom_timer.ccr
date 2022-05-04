#include "third_party/blink/renderer/core/inspector/forensics/replay_dom_timer.h"

namespace blink {

void ReplayDOMTimer::Trace(Visitor* visitor) {
  visitor->Trace(dom_timer_);
  visitor->Trace(timer_event_);
  visitor->Trace(context_);
}

base::TimeTicks ReplayDOMTimer::Execute() {
  dom_timer_->Fired();
  return base::TimeTicks::Now();
}


} // blink
