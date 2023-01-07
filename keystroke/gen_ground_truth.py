#!/usr/bin/env python3
import sys
import os
import numpy as np
import re

from pathlib import Path

#def gen_ground_truth(keystroke_dir):
#    #def sorted_alphanumeric(data):
#    #    convert = lambda text: int(text) if text.isdigit() else text.lower()
#    #    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)] 
#    #    return sorted(data, key=alphanum_key)
#    directories = sorted_alphanumeric(os.listdir(keystroke_dir))
#    reference = []
#    for dir_name in directories:
#        reference_user = np.loadtxt(keystroke_dir + "/"+ dir_name + "/latency.txt")
#        for row in reference_user:
#            for lat in row:
#                ground_truth_val = lat
#                reference.append(ground_truth_val)
#    return reference

def save_lats(gt_list,outfile_name):
    with open(outfile_name,'w') as o:
        for val in gt_list:
            o.write(str(val)+'\n')


def load_lats(infile_name):
    return [float(x) for x in Path(infile_name).read_text().split()]
    

def main():
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <keystroke_lat_file> <output_ground_truth_file>')
        return 1

    gt = load_lats(sys.argv[1])
    save_lats(gt,sys.argv[2])


if __name__ == '__main__':
    main()
