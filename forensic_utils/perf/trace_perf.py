import click

import pandas as pd
import json

@click.command()
@click.argument('file')
def analyze(file):
    with open(file) as f:
        data = json.load(f)
        metrics = [[row['dur'], row['name']] for row in data['traceEvents'] if row.get('dur')]
        df = pd.DataFrame(metrics, columns=['duration(ms)', 'api'])
        print(df.groupby('api').describe())


@click.group()
def cli():
    pass

if __name__ == '__main__':
    cli.add_command(analyze)
    cli()
