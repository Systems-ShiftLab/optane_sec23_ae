#!/usr/bin/env python3
import sys
import glob
import statistics
from pathlib import Path

#sender_msg="0101100011010111011001001011101111010000101001100100000011110100010101111111101111000101000110101011" 
sender_msg = Path('msg.txt').read_text().strip()

def gen_stats(inp_list,thres):
    total_cnt = len(inp_list)
    correct = 0
    for idx,val in enumerate(inp_list):
        guessed = 0
        if val >= thres: 
            guessed = 1
        real = int(sender_msg[idx])
        if real == guessed: correct +=1
    fraction = correct/total_cnt
    if fraction < 0.5 : 
        print("flip")
        fraction = 1 - fraction
    return fraction

def get_list(inp_file,column):
    ret = []
    with open(inp_file) as f:
        for line in f:
            val = int(line.split('-')[column])
            ret.append(val)
    return ret

def main():
    if len(sys.argv) != 5:
        print(f'Usage: {sys.argv[0]} <receiver file glob> <column number> <threshold> <output dat file>')
        return

    receiver_file_glob = sys.argv[1]
    col_number  = int(sys.argv[2])
    threshold  = int(sys.argv[3])
    ofile  = sys.argv[4]


    stats = []
    for receiver_file in glob.glob(receiver_file_glob):
        l = get_list(receiver_file,col_number)
        s = gen_stats(l,threshold)
        print(receiver_file,s)
        stats.append(s)

    mn = statistics.mean(stats)
    stdev = 0
    if len(stats) > 1:
        stdev = statistics.stdev(stats)

    with open(ofile,'w') as ofd:
        ofd.write(f"noteboard_acc_mean = {mn}\n")
        ofd.write(f"noteboard_acc_stdev = {stdev}\n")

if __name__ == '__main__':
    main()
