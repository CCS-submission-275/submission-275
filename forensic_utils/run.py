#!/usr/bin/python3
from logging import raiseExceptions
import click
import time


from controller import ReplayController, RecordController
from utils import merge_log_files

@click.command('load-logs')
@click.argument('logfile', type=click.Path())
@click.option('--merge', is_flag=True)
@click.option('--screenshot', is_flag=True, default=False)
@click.option('--site', type=str, default='default')
def load_logs(logfile, merge, screenshot, site):
    """ Loads the audit logs in @logfile into the datastore."""
    controller = ReplayController(site, screenshot)
    # controller.load_page()

    time.sleep(1)

    if merge:
        datafile = merge_log_files(logfile)
        if not datafile:
            raise Exception('Log files are not merged')
    else:
        datafile = logfile

    controller.load_logs(datafile)
    time.sleep(1)
    controller.start()
    input()


@click.command()
@click.argument('logfile', type=click.Path())
def merge_logs(logfile):
    merge_log_files(logfile)

@click.command('start-replay')
def start_replay():
    """Start replay."""
    controller = ReplayController()
    controller.start()


@click.command('start-record')
@click.option('--screenshot', is_flag=True, default=False)
@click.option('--site', type=str, default='default')
def start_record(screenshot, site):
    commands = {'stop', 'exit', 'start'}
    controller = RecordController(site, screenshot)
    # controller.start()
    while command := input('$>'):
        if command not in commands:
          print(f'Unsupported command! Now, it only supports {commands}')
        if command == 'exit':
            exit()
        elif command == 'stop':
            controller.stop()
        elif command == 'start':
            controller.start()


@click.group()
def cli():
    pass


if __name__ == '__main__':
    cli.add_command(load_logs)
    cli.add_command(start_replay)
    cli.add_command(start_record)
    cli.add_command(merge_logs)
    cli()
