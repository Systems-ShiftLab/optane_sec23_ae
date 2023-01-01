import websocket
import _thread
import time

import click
import tqdm
import os
import re
import numpy as np

directory = ""
usleep = lambda x: time.sleep(x/1000000.0)

#https://gist.github.com/SeanSyue/8c8ff717681e9ecffc8e43a686e68fd9
def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)] 
    return sorted(data, key=alphanum_key)

def on_message(ws, message):
    pass
    #print(message)

def on_error(ws, error):
    print(error)
    exit(1)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    global directory
    def run(*args):
        directories = sorted_alphanumeric(os.listdir(directory))
        for dir_name in tqdm.tqdm(directories):
            latencies = np.loadtxt(directory + "/"+ dir_name + "/latency.txt")
            for row in latencies:
                for lat in row:
                    ws.send("A")
                    usleep(lat*1000.0)
            #for last byte
            ws.send("B")
        ws.close()
    _thread.start_new_thread(run, ())

@click.command()
@click.option('--websocket_url',default="ws://localhost:8080", help='Websocket URL')
@click.option('--keystroke_dir',help='Keystroke Directory',required=True)
def main(websocket_url,keystroke_dir):
    global directory
    directory = keystroke_dir
    #websocket.enableTrace(False)
    ws = websocket.WebSocketApp(websocket_url,
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close)

    ws.run_forever()


if __name__ == "__main__":
    main()
