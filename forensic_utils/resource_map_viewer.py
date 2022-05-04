#!/usr/bin/python3

import click
import re
import pandas as pd

MAP_ENTRY = re.compile(r'\[ResourceMapEntry\]: (.*),(.*),(.*),(.*),(.*)')
KEY_NOT_FOUND = re.compile(r'\["RequestIdentifier"\]: (.*),(.*),(.*),(.*),(.*)')
KEY_FOUND = re.compile(r'\["ResourceHit"\]: (.*),(.*),(.*),(.*),(.*)')

#PATTERN = re.compile(r'(\d+\:\d+) \|\| (\d+\.\d+) \|\| LOG::Forensics::(\w+) :(.*)')
#KEY_NOT_FOUND = '[ResourceDataStoreKeyNotFound]'

class Viewer:

    def __init__(self):
        self.map_entries = set()
        self.missed_entries = set()
        self.hit = set()

    def parse(self, logfile):

        with open(logfile, 'r') as infile:
            for line in infile:

                if m:= MAP_ENTRY.search(line):
                    self.map_entries.add(m.groups())
                elif m:= KEY_NOT_FOUND.search(line):
                    self.missed_entries.add(m.groups())
                elif m := KEY_FOUND.search(line):
                    self.hit.add(','.join(m.groups()))


    def create_output_view(self):

        columns = ["url", "type", "useStream", "origin", "method"]
        html_map = pd.DataFrame(self.map_entries, columns=columns).to_html()
        not_hit = set()
        for entry in self.map_entries:
            if ','.join(entry) not in self.hit:
                not_hit.add(entry)
        html_not_hit = pd.DataFrame(not_hit, columns=columns).to_html()
        html_missed = pd.DataFrame(self.missed_entries, columns=columns).to_html()
        with open('output-resource-map.html', 'w') as outfile:
            outfile.write(
                f'''<h4>Missed requests</h4>{html_missed}<hr/>
                <h4>Not hit requests</h4>{html_not_hit}<hr/>
                <h4>The whole map</h4>{html_map}
                ''')



@click.command('view')
@click.argument('replay-logfile', type=click.Path(exists=True))
def view(replay_logfile):
    '''
    View the State of the Resource Map when a resource request was made.

    NOTE: replay.sh must be recording logs from resource_data_store.
    '''

    viewer = Viewer()
    viewer.parse(replay_logfile)
    viewer.create_output_view()



@click.group()
def cli():
    pass

if __name__ == '__main__':
    cli.add_command(view)
    cli()
