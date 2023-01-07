#!/usr/bin/env python3

import sys
import numpy as np

def gen_delta(input_list):
    l1 = input_list[:-1]
    l2 = input_list[1:]
    return np.subtract(l2,l1)

def load_trace_file(input_file,thres):
    ret = []
    with open(input_file) as f:
        for line in f:
            timeval, rmw_evicts = [int(x) for x in line.split(';')]
            if rmw_evicts >= thres:
                ret.append(timeval)
    return ret

def save_diff_file(diff_list,outfile_name):
    with open(outfile_name,'w') as o:
        for val in diff_list:
            o.write(str(val/1000)+'\n')


def main():
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <input_trace> <output_diff> <rmw eviction threshold (~10)>')
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    rmw_thres = int(sys.argv[3])

    print(f"Generating for {input_file}")
    trace = load_trace_file(input_file,rmw_thres)
    print(f"Trace length: {len(trace)}")
    delta = gen_delta(trace)
    save_diff_file(delta,output_file)

if __name__ == '__main__':
    main()
