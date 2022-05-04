import sys
import utils
from datetime import datetime
from utils import unescape
import re

from context import FrameContext

BLACKLISTED_SCOPES = {'DedicatedWorkerGlobalScope', 'ServiceWorkerGlobalScope',
                      'ServiceWorkerContainer', 'IDBOpenDBRequest', 'IDBTransaction'}

# NOTE: Must be consistent w/ third_party/blink/renderer/bindings/scripts/v8_attributes.py
REPLAYED_GETTERS = [
    'Document.readyState',
    'Document.title',
    'Document.lastModified',
    'Document.URL',
    'XMLHttpRequest.readyState',
    'XMLHttpRequest.getResponseHeader',
    'Document.cookie',
    'Storage.length',
    'Event.timeStamp',
    'Location.href',
    'Location.search',
    'Location.hostname',
    'Location.pathname',
    'Location.origin',
    'Location.host',
    'Element.clientHeight',
    'Element.clientWidth',
    'Document.compatMode',
    'Document.referrer',
    'Screen.availHeight',
    'Screen.availWidth',
    'PerformanceTiming.navigationStart',
    'PerformanceTiming.domLoading',
    'PerformanceTiming.domInteractive',
    'PerformanceTiming.unloadEventStart',
    'PerformanceTiming.unloadEventEnd',
    'PerformanceTiming.redirectStart',
    'PerformanceTiming.redirectEnd',
    'PerformanceTiming.fetchStart',
    'PerformanceTiming.domainLookupStart',
    'PerformanceTiming.domainLookupEnd',
    'PerformanceTiming.connectStart',
    'PerformanceTiming.connectEnd',
    'PerformanceTiming.secureConnectionStart',
    'PerformanceTiming.requestStart',
    'PerformanceTiming.responseStart',
    'PerformanceTiming.responseEnd',
    'PerformanceTiming.domLoading',
    'PerformanceTiming.domInteractive',
    'PerformanceTiming.domContentLoadedEventStart',
    'PerformanceTiming.domContentLoadedEventEnd',
    'PerformanceTiming.domComplete',
    'PerformanceTiming.loadEventStart',
    'PerformanceTiming.loadEventEnd',
    'DOMRectReadOnly.x',
    'DOMRectReadOnly.y',
    'DOMRectReadOnly.width',
    'DOMRectReadOnly.height',
    'DOMRectReadOnly.top',
    'DOMRectReadOnly.right',
    'DOMRectReadOnly.bottom',
    'DOMRectReadOnly.left',
    'HTMLElement.offsetHeight',
    'HTMLElement.offsetWidth',
    'Window.length',
    'Window.pageYOffset',
    'Window.pageXOffset',
    'UIEvent.which',
    'MouseEvent.offsetX',
    'MouseEvent.offsetY',
    'XMLHttpRequest.statusText',
    'Navigator.userAgent',
    'Navigator.appVersion',
    'Screen.width',
    'Screen.height',
    'PluginArray.length',
    'Plugin.name',
    'Window.devicePixelRatio',
]


# NOTE: Make sure EventType is consistent with browser_protocol.pdl
class EventType:
    # DOM event types
    DOMInsertNode = 'DOMInsertNode'
    DOMCreateNode = 'DOMCreateNode'
    DOMInsertFrame = 'DOMInsertFrame'
    DOMRemoveNode = 'DOMRemoveNode'
    DOMModifyAttr = 'DOMModifyAttr'
    DOMRemoveAttr = 'DOMRemoveAttr'
    FrameNavigation = 'FrameNavigation'
    DidCreateDocumentLoader = 'LoadPage'
    LoadResource = 'LoadResource'
    RecordEvent = 'RecordEvent'
    TimerEvent = 'TimerEvent'
    IdleCallbackEvent = 'IdleCallbackEvent'
    AnimationFrameEvent = 'AnimationFrameEvent'
    GetterEvent = 'GetterEvent'
    XMLHttpRequest = 'JSEvent'
    ScriptExecution = 'ScriptExecution'
    StorageEvent = 'StorageEvent'
    V8Records = 'V8Records'
    CryptoRandomEvent = 'CryptoRandomEvent'
    PerformanceEvent = 'PerformanceEvent'

def skip_log(handler='', entry={}):

    return False

class LogParser:

    def __init__(self, logfile, debug=False,
                 should_replay_dom_event_by_js=False):
        self.logfile = logfile
        self.main_frame_render_pid = None
        self.debug = debug
        self.should_replay_dom_event_by_js = should_replay_dom_event_by_js
        self.context_map = dict()
        self.frame_set = set()
        # Maps hash to scriptId.
        self.script_map = dict()
        self.opened_window_map = dict()

    def parse_logs(self):
        ret = []
        tids = set()
        skip = False
        with open(self.logfile, 'r') as infile:
            for line_no, line in enumerate(infile):
                #m = utils.PATTERN.search(line)
                PATTERN = re.compile(r'.*(\d+\:\d+) \|\| (\d+\.\d+) \|\| LOG::Forensics::(\w+) :(.*)')
                m = PATTERN.search(line)

                if not m:
                    continue
                tid, dt, handler, content = m.groups()
                try:
                    entry = utils.content_to_dict(content)
                except:
                    print("Continuing", content)
                    continue

                if handler == 'DidCallCallbackStartInternal' and entry.get('interface') == 'IntersectionObserverCallback':
                        skip = True
                elif handler == 'DidCallCallbackEndInternal' and entry.get('interface') == 'IntersectionObserverCallback':
                    skip = False

                if skip:
                    continue
                dt = utils.parse_datetime(dt)
                if handler in self.handlers:
                    ret.append(self.__getattribute__(
                        self.handlers[handler])(entry, dt))
        return ret

    def handle_create_dom_node(self, entry, dt):
        output = self.create_base(entry)
        if context := self.context_map[entry['frame']]:
            output['executingEvent'] = context.get_executing_event()
        output['eventType'] = EventType.DOMCreateNode
        return output

    def handle_insert_dom_node(self, entry, dt):
        output = self.create_base(entry)
        output['parentId'] = entry['parentId']
        output['nextSiblingId'] = entry['nextSiblingId']
        output['attributes'] = self.get_attributes(entry)
        output.update({'eventType': EventType.DOMInsertNode})
        node_type = entry['nodeName'].lower().strip()

        output['nodeString'] = entry['nodeType']

        self.set_reason(entry['frame'], output)

        if 'frame' in node_type and 'frameset' not in node_type:
            # NOTE: We handle inserting frame nodes with a different handler.
            return
        if context := self.context_map[entry['frame']]:
            output['executingEvent'] = context.get_executing_event()

        output['nodeType'] = node_type
        if node_type in ('#text', '#comment'):
            text = entry['markup']
            # output['text'] = unescape(text)
            output['text'] = text
            # assert(len(output['text']) == entry['size'], "Corrupted unescape. \
            #       The script's hash wll be incorrect and replay will fai.")
        elif node_type == 'html' and 'doctype' in entry['nodeType'].lower():
            tag, text = entry['nodeType'].lower().split()
            output['nodeType'] = tag.strip()
            output['text'] = text.strip()

        if self.debug:
            output['dt'] = datetime.timestamp(dt)
            output['document'] = entry['document']
            output['entry'] = entry
        return output

    def handle_insert_dom_frame(self, entry, dt):
        output = self.create_base(entry)
        output['parentId'] = entry['parentId']
        output['nextSiblingId'] = entry['nextSiblingId']
        output['attributes'] = self.get_attributes(entry)
        output['subframeId'] = entry['subframeId']
        output['nodeString'] = entry['nodeType']
        self.set_reason(entry['frame'], output)
        output['nodeType'] = entry['nodeName'].lower().strip()
        output.update({'eventType': EventType.DOMInsertFrame})
        frame_id = output['subframeId']
        parent_frame_id = entry['frame']
        if parent_frame_id in self.context_map:
            output['isJSExecuting'] = self.context_map[parent_frame_id].is_js_executing()
        else:
            output['isJSExecuting'] = False

        if frame_id not in self.frame_set:
            self.frame_set.add(frame_id)
            return output
        else:
            return None

    def handle_create_document(self, entry, dt):
        """
            This sets the security origin
        """
        output = self.create_base(entry)
        output['nodeString'] = 'ForensicDocument'
        output.update({'eventType': EventType.DOMInsertNode})
        self.set_reason(entry['frame'], output)
        output['parentId'] = 0
        output['frameUrl'] = entry['frameUrl']
        return output

    def handle_will_remove_dom_node(self, entry, dt):
        output = self.create_base(entry)
        self.set_reason(entry['frame'], output)
        output['parentId'] = entry['parentId']
        output['nodeString'] = entry['nodeType']
        if context := self.context_map[entry['frame']]:
            output['executingEvent'] = context.get_executing_event()
        output.update({'eventType': EventType.DOMRemoveNode})
        return output

    def handle_did_modify_dom_attr(self, entry, dt):
        output = dict()
        output['eventType'] = EventType.DOMModifyAttr
        output['frameId'] = entry['frame']
        output['nodeId'] = entry['nodeId']
        output['attrName'] = entry['attrName']
        output['newValue'] = entry['newValue']
        return output

    def handle_did_remove_dom_attr(self, entry, dt):
        output = dict()
        output['eventType'] = EventType.DOMRemoveAttr
        output['frameId'] = entry['frame']
        output['attrName'] = entry['removedAttr']
        output['nodeString'] = entry['nodeType']
        output['nodeId'] = entry['nodeId']
        return output

    def handle_did_invalidate_style_attr(self, entry, dt):
        # TODO(joey): This event should be impl'd for full DOM verfication.
        entry.update({'eventType': EventType.DOMRemoveAttr})
        return None

    def handle_character_data_modified(self, entry, dt):
        # TODO(joey): This event should be impl'd for full DOM verfication.
        return None

    def handle_frame_navigation(self, entry, dt):
        return {
            'eventType': EventType.FrameNavigation,
            'url': entry['url'],
            'frameId': entry['frame'],
            'replaceCurrentItem': entry['replaceCurrentItem']
        }

    def handle_load_resource(self, entry, dt):
        print(entry['requestUrl'])
        entry['eventType'] = EventType.LoadResource
        return entry

    def handle_load_document(self, entry, dt):
        return {
            'eventType': EventType.DidCreateDocumentLoader,
            'url': entry['url'],
            'WebNavigationType': entry['WebNavigationType'],
            'frameId': entry['frame']
        }

    def handle_stream_response(self, entry, dt):
        entry['eventType'] = EventType.LoadResource
        return entry

    def handle_record_dispatch_event_start(self, entry, dt):
        if entry.get('targetInterfaceName') in BLACKLISTED_SCOPES:
            # print(f"EVENT NOT BEING REPLAYED (BLACKLISTED): {entry}")
            return

        interface = entry.get('interfaceName')
        frame_id = entry['frame']


        context_map = self.context_map[frame_id]
        # HACK: A hack to fix the issue with window events being misfired onto the
        # "document node."

        if entry['eventType'] == 'unload':
            return

        if entry['nodeStr'] == '#document' and entry['eventType'] in {'load', 'beforeunload', 'unload'}:
            entry['targetInterfaceName'] = 'DOMWindow'

        if interface in self.handlers:
            context_map.js_event_execution_begin(entry['eventType'])
            if not context_map.is_nested_execution():
                output = self.__getattribute__(self.handlers[interface])(entry, dt)
                if output:
                    ret_val = context_map.next_sequence_number()
                    output['sequenceNumber'] = ret_val
                    output['timeDelta'] = context_map.get_time_delta(dt)
                    return output
        else:
            # print(f"EVENT NOT BEING REPLAYED: {entry}")
            pass

        # TODO(joey): Add log info when event is not parsed.

    def handle_record_dispatch_event_end(self, entry, dt):
        if entry.get('targetInterfaceName') in BLACKLISTED_SCOPES:
            return

        if entry['eventType'] == 'unload':
            return

        interface = entry.get('interfaceName')
        frame_id = entry['frame']
        context_map = self.context_map[frame_id]
        if interface in self.handlers:
            context_map.js_event_execution_end(entry['eventType'])

    def handle_frame_initialized(self, entry, dt):
        frame_id = entry['frame']
        # NOTE: The frame initialization should only be occurring due to
        # remote frames. We really should check the pids here to verify.
        if frame_id not in self.context_map:
            self.context_map[frame_id] = FrameContext(frame_id)

    def handle_created_window(self, entry, dt):
        created_frame_id = entry.get('createdFrame')
        # we load logs that starts with this frame_id
        # we replay the other log here
        self.opened_window_map[created_frame_id] = entry.get('frame')

    def handle_event_base(self, entry, dt):
        output = dict()
        output['jsEventType'] = entry['eventType']
        output['eventTargetInterfaceName'] = entry['targetInterfaceName']
        output['targetId'] = entry['targetId']
        output['frameId'] = entry['frame']
        output['eventType'] = 'RecordEvent'
        output['interfaceName'] = entry['interfaceName']
        output['bubbles'] = entry['bubbles']
        output['cancelable'] = entry['cancelable']
        output['composed'] = entry['composed']
        return output

    def add_ui_event_base(self, entry, dt):
        output = dict()
        output['firesTouchEvents'] = entry['firesTouchEvents']
        output['hasSourceCapabilities'] = entry['hasSourceCapabilities']
        return output

    def handle_modifier_event_base(self, entry, dt):
        output = dict()
        output['detail'] = entry['detail']
        output['altKey'] = entry['altKey']
        output['ctrlKey'] = entry['ctrlKey']
        output['metaKey'] = entry['metaKey']
        output['shiftKey'] = entry['shiftKey']
        return output

    def handle_mouse_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output.update(self.handle_modifier_event_base(entry, dt))
        output.update(self.add_ui_event_base(entry, dt))
        output['screenX'] = entry['screenX']
        output['screenY'] = entry['screenY']
        output['clientY'] = entry['clientY']
        output['clientX'] = entry['clientX']
        output['movementX'] = entry['movementX']
        output['movementY'] = entry['movementY']
        output['region'] = entry['region']
        output['button'] = entry['buttons']
        output['menuSourceType'] = entry['menuSourceType']
        output['syntheticEventType'] = entry['syntheticEventType']
        output['relatedTargetId'] = entry['relatedTargetId']
        return output

    def handle_focus_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        # output.update(self.add_ui_event_base(entry, dt))
        output['relatedTargetId'] = entry['relatedTargetId']
        return output

    def handle_run_compiled_script_begin(self, entry, dt):
        frame_id, script_id = entry['frame'], entry['scriptID']
        assert (frame_id in self.context_map)
        context = self.context_map[frame_id]

        output = dict()
        output['eventType'] = EventType.ScriptExecution
        output['frameId'] = frame_id
        output['scriptId'] = str(script_id)
        output['hash'], output['url'] = self.context_map[frame_id].script_map[script_id]
        output['sequenceNumber'] = self.context_map[frame_id].next_sequence_number()

        context.script_execution_begin(output['hash'])

        return output

    def handle_run_compiled_script_end(self, entry, dt):
        frame_id, script_id = entry['frame'], entry['scriptID']
        assert (frame_id in self.context_map)
        context = self.context_map[frame_id]
        self.context_map[frame_id].script_execution_finish(context.script_map[script_id][0])

    def handle_post_message_event(self, entry, dt):
        frame_id = entry['frame']
        message_id = entry['messageId']
        self.context_map[frame_id].post_message_info[message_id] = entry

    def handle_message_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output['sourceOrigin'] = entry['sourceOrigin']
        output['lastEventId'] = entry['lastEventId']
        output['message'] = entry['message']
        output['hasUserActive'] = entry['hasUserActive']
        output['hasBeenActive'] = entry['hasBeenActive']
        output['isActive'] = entry['isActive']
        output['portCnt'] = entry['portCnt']
        output['messagePortIds'] = entry['messagePortIds']
        return output

    def handle_keyboard_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output.update(self.add_ui_event_base(entry, dt))
        output.update(self.handle_modifier_event_base(entry, dt))
        output['code'] = entry['code']
        output['key'] = entry['key']
        output['location'] = entry['location']
        output['isComposing'] = entry['isComposing']
        output['charCode'] = entry['charCode']
        output['keyCode'] = entry['keyCode']
        output['repeat'] = entry['repeat']
        return output

    def handle_focus_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output.update(self.add_ui_event_base(entry, dt))
        output['relatedTargetId'] = entry['relatedTargetId']
        return output

    def handle_input_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output.update(self.add_ui_event_base(entry, dt))
        output.update(self.handle_modifier_event_base(entry, dt))
        output['data'] = entry['data']
        output['inputType'] = entry['inputType']
        output['isComposing'] = entry['isComposing']
        return output

    def handle_text_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output.update(self.add_ui_event_base(entry, dt))
        output.update(self.handle_modifier_event_base(entry, dt))
        output['data'] = entry['data']
        return output

    def handle_storage_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output['key'] = entry['key']
        output['oldValue'] = entry['oldValue']
        output['newValue'] = entry['newValue']
        output['url'] = entry['url']
        output['storageType'] = entry['storageType']
        return output

    def handle_page_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        # output['eventType'] = entry['eventType']
        return output

    def handle_progress_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output['loaded'] = entry['loaded']
        output['total'] = entry['total']
        output['lengthComputable'] = entry['lengthComputable']
        return output

    def handle_performance_now(self, entry, dt):
        output = dict()
        output['frameId'] = entry['frame']
        output['eventType'] = EventType.PerformanceEvent
        output['now'] = float(entry['now'])
        return output

    def handle_before_unload_event(self, entry, dt):
        output = dict()
        output.update(self.handle_event_base(entry, dt))
        output['returnValue'] = entry['returnValue']
        return output

    def handle_timer_event_begin(self, entry, dt):

        frame_id = entry['frame']
        output = dict()
        output['frameId'] = frame_id
        output['eventType'] = EventType.TimerEvent
        output['sequenceNumber'] = self.context_map[frame_id].next_sequence_number()
        output['timerId'] = entry['timerID']
        output['timeDelta'] = self.context_map[frame_id].get_time_delta(dt)
        self.context_map[frame_id].timer_event_begin(entry['timerID'])
        return output

    def handle_timer_event_end(self, entry, dt):
        frame_id = entry['frame']
        self.context_map[frame_id].timer_event_finish(entry['timerID'])

    def handle_crypto_values(self, entry, dt):
        output = {}
        output['eventType'] = EventType.CryptoRandomEvent
        output['frameId'] = entry['frame']
        output['url'] = entry['frameUrl']
        output['values'] = entry['values']
        return output

    def handle_animation_frame_begin(self, entry, dt):
        output = dict()
        frame_id = entry['frame']
        output['eventType'] = EventType.AnimationFrameEvent
        output['frameId'] = entry['frame']
        output['callbackId'] = entry['callbackId']
        output['highResNowMsLegacy'] = entry['highResNowMsLegacy']
        output['highResNowMs'] = entry['highResNowMs']
        output['isLegacyCall'] = entry['isLegacyCall']
        output['timeDelta'] = self.context_map[frame_id].get_time_delta(dt)
        self.context_map[frame_id].animation_frame_event_begin(entry['callbackId'])
        return output

    def handle_animation_frame_end(self, entry, dt):
        frame_id = entry['frame']
        self.context_map[frame_id].animation_frame_event_finish(entry['callbackId'])

    def handle_idle_callback_begin(self, entry, dt):
        output = dict()
        frame_id = entry['frame']
        output['eventType'] = EventType.IdleCallbackEvent
        output['frameId'] = entry['frame']
        output['callbackType'] = entry['callbackType']
        output['callbackId'] = entry['callbackId']
        output['allottedTime'] = entry['allottedTime']
        output['timeDelta'] = self.context_map[frame_id].get_time_delta(dt)
        self.context_map[frame_id].idle_callback_event_begin(entry['callbackId'])
        return output

    def handle_idle_callback_end(self, entry, dt):

        frame_id = entry['frame']
        self.context_map[frame_id].idle_callback_event_finish(entry['callbackId'])

    def handle_getter_event(self, entry, dt):
        output = dict()
        interface, attribute = entry['interface'], entry['attribute']
        if f'{interface}.{attribute}' in REPLAYED_GETTERS:
            output['eventType'] = EventType.GetterEvent
            output['frameId'] = entry['frame']
            output['interface'] = entry['interface']
            output['attribute'] = entry['attribute']
            output['returnType'] = entry['returnType']
            output['returnValue'] = entry['returnValue']
        return output

    def handle_did_compile_script(self, entry, dt):
        frame, script, _hash = entry['frame'], entry['scriptID'], entry['hash']
        self.context_map[frame].script_map[script] = (_hash, entry['url'])

    def handle_storage_get_item(self, entry, dt):
        output = dict()
        output['eventType'] = EventType.StorageEvent
        output['apiName'] = 'getItem'
        output['frameId'] = entry['frame']
        output['value'] = entry['value']
        output['type'] = entry['type']
        output['isNull'] = entry['isNull']
        output['key'] = entry['key']
        return output

    def handle_storage_key(self, entry, dt):
        output = dict()
        output['eventType'] = EventType.StorageEvent
        output['apiName'] = 'key'
        output['frameId'] = entry['frame']
        output['key'] = entry['key']
        output['type'] = entry['type']
        output['index'] = entry['index']
        return output

    def handle_do_nothing(self, entry, dt):
        pass

    def set_reason(self, frame_id, output):
        context = self.context_map[frame_id]
        output['reason'] = 'api' if context.is_js_executing() else 'parser'

    def create_base(self, entry):
        output = dict()
        output['nodeId'] = entry['nodeId']
        output['frameId'] = entry['frame']
        output['nodeType'] = (entry.get('nodeName') or entry.get('nodeType', '')).lower()
        return output

    def get_attributes(self, entry):
        attrs = dict()
        if "attributes" in entry:
            for name, value in entry['attributes'].items():
                name, value = unescape(name), unescape(value)
                if name == '' or name == ',':
                    print("Ignoring Attribute (empty name)", entry)
                    continue
                attrs[name] = value
        return attrs

    def handle_v8_records(self, entry, dt):
        entry['eventType'] = EventType.V8Records
        return entry

    handlers = {
        'DidInsertDOMNodeInternal': 'handle_insert_dom_node',
        'DidCreateDOMNodeInternal': 'handle_create_dom_node',
        'DidInsertDOMFrameInternal': 'handle_insert_dom_frame',
        'DidCreateDocumentInternal': 'handle_create_document',
        'WillRemoveDOMNodeInternal': 'handle_will_remove_dom_node',
        'DidModifyDOMAttrInternal': 'handle_did_modify_dom_attr',
        'DidRemoveDOMAttrInternal': 'handle_did_remove_dom_attr',
        'DidInvalidateStyleAttrInternal': 'handle_did_invalidate_style_attr',
        'CharacterDataModifiedInternal': 'handle_character_data_modified',
        # handle_frame_navigation is also responsible for navigation.
        'FrameRequestedNavigationInternal': 'handle_frame_navigation',
        'DidFinishLoadingResourceInternal': 'handle_load_resource',
        'DidAppendDataInternal': 'handle_stream_response',
        # handle_load_document handles navigation.
        'DidCreateDocumentLoaderInternal': 'handle_load_document',
        'WindowCreatedInternal': 'handle_created_window',
        'RecordDispatchEventStartInternal': 'handle_record_dispatch_event_start',
        'RecordDispatchEventEndInternal': 'handle_record_dispatch_event_end',
        'FrameInitializedInternal': 'handle_frame_initialized',
        # Timer Animation, & IdleCallback Events
        'RecordTimerEventIDBeginInternal': 'handle_timer_event_begin',
        'RecordTimerEventIDEndInternal': 'handle_timer_event_end',
        'RecordIdleCallbackEventBeginInternal': 'handle_idle_callback_begin',
        'RecordIdleCallbackEventEndInternal': 'handle_idle_callback_end',
        'RecordRequestAnimationFrameEventBeginInternal': 'handle_animation_frame_begin',
        'RecordRequestAnimationFrameEventEndInternal': 'handle_animation_frame_end',
        # Script event handlers
        'DidCompileScriptInternal': 'handle_did_compile_script',
        'DidRunCompiledScriptBeginInternal': 'handle_run_compiled_script_begin',
        'DidRunCompiledScriptEndInternal': 'handle_run_compiled_script_end',
        # Handle storage events
        'RecordGetItemInternal': 'handle_storage_get_item',
        'RecordKeyInternal': 'handle_storage_key',
        'RecordPerformanceNowEventInternal': 'handle_performance_now',
        # PostMessageEvent Handler
        # 'RecordPostMessageEventInternal' : 'handle_post_message_event',
        # Non Direct Handlers.
        'MouseEvent': 'handle_mouse_event',
        'KeyboardEvent': 'handle_keyboard_event',
        'InputEvent': 'handle_input_event',
        'FocusEvent': 'handle_focus_event',
        'TextEvent': 'handle_text_event',
        'Event': 'handle_event_base',
        'ProgressEvent': 'handle_progress_event',
        'StorageEvent': 'handle_storage_event',
        'MessageEvent': 'handle_message_event',
        'BeforeUnloadEvent': 'handle_before_unload_event',
        'PageTransitionEvent': 'handle_page_event',
        # Getter Events
        'DidCallV8GetAttributeInternal': 'handle_getter_event',
        'DumpV8RecordsInternal': 'handle_v8_records',
        # Crypto Random
        'CryptoRandomValuesInternal': 'handle_crypto_values',

    }


if __name__ == '__main__':
    parser = LogParser(sys.argv[1])
    ret = parser.parse_logs()
