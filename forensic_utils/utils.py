import logging
import json
from datetime import datetime
import re
import glob
import os

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
logger.addHandler(ch)


# Reg pattern to extract datetime, handler, content
# PATTERN = re.compile(
#     r'^\[(\d+):(\d+):(\d+/\d+\.\d+):VERBOSE7:\w+_recorder\.cc\(\d+\)\] LOG::Forensics::(\w+) :(.*)')
PATTERN = re.compile(r'(\d+\:\d+) \|\| (\d+\.\d+) \|\| LOG::Forensics::(\w+) :(.*)')



NODE_TYPES = {
    '1' : "element_node",
    '2' : "Attribute_node",
    '3' : "text_node",
    '4' : "cdata_section_node",
    '7' : "processing_instruction_node",
    '8' : "comment_node",
    '9' : "document_node",
    '10' : "document_type_node",
    '11' : "document_fragment_node",
}


def unescape(string):
    try:
        out = string.encode('utf-8').decode('unicode_escape')
    except UnicodeDecodeError:
        return "FailedToParse"


    if out and out[0] == '"':
        out = out[1:]

    if out and out[-1] == '"':
        out = out[:-1]
    return out


def get_node_id(entry):
    return entry.get('nodeId')


def content_to_dict(content):
    return json.loads(json.loads(content))

def parse_datetime(ts):
    """
    The date/time of the log message, in MMDD/HHMMSS.Milliseconds format
    """
    return datetime.fromtimestamp(float(ts))


def merge_log_files(logfile):
    files = glob.glob(logfile+'*[0-9]*')
    logger.debug(f'Found log files: {files}')
    merged = []
    for file in files:
        with open(file) as f:
            lines = f.readlines()
            for line in lines:
                line = line.strip()
                if not line:
                    continue
                words = line.split(' || ')
                ts = words[1]
                merged.append([float(ts), line])
    merged.sort(key=lambda x: x[0])
    output = logfile+'.merged'
    with open(output, 'w') as f:
        out = [_[1] for _ in merged]
        f.write('\n'.join(out))
    if os.path.exists(output):
        return output
