#!/usr/bin/env python3

import sys
import numpy as np
import re

from glob import glob
from gen_stats import gen_stats
from gen_ground_truth import load_lats

OBSERVED_NAME="prober.diffsync"
TRUTH_NAME="ground_truth.txt"

#https://gist.github.com/SeanSyue/8c8ff717681e9ecffc8e43a686e68fd9
def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)] 
    return sorted(data, key=alphanum_key)


def save_time_dist(cum_obs,cum_gt,ofile):
    fd = open(ofile,'w')
    fd.write('index,result,reference\n')
    for (idx,(obs,gt)) in enumerate(zip(cum_obs,cum_gt)):
        fd.write(f'{idx},{obs},{gt}\n')
    fd.close()

def main():
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <result_dir> <final_stats> <final_csv>' )
        return

    target_dir = sys.argv[1]
    ofile_stat = sys.argv[2]
    ofile_csv = sys.argv[3]

    input_list = sorted_alphanumeric(glob(target_dir+'/*'))
    
    cum_diffs = []
    cum_truth = []
    cum_obs = []

    precision_list = []
    recall_list = []

    for input_dir in input_list:
        obs_file = input_dir+'/'+OBSERVED_NAME
        truth_file = input_dir+'/'+TRUTH_NAME
        try:
            obs = load_lats(obs_file)
            gt = load_lats(truth_file)
        except FileNotFoundError as err:
            print(err)
            print(f"Skipping {input_dir}")
            continue

        diffs,fp,fn = gen_stats(obs,gt)

        tp = len(obs) - fp

        prec = tp / (tp+fp)
        recall = tp / (tp+fn)

        cum_obs.extend(obs)
        cum_truth.extend(gt)
        cum_diffs.extend(diffs)

        precision_list.append(prec)
        recall_list.append(recall)

        print(f"Input: {input_dir}, Missed Keystrokes: {fn}, Fake Keystrokes: {fp}, Precision: {prec}, Recall: {recall}")

    from statistics import mean,stdev
    with open(ofile_stat,'w') as stat_out:
        prec_mean = mean(precision_list)
        prec_stdev = stdev(precision_list)
        recall_mean = mean(recall_list)
        recall_stdev = stdev(recall_list)
        stat_out.write(f'prec_mean = {prec_mean:0.2f}\n')
        stat_out.write(f'prec_stdev = {prec_stdev:0.2f}\n')
        stat_out.write(f'recall_mean = {recall_mean:0.2f}\n')
        stat_out.write(f'recall_stdev = {recall_stdev:0.2f}\n')
        

    save_time_dist(cum_obs,cum_truth,ofile_csv)
    exit()


if __name__ == '__main__':
    main()
