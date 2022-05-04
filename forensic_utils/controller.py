#!/usr/bin/python3

import pychrome
import base64
import parser
import time
from utils import logger
import pathlib
import pyautogui

BASE_DIR = pathlib.Path(__file__).parent

RECORDINGS = BASE_DIR.parent / 'forensic_recordings'

event_handler_map = {
    parser.EventType.DOMInsertNode: 'insertDOMNodeEvent',
    parser.EventType.DOMCreateNode: 'insertDOMNodeEvent',
    parser.EventType.DOMRemoveNode: 'insertDOMNodeEvent',
    parser.EventType.FrameNavigation: 'insertNavigationEvent',
    parser.EventType.DOMInsertFrame: 'insertDOMNodeEvent',
    parser.EventType.LoadResource: 'insertLoadResourceEvent',
    parser.EventType.DidCreateDocumentLoader: 'insertNavigationEvent',
    parser.EventType.RecordEvent: 'insertRecordedEvent',
    parser.EventType.DOMModifyAttr: 'insertDOMAttrEvent',
    parser.EventType.DOMRemoveAttr: 'insertDOMAttrEvent',
    parser.EventType.TimerEvent: 'insertTimerEvent',
    parser.EventType.IdleCallbackEvent: 'insertIdleCallbackEvent',
    parser.EventType.GetterEvent: 'insertGetterEvent',
    parser.EventType.XMLHttpRequest: 'insertJSEvent',
    parser.EventType.ScriptExecution: 'insertScriptExecutionEvent',
    parser.EventType.StorageEvent: 'insertStorageEvent',
    parser.EventType.V8Records: 'insertV8Record',
    parser.EventType.AnimationFrameEvent: 'insertAnimationFrameEvent',
    parser.EventType.CryptoRandomEvent: 'insertCryptoRandomEvent',
    parser. EventType.PerformanceEvent: 'insertPerformanceEvent',
}


class Controller:
    def __init__(self, site: str, taking_screenshot=False):
        # Initialize connection with DevTools.
        self.browser = pychrome.Browser(url="http://127.0.0.1:9222")
        # self.chrome.attach_to_browser_target()
        # XXX: Currently, we assume only one tab.
        tabs = self.browser.list_tab()
        # session_id_m, _ = self.chrome.Target.attachToTarget(
            # targetId=tabs[0]['id'], flatten=True)
        # self.session_id = session_id_m['result']['sessionId']
        # Attach to Forensics domain.
        # self.chrome.Forensics.enable(sessionId=self.session_id)
        print(tabs)
        self.tab = tabs[0]
        self.folder = RECORDINGS / site
        self.tab.start()

        if taking_screenshot:
            self.tab.call_method("Forensics.enable")
            if not (self.folder).exists:
                (self.folder).mkdir()
            self.tab.set_listener('Forensics.JSEventExecuted', self.take_screenshot)
            print("set screenshot listener")

        self.mode = 'NotStarted'

    def take_screenshot(self, eventType=None, interfaceName=None, eventTargetInterfaceName=None):
        event = f"{time.time()}-{self.mode}-{eventTargetInterfaceName}-{interfaceName}-{eventType}"
        print("taking screenshot for", event)
        pyautogui.screenshot(self.folder / f'{event}.png')



class RecordController(Controller):
    def __init__(self, site='default', taking_screenshot=False):
        super().__init__(site, taking_screenshot)
        self.mode = 'Record'

    def start(self):
        self.tab.call_method('Forensics.startRecord')

    def stop(self):
        self.tab.call_method('Forensics.stopRecord')

class ReplayController(Controller):
    def __init__(self, site='default', taking_screenshot=False):
        super().__init__(site, taking_screenshot)
        self.mode = 'Replay'


    def load_logs(self, logfile):
        """ Loads logs into Forensic agent. """

        # Parses all events in the logfile.
        self.parser = parser.LogParser(logfile,
                                       should_replay_dom_event_by_js=False)

        # events is a list of json objects, where each entry in the list represents
        # a single logfile.
        events = self.parser.parse_logs()

        for event in events:
            if not event:
                continue

            # The method determines which API to send the logfile to.
            method = event_handler_map.get(event['eventType'])
            if method:
                #NOTE: The session ID determines which tab to send the event to.
                res = self.tab.call_method(f'Forensics.{method}', event=event)
                if res:
                    logger.debug(f'Error: {method}: {event}')
                    logger.debug(f'Result: {res}')
            else:
                logger.debug(f'No handler for {event}')

    def start(self):
        logger.debug(f'Start replay')
        self.tab.call_method('Forensics.startReplay')

    def load_page(self, url='about:blank'):
        logger.debug(f'Load {url}')
        self.tab.call_method('Forensics.loadPage', url=url)

    def stop(self):
        self.tab.call_method('Forensics.stopReplay')
