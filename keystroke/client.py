#!/usr/bin/env python3

import websocket
import _thread
import time

import click
import os
import re

from pathlib import Path
from gen_ground_truth import load_lats

lats_file = ""
usleep = lambda x: time.sleep(x/1000000.0)

def on_message(ws, message):
    pass
    #print(message)

def on_error(ws, error):
    print(error)
    exit(1)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    global lats_file
    def run(*args):
        latencies = load_lats(lats_file)
        for lat in latencies:
            ws.send("A")
            usleep(lat*1000.0)
        ws.send("B")
        ws.close()
    _thread.start_new_thread(run, ())

@click.command()
@click.option('--websocket_url',default="ws://10.0.0.1:4321", help='Websocket URL')
@click.option('--keystroke_file',help='Keystroke Latency File',required=True)
def main(websocket_url,keystroke_file):
    global lats_file
    lats_file = keystroke_file
    #websocket.enableTrace(False)
    ws = websocket.WebSocketApp(websocket_url,
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close)

    ws.run_forever()


if __name__ == "__main__":
    main()
