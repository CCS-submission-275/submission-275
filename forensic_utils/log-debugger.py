#!/usr/bin/python3

import click
import parser
import utils
import json
from collections import defaultdict



class Frame(object):

    def __init__(self):
        self.node_map = defaultdict(list)

class LogDebugger(object):

    def __init__(self, logfile):
        self.parser = parser.LogParser(logfile, True)
        self.logs = [log for log in self.parser.parse_logs() if log]
        self.frame_map = defaultdict(Frame)

        # Setup frame map.
        entry_cnt = 0
        for log in self.logs:
            frame_id = log.get('frameId')
            node_id = log.get('nodeId')
            log['entry_cnt'] =  entry_cnt
            entry_cnt += 1
            if frame_id and node_id:
                self.frame_map[frame_id].node_map[node_id].append(log)

    def get_log_for_id(self, node_id):

        for frame_id, frame in self.frame_map.items():
            print(frame_id)
            for entry in frame.node_map[int(node_id)]:
                print(json.dumps(entry, sort_keys=True, indent=4))


@click.group()
def cli():
    pass


@click.command('find-by-node-id')
@click.argument('logfile')
@click.argument('node-id')
def find_by_node_id(logfile, node_id):
    debugger = LogDebugger(logfile)
    debugger.get_log_for_id(node_id)

if __name__ == '__main__':
    cli.add_command(find_by_node_id)
    cli()
