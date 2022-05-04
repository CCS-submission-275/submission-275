#!/usr/bin/python3
import click
import parser
import random
import utils
import copy
import json
from collections import defaultdict
from types import SimpleNamespace

import pandas as pd
from urllib.parse import urlparse
from context import FrameContext
from collections import namedtuple

from parser import REPLAYED_GETTERS

BLACKLISTED_APIS = {'PerformanceObserver', 'Performance', 'PerformanceObserverEntryList',
                     'PerformanceEntry', 'PerformanceNavigationTiming',
                    'PerformanceNavigation', 'PerformanceResourceTiming'}

#BLACKLISTED_APIS = {}


V8_RECORD_TYPES = {'math_random_values', 'current_timestamps'}

class Frame:

    def __init__(self):
        self.execution_events = list()
        self.execution_stack = list()
        self.current_event = None
        # A map from script ids to script name (name-col-line).
        self.script_id_map = dict()
        self.v8_records = [] # [type_origin, value]
        self.crypto_random_values = [] # [type_url, values]

    def is_js_executing(self):
        return True if len(self.execution_stack) != 0 else False

    def push_execution_event(self, event, dt):
        ''' Adds a new execution event and sets it to the current event. '''
        # We remove unknown events from the stack if a new event
        # is pushed onto the stack.
        if len(self.execution_stack) > 0:
            prev_event = copy.copy(self.execution_stack[0])
            if 'unknown' not in prev_event.debug_name:
                nested_event = ExecutionEvent.create_nested(prev_event)
                self.pop_execution_event(dt)
                self.push_execution_event(nested_event, dt)
            else:
                self.pop_execution_event(dt)

        # Add new event to stack.
        event.start_dt = dt
        self.execution_stack.insert(0, event)

    def pop_execution_event(self, dt):
        # Remove execution event from the stack.
        assert(len(self.execution_stack) > 0)
        event = self.execution_stack.pop(0)
        # assert(event_id == event.id)
        event.end_dt = dt
        # Add this event to the finished execution events.
        self.execution_events.append(event)

    def add_v8_records(self, security_origin, values, record_type):
        if record_type not in V8_RECORD_TYPES:
            raise Exception('Unsupported record_type: ' + record_type)
        for value in values:
            self.v8_records.append([record_type+'/'+security_origin, value])

    def add_crypto_random_values(self, values, url):
        self.crypto_random_values.append(['crypto_random' + '/' + url, values.encode('utf-8').hex()])

    def prepare_for_export(self):
        while len(self.execution_stack) > 0:
            self.pop_execution_event(None)

    def to_df(self, table):
        # Pop everything off the stack.
        self.prepare_for_export()
        print('='*20+table+'='*20)
        
        dfs = list()
        for event in self.execution_events:
            df = event.to_df()
            #if df.Api[0] != 0:
            dfs.append(df)




        print(f"Exec Events: {len(self.execution_events)}")
        # add the v8 records which should be used in order

        if not options.no_random:
            dfs.append(pd.DataFrame(self.v8_records, columns=['ExecutionEvent', 'Api']))
            dfs.append(pd.DataFrame(self.crypto_random_values, columns=['ExecutionEvent', 'Api']))
        return pd.concat(dfs)

    def to_json(self):
        self.prepare_for_export()
        return {
            'ExecutionEvents' : [event.to_json() for event in self.execution_events],
            'v8Records': self.v8_records,
            'cryptoValues' : self.crypto_random_values,
        }

    def get_current_event(self):
        if len(self.execution_stack) > 0:
            return self.execution_stack[0]
        else:
            return None

class DOMEvent:

    def __init__(self, t, entry):
        self.nodeName = entry['nodeName']
        self.markup_hash = entry['markup']
        self.t = t

    def __str__(self):
        return f'{self.t}: {self.nodeName}: {self.markup_hash}'

    def to_tuple(self):
        return ('NA', self.__str__(), 'NA')

class IFrameInsertionEvent:

    def __init__(self, entry):
        self.entry = entry
        self.hash = hash(entry['nodeType'])

    def __str__(self):
        return f'Iframe insertion: {self.hash}'

    def to_tuple(self):
        return ('NA', self.__str__(), 'NA')

class API:
    def __init__(self, entry):
        self.interface = entry['interface']
        self.attribute = entry['attribute']
        self.method = f'{self.interface}.{self.attribute}'
        self.return_value = ""
        self.arg_list = entry.get('argList')
        self.entry = entry
        if 'return' in entry:
            ret_val = entry['return']
            self.return_val = ret_val if len(ret_val) < 50 else hash(ret_val)

    def __str__(self):
        if options.arguments:
            return f'{self.method}: {self.arg_list}:{self.return_val}'
        else:
            return f'{self.method}: {self.return_val}'

    def to_tuple(self):
        self.arg_list = f'{self.arg_list}'
            #','.join(self.arg_list) if self.arg_list else ''
        return (self.return_value, self.method, 'ArgList')#,'.join(self.arg_list))

class ErrorEvent:
    def __init__(self, entry):
        self.entry = entry
        self.msg = entry['Error']

    def __str__(self):
        return f'ReplayError: {self.msg}'

    def to_tuple(self):
        return ('NA', self.msg, '')


class NavigationEvent:
    def __init__(self, entry):
        self.interface = entry['url']
        self.attribute = ''
        self.arg_list = []
    def __str__(self):
        return f'{self.url}'


class GetterAPI(API):
    def __init__(self, entry):
        super().__init__(entry)
        self.return_value = entry['returnValue']
        if self.interface == 'Node' and self.attribute == 'nodeType':
            node_type = utils.NODE_TYPES[self.return_value]
            self.return_value = f"{self.return_value}({node_type})"

    def __str__(self):
        if len(self.return_value) > 2550:
            return_value = str(hash(self.return_value))
        else:
            return_value = self.return_value + " " + str(hash(self.return_value))

        return f'Getter-{self.interface}.{self.attribute}:{return_value}'

    def name(self):
        return f'{self.interface}.{self.attribute}'

    def __eq__(self, other):
        return (
            self.interface == other.interface and
            self.attribute == other.attribute and
            self.return_value == other.return_value
        )

class ListenerEventAPI(API):
    def __init__(self, entry):
        super().__init__(entry)
        args = entry['argList']
        self.event = entry['argList'][0]
        #self.line = args[1]['line']
        #self.col = args[1]['column']

    def __str__(self):
        out = super().__str__()
        return f'{self.event}:{out}' #{self.line}:{self.col}:{out}'


class CallbackAPI(API):
    def __init__(self, entry):
        super().__init__(entry)
        self.return_val = entry['return']
        if entry['attribute'] in {'clearTimeout', 'clearInterval'}:
            # Get the first parameger
            self.return_val = entry['argList'][0]


class Stub(API):
    def __init__(self, interface):
        self.return_val = ''
        self.return_value = ''
        self.interface = interface
        self.attribute = ''
        self.arg_list = list()
        self.method = f'{self.interface}.{self.attribute}'

class ExecutionEvent:

    error_cnt = 0
    def __init__(self):
        self.id = None
        self.start_dt = None
        self.end_dt = None
        self.event_type = None
        self.debug_name = None
        self.apis = [] if options.getters_only else [Stub('--EventBegin--')]
        self.callbacks_registered = list()

    @staticmethod
    def create_nested(event):
        '''Creates a new event when the events are nested.'''
        if not options.getters_only:
            event.apis.append(Stub("--Nested--"))
        nested_event = copy.copy(event)
        nested_event.apis = [] if options.getters_only else [Stub("---Continued---")]
        return nested_event

    @staticmethod
    def from_unknown(entry, id):
        event = ExecutionEvent()
        event.id = "unknown-" + str(id)
        event.debug_name = event.id
        event.event_type = "unknown"
        return event

    @staticmethod
    def from_error(entry):
        event = ExecutionEvent()
        event.id = "ReplayError-" + str(ExecutionEvent.error_cnt)
        ExecutionEvent.error_cnt += 1
        event.debug_name = event.id
        event.event_type = "replayError"
        return event

    @staticmethod
    def from_script(entry):
        event = ExecutionEvent()
        event.id = 'script-' + str(entry['scriptID'])
        event.event_type = 'script'
        return event

    @staticmethod
    def from_function(entry):
        event = ExecutionEvent()
        script_id, line, col = entry['scriptID'], entry['line'], entry['column']
        event.event_type = 'function'
        event.id = f'function-{col}'
        event.debug_name = event.id
        return event

    @staticmethod
    def from_timer_event(entry):
        event = ExecutionEvent()
        event.id = 'timer-' + str(entry['timerID'])
        event.debug_name = event.id
        event.event_type = 'timer'
        return event

    @staticmethod
    def from_dispatch_event(entry):
        event = ExecutionEvent()
        #NOTE(joey): This id is not unique.
        event.id = f"{entry['nodeStr']}-{entry['eventType']}"
        event.debug_name = event.id
        event.event_type = 'Event'
        return event

    @staticmethod
    def from_idle_callback_event(entry):
        event = ExecutionEvent()
        event.id = 'idleCallback-' + str(entry['callbackId'])
        event.debug_name = event.id
        event.event_type = 'IdleCallback'
        return event

    def from_animation_frame_event(entry):
        event = ExecutionEvent()
        event.id = 'animationCallback-' + str(entry['callbackId'])
        event.debug_name = event.id
        event.event_type = 'AnimationCallback'
        return event

    @staticmethod
    def from_parser(entry):
        event = ExecutionEvent()
        event.id = "parser"
        event.debug_name = "parser"
        event.event_type = "parser"
        return event

    def to_df(self):
        df = pd.DataFrame()
        if len(self.apis) > 1:
            api_strs = [str(api) for api in self.apis]
        else:
            api_strs = [] if options.getters_only else [] #["No APIs invoked."]

        if options.execution_only:
            event_names = [self.debug_name]
            df['ExecutionEvent'] = event_names
            df['Api'] = len(api_strs)
        else:
            event_names = [self.debug_name]*len(api_strs)
            df[f'ExecutionEvent'] = event_names
            df['Api'] = api_strs
        return df

    def to_json(self):


        for api in self.apis:
            api.to_tuple()

        return {
            "ExecutionEvent" : self.debug_name,
            "apis" : [api.to_tuple() for api in self.apis],
        }

    def to_list(self):
        output = list()
        for api in self.apis:
            output.append(api.__str__())
        return output

    def diff_getters(self, other):
        '''
        Compares the getters in this ExecutionEvent to those in other. It
        removes any apis that have the same return value.

        NOTE: WE assume 'self' ExecutionEvent is the recorded one.

        If they are equal, all apis will be removed.
        :return true if equal execution events.
        '''
        assert(options.getters_only)

        print(self.debug_name, other.debug_name)
        if self.debug_name != other.debug_name: return False

        to_remove = set()
        for idx in range(len(self.apis)):
            # othered has more apis.
            if idx == len(other.apis) or self.apis[idx] != other.apis[idx]:
                break
            else:
                to_remove.add(idx)

        self.apis = [v for i,v in enumerate(self.apis) if i not in to_remove]
        other.apis = [v for i,v in enumerate(other.apis) if i not in to_remove]
        return True if len(self.apis) == 0 and len(other.apis) == 0 else False

    def __str__(self):
        return f'{self.debug_name}'


class Tracer:

    def __init__(self, manager, logfile, is_record_trace, unknown_cnt=0):
        self.manager = manager
        self.logfile = logfile
        self.context_map = dict()
        self.frame_map = dict() #defaultdict(Frame)
        self.is_record_trace = is_record_trace
        self.print = False
        self.api_count = 0
        self.unknown_cnt = unknown_cnt

    def get_frame(self, frame_id):
        return self.frame_map[frame_id][-1] if frame_id in self.frame_map else Frame()

    def parse_logs(self, hash_script_map={}):
        '''
        :param hash_script_map: Maps hash values to script identifiers during
        the recording. This will be used in the replay to recover script
        identifiers.  We only need a non-empty hash_script map during replay.
        '''
        self.hash_script_map = hash_script_map
        skip = False
        with open(self.logfile, 'r') as infile:
            for line_no, line in enumerate(infile):
                m = utils.PATTERN.search(line)
                if not m:
                    continue
                tid, dt, handler, content = m.groups()
                # Set initial render thread pid.
                try:
                    entry = utils.content_to_dict(content)
                except:
                    continue
                if handler == 'DidCallCallbackBeginInternal' and entry['interface'] == 'IntersectionObserverCallback':
                    skip = True
                elif handler == 'DidCallCallbackEndInternal' and entry['interface'] == 'IntersectionObserverCallback':
                    skip = False
                if skip:
                    continue
                dt = utils.parse_datetime(dt)
                if handler in self.handlers:
                    handler_func = self.__getattribute__(self.handlers[handler])
                    handler_func(entry, dt)

    def handle_compiled_script_begin(self, entry, dt):
        frame_id, script_id = self.frame_id(entry), entry['scriptID']
        frame = self.get_frame(frame_id)
        event = ExecutionEvent.from_script(entry)
        hash, url, line, col = frame.script_id_map[script_id]
        event.debug_name = f'{hash}-{urlparse(url).path}'
        event.url, event.line, event.hash = url, line, col
        frame.push_execution_event(event, dt)

    def handle_compiled_script_end(self, entry, dt):
        frame_id, script_id = self.frame_id(entry), entry['scriptID']
        frame = self.get_frame(frame_id)
        frame.pop_execution_event(script_id)

    def handle_compile_script(self, entry, dt):
        frame_id, script_id = self.frame_id(entry), entry['scriptID']

        frame = self.get_frame(frame_id)

        if not frame:
            return

        script_map = self.get_frame(frame_id).script_id_map
        url, line, col = entry['url'], entry['line'], entry['column']



        # If this is recording trace, then add this to hash_script_map.
        hash = entry['hash']
        if self.is_record_trace:
            script_map[script_id] = (hash, url, line, col)
            self.hash_script_map[hash] = script_map[script_id]
        else:
            # In replay look up script information using hash_script_map
            if hash in self.hash_script_map:
                script_map[script_id] = self.hash_script_map[hash]
            else:
                script_map[script_id] = ('replay-only-' + str(hash), '?', -1, -1)

    def handle_timer_event_begin(self, entry, dt):
        event = ExecutionEvent.from_timer_event(entry)
        self.push_event(entry, event, dt)

    def handle_event_begin(self, entry, dt):
        event = ExecutionEvent.from_dispatch_event(entry)
        self.push_event(entry, event, dt)

    def handle_idle_callback_begin(self, entry, dt):
        event = ExecutionEvent.from_idle_callback_event(entry)
        self.push_event(entry, event, dt)

    def handle_animation_frame_begin(self, entry, dt):
        event = ExecutionEvent.from_animation_frame_event(entry)
        self.push_event(entry, event, dt)

    def handle_function_begin(self, entry, dt):
        if options.function_level_info:
            event = ExecutionEvent.from_function(entry)
            self.push_event(entry, event, dt)

    def handle_pop_execution_event_end(self, entry, dt):
        self.pop_event(entry, dt)

    def handle_function_end(self, entry, dt):
        if options.function_level_info:
            self.handle_pop_execution_event_end(entry, dt)

    def push_event(self, entry, event, dt):
        ''' generic push '''
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)
        frame.push_execution_event(event, dt)

    def pop_event(self, entry, dt):
        ''' generic pop '''
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)
        frame.pop_execution_event(dt)

    def handle_getter_event(self, entry, dt):
        if options.registration_only:
            return

        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)

        if frame is None:
            return


        event = frame.get_current_event()
        if not event:
            self.unknown_cnt += 1
            event = ExecutionEvent.from_unknown(entry, self.unknown_cnt)
            frame.push_execution_event(event, dt)

        getter = GetterAPI(entry)
        if getter.interface in BLACKLISTED_APIS:
            return

        #XMLHttpRequest.responseURL

        if getter.attribute in {'responseURL'}:
            return
        # Check options
        if options.recorded_only and getter.name() not in REPLAYED_GETTERS:
            return
        if options.return_only and not getter.return_value:
            return

        self.api_count += 1
        event.apis.append(getter)

    def handle_navigation_event(self, entry, dt):
        if not self.is_record_trace and entry['recordId'] == 'recordId':
            # This is the nav event when loading the browser prior to starting
            # the replay.
            return

        print("Inserting navigation event", entry)
        #if entry['url'] == '':
            #return

        frame_id = self.frame_id(entry)
        if frame_id in self.frame_map:
            self.frame_map[frame_id].append(Frame())
        else:
            self.frame_map[frame_id] = [Frame()]

    def handle_api_event(self, entry, dt):
        if options.getters_only:
            return
        # Attribute this api to an execution event.
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)
        if frame is None:
            return

        event = frame.get_current_event()



        if not event:
            # Create an unknown event, which means there is a bug in the
            # recording, where we aren't tracking which Execution event
            # this API occurred in.
            self.unknown_cnt += 1
            event = ExecutionEvent.from_unknown(entry, self.unknown_cnt)
            frame.push_execution_event(event, dt)
        # Add this api to an execution event.
        api = self.create_api_object(entry)

        if api and api.interface not in BLACKLISTED_APIS:
            self.api_count += 1
            event.apis.append(api)

    def handle_replay_error(self, entry, dt):
        if options.getters_only or options.execution_only or options.no_error:
            return
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)
        error = ErrorEvent(entry)
        event = ExecutionEvent.from_error(entry)
        event.apis.append(error)

        frame.push_execution_event(event, dt)
        frame.pop_execution_event(dt)

    def handle_v8_records_internal(self, data, record_type):

        for item in data:
            frame_id = item['securityToken'][-32:]
            security_origin = item['securityToken'][:-32]
            frame = self.get_frame(frame_id)
            if frame is not None:
                frame.add_v8_records(security_origin, item['values'], record_type)

    def handle_v8_records(self, entry, dt):
        math_rnds = entry['mathRandomNumbers']
        current_ts = entry['currentTimestamps']
        self.handle_v8_records_internal(math_rnds, 'math_random_values')
        self.handle_v8_records_internal(current_ts, 'current_timestamps')

    def handle_crypto_random_event(self, entry, dt):
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)
        frame.add_crypto_random_values(entry['values'], entry['frameUrl'])

    def handle_dom_node_insert(self, entry, dt):
        if not options.include_dom_info:
            return

        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)

        if not frame:
            return
        if frame.is_js_executing():
            event = frame.get_current_event()
        else:
            event = ExecutionEvent.from_parser(entry)
            frame.push_execution_event(event, dt)

        dom_event = DOMEvent('insert', entry)
        event.apis.append(dom_event)

    def handle_dom_frame_insertion(self, entry, dt):
        frame_id = self.frame_id(entry)
        frame = self.get_frame(frame_id)

        if not frame:
            return
        if frame.is_js_executing():
            event = frame.get_current_event()
        else:
            return

        iframe_event = IFrameInsertionEvent(entry)
        event.apis.append(iframe_event)

    def create_api_object(self, entry):
        ''' Creates the correct API object depending on the API called.'''

        callback_apis = {
            'setTimeout', 'setInterval', 'clearTimeout', 'clearInterval',
            'requestAnimationFrame', 'requestIdleCallback'
        }

        api_name = entry['attribute']
        if api_name == 'addEventListener' or api_name == 'removeEventListener':
            return ListenerEventAPI(entry)
        elif api_name in callback_apis:
            return CallbackAPI(entry)
        else:
            if not options.registration_only:
                return API(entry)

    def frame_id(self, entry):
        return entry['frame'] if self.is_record_trace else entry['recordId']

    handlers = {
        'DidRunCompiledScriptBeginInternal': 'handle_compiled_script_begin',
        'DidRunCompiledScriptEndInternal': 'handle_pop_execution_event_end',
        'DidCompileScriptInternal': 'handle_compile_script',
        'RecordTimerEventIDBeginInternal': 'handle_timer_event_begin',
        'RecordTimerEventIDEndInternal': 'handle_pop_execution_event_end',
        'RecordDispatchEventStartInternal' : 'handle_event_begin',
        'RecordDispatchEventEndInternal' : 'handle_pop_execution_event_end',
        'RecordRequestAnimationFrameEventBeginInternal' : 'handle_animation_frame_begin',
        'RecordRequestAnimationFrameEventEndInternal' : 'handle_pop_execution_event_end',
        'DidCallV8MethodInternal' : 'handle_api_event',
        'RecordIdleCallbackEventBeginInternal' : 'handle_idle_callback_begin',
        'RecordIdleCallbackEventEndInternal': 'handle_pop_execution_event_end',
        'DidCallV8GetAttributeInternal': 'handle_getter_event',
        'DumpV8RecordsInternal': 'handle_v8_records',
        'DidInsertDOMNodeInternal' : 'handle_dom_node_insert',
        'DidCallFunctionBeginInternal' : 'handle_function_begin',
        'DidCallFunctionEndInternal' : 'handle_function_end',
        'DidCreateDocumentLoaderInternal' : 'handle_navigation_event',
        'CryptoRandomValuesInternal': 'handle_crypto_random_event',
        'DidInsertDOMFrameInternal' : 'handle_dom_frame_insertion',
        'ReplayErrorInternal':  'handle_replay_error',
    }

class TraceManager:

    def __init__(self, record_logs, replay_logs):

        self.record_tracer = Tracer(self, record_logs, True)


        is_double_record = True if options.double_record == True else False
        self.replay_tracer = Tracer(self, replay_logs, is_double_record)
        self.have_traced = False

    def run(self):
        print("Begin Record Trace")
        self.record_tracer.parse_logs()
        hash_script_map = self.record_tracer.hash_script_map
        print("Begin Replay Trace")
        self.replay_tracer.parse_logs(hash_script_map)
        self.have_traced = True

    def gen_html(self, frame_map, classes='table'):
        output = dict()

        idx = 0
        for frame_id, frames in frame_map.items():
            for frame in frames:
                if frame.execution_events:
                    output[f'{frame_id}-{idx}'] = frame.to_df(classes).to_html(
                        classes=classes, float_format='{:2.52f}'.format)
                    idx += 1
        return output

    def export_json(self):

        def helper(frame_map):
            output = dict()
            for frame_id, frames in frame_map.items():
                output[frame_id] = list()
                for frame in frames:
                    if len(frame.execution_events) != 0:
                        output[frame_id].append(frame.to_json())
            return output

        output = {
            'record' : helper(self.record_tracer.frame_map),
            'replay' : helper(self.replay_tracer.frame_map)
        }

        with open('tracer.json', 'w') as outfile:
            json.dump(output, outfile)

    def export(self):
        assert(self.have_traced)

        # Create json output.
        if options.to_json:
            self.export_json()

        # Get templates
        with open('tracer-app/tracer-html.tmpl', 'r') as infile:
            html_template = infile.read()

        with open('tracer-app/table-html.tmpl', 'r') as infile:
            table_template = infile.read()

        # Generate html tables.
        record_html_map = self.gen_html(self.record_tracer.frame_map,
                                        'record-table')
        replay_html_map = self.gen_html(self.replay_tracer.frame_map,
                                        'replay-table')
        tables = list()
        for frame_id in record_html_map.keys():
            record_html = record_html_map[frame_id]

            if frame_id in replay_html_map:
                replay_html = replay_html_map[frame_id]
            else:
                replay_html = "Corrupted"

            output_table = table_template.format(
                record_table=record_html, replay_table=replay_html)
            tables.append(output_table)

        # Generate html output
        output = html_template.format(tables='\n'.join(tables))
        with open('output.html', 'w') as outfile:
            outfile.write(output)

@click.command()
@click.argument('record-trace', type=click.Path(exists=True))
@click.argument('replay-trace', type=click.Path(exists=True))
@click.option('--getters-only', is_flag=True)
@click.option('--recorded-only', is_flag=True)
@click.option('--return-only', is_flag=True)
@click.option('--execution-only', is_flag=True)
@click.option('--registration-only', is_flag=True)
@click.option('--arguments', is_flag=True)
@click.option('--include-dom-info', is_flag=True)
@click.option('--function-level-info', is_flag=True)
@click.option('--double-record', is_flag=True)
@click.option('--no-random', is_flag=True)
@click.option('--no-error', is_flag=True)
@click.option('--to-json', is_flag=True)
def trace(record_trace, replay_trace, getters_only, recorded_only,
          execution_only, registration_only, return_only, arguments,
          include_dom_info, function_level_info, no_random, no_error,
          to_json, double_record):
    '''
    :param record_trace: The file with the recording logs.\n
    :param replay_trace: The file with the replay logs.\n
    :param --getters-only: Only includes attribute getters in the trace.\n
    :param --recorded-only: Only includes attribute getters that are recorded in
    the trace. If --getters-only is not set, then this flag does nothing.\n
    :param --return-only: Only includes attribute getters that return a value in
    the trace. If --getters-only is not set, then this flag does nothing.\n
    :param --execution-only: Only includes execution events in the trace.\n
    :param --arguments: Print the argList for each API called.\n
    :param --include-dom-info: Includes events related to DOM modificatinos to the trace. \n
    :param --registration-only: Only outputs registration events. \n
    :param --function-level-info: Includes function level information. \n
    :param --no-random: Do not include random valeus in trace. \n
    :param --no-error: Do not include ReplayErrors in the trace. \n
    :param --to-json: Output data to json file. \n
    :param --double-record: Compares two recording traces.
    :return:
    '''

    global options
    options_dict = {
        'getters_only' : getters_only,
        'recorded_only' : recorded_only,
        'return_only' : return_only,
        'execution_only' : execution_only,
        'registration_only' : registration_only,
        'arguments': arguments,
        'include_dom_info': include_dom_info,
        'function_level_info' : function_level_info,
        'no_random' : no_random,
        'no_error' : no_error,
        'to_json' : to_json,
        'double_record': double_record,
    }
    options = namedtuple('Struct', options_dict.keys())(*options_dict.values())

    manage = TraceManager(record_trace, replay_trace)
    manage.run()
    manage.export()
    print("api count record: ", manage.record_tracer.api_count)
    print("api count replay: ", manage.replay_tracer.api_count)



@click.group()
def cli():
    pass

if __name__ == '__main__':
    cli.add_command(trace)
    cli()
