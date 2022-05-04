from datetime import datetime, timedelta

class FrameContext:

    def __init__(self, frame_id):
        self.frame_id = frame_id
        # Maps hashes to script ids.
        self.script_map = dict()
        # Help w/ determining if executing JS.
        self.execution_stack = list()
        self.in_timer_event = False
        self.is_script_executing = False
        self.in_idle_callback_event = False
        self.in_animation_callback_event = False
        self.in_js_event = False
        # The id of the current execution unit. It may be a script, timer, or
        # idle callback id.
        self.current_id = None
        # Helping w/ maintaining timer deltas.
        self.is_first_event = True
        self.sequence_number = 0
        self.last_event_ts = None
        # We record postMesage events using two hooks. This map holds information
        # recorded in the first hook until the message from the 2nd hook comes.
        self.post_message_info = dict()

    def is_js_executing(self):
        return len(self.execution_stack) != 0

    def is_nested_execution(self):
        return len(self.execution_stack) > 1

    def is_callback_executing(self):
        return (self.in_timer_event
                or self.in_idle_callback_event
                or self.in_animation_callback_event)

    def script_execution_begin(self, script_id):
        self.current_id = script_id
        self.execution_stack.insert(0, f"script-{script_id}")

    def script_execution_finish(self, script_id):
        assert f"script-{script_id}" == self.execution_stack[0], f"{self.execution_stack[0]}, {script_id}"
        self.is_script_executing = False
        self.current_script = None
        self.execution_stack.pop(0)

    def timer_event_begin(self, timer_id):
        assert self.is_js_executing() == False, self.execution_stack
        self.current_id = timer_id
        self.in_timer_event = True
        self.execution_stack.insert(0, f"timer-{timer_id}")

    def timer_event_finish(self, timer_id):
        assert(f"timer-{timer_id}" == self.execution_stack[0])
        self.execution_stack.pop(0)
        self.in_timer_event = False

    def idle_callback_event_begin(self, callback_id):
        assert self.is_js_executing() == False, self.execution_stack
        self.current_id = callback_id
        self.execution_stack.insert(0, f"idle-{callback_id}")
        self.in_idle_callback_event = True

    def idle_callback_event_finish(self, callback_id):
        assert(f"idle-{callback_id}" == self.execution_stack[0])
        self.execution_stack.pop(0)
        self.in_idle_callback_event = False

    def animation_frame_event_begin(self, callback_id):
        assert(self.is_js_executing() == False)
        self.current_id = callback_id
        self.execution_stack.insert(0, f"animation-{callback_id}")
        self.in_animation_callback_event = True

    def animation_frame_event_finish(self, callback_id):
        assert(f"animation-{callback_id}" == self.execution_stack[0])
        self.execution_stack.pop(0)
        self.in_animation_callback_event = False

    def js_event_execution_begin(self, event_id):
        self.execution_stack.insert(0, f"event-{event_id}")
        self.current_id = event_id
        self.in_js_event = True

    def js_event_execution_end(self, event_id):
        assert(f"event-{event_id}" == self.execution_stack[0])
        self.execution_stack.pop(0)
        self.in_js_event = False

    def next_sequence_number(self):
        ret_val = self.sequence_number
        self.sequence_number += 1
        return ret_val

    def get_time_delta(self, dt):
        """
        :param dt: The datetime for the current event.
        :return: The milliseconds between the previous event and this event.
        """
        ts = datetime.timestamp(dt)

        if self.is_first_event:
            self.is_first_event = False
            delta = 0
        else:
            delta = (ts - self.last_event_ts)*1000

        assert(delta >= 0)
        self.last_event_ts = ts
        return delta

    def get_executing_event(self):
        return self.execution_stack[0] if len(self.execution_stack) else 'NoJSExecution'
