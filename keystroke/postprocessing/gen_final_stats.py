#!/usr/bin/env python3

import sys
import numpy as np
import glob

from  gen_ground_truth import load_ground_truth

def load_delta_file(input_file):
    ret = []
    with open(input_file) as f:
        for line in f:
            timediff = int(line)
            ret.append(timediff/1000)
    return ret

def sync_start(observed_list,ground_truth_list,sync_criterion):
    MAX_JITTER_MS=10
    def check_bnd(exp_val,truth):
        diff = abs(exp_val - truth)
        return diff <= MAX_JITTER_MS

    def strict_crit():
        return check_bnd(observed_list[0],ground_truth_list[0]) and check_bnd(observed_list[1],ground_truth_list[1])

    def lenient_crit():
        return check_bnd(observed_list[0],ground_truth_list[0]) or check_bnd(observed_list[1],ground_truth_list[1]) or check_bnd(observed_list[2],ground_truth_list[2]) 

    crit = strict_crit
    if sync_criterion == 'lenient':
        crit = lenient_crit

    TRY_REMOVE=10
    for _ in range(TRY_REMOVE):
        if crit():
            return True
        del(observed_list[0])
    return False


def gen_stats(observed_list,ground_truth_list):
    JITTER_THRES=20
    results = [x for x in observed_list if x >= 15]
    reference = ground_truth_list

    diffs = []
    i = 0
    i_ref = 0
    errors = 0

    while i < len(results)-1 and i_ref < len(reference)-1:
        res = results[i]
        ref = reference[i_ref]
        d = abs(res - ref)
        
        if d < JITTER_THRES:
            diffs.append(d)
            i += 1
            i_ref += 1
            continue


        combined_result = results[i] + results[i+1]
        combined_ref    = reference[i_ref] + reference[i_ref+1]

        diff_combined_result    = abs(combined_result - ref)
        diff_combined_reference = abs(combined_ref - res)
        
        if diff_combined_result < diff_combined_reference:
            i += 2
            i_ref += 1
            diffs.append(diff_combined_result)
            errors += 1
        else:
            i += 1
            i_ref += 2
            diffs.append(diff_combined_reference)
            errors += 1

    return diffs, errors

def save_time_dist(results,reference,outfile):
    diffs,_ = gen_stats(results,reference)
    N = len(diffs)
    data = np.array([np.arange(N), results[:N], reference[:N]]).T
    np.savetxt(outfile, data, delimiter=';', header="index;result;reference")


def main():
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} <target_dir> <ground_truth_file> <sync_criterion -- strict/lenient>')
        return

    target_dir = sys.argv[1]
    ground_truth_file = sys.argv[2]
    sync_criterion = sys.argv[3]
    gt_list = load_ground_truth(ground_truth_file)

    input_file_list = glob.glob(target_dir+'/*.txt.diff')

    errors_dict = {}
    diffs_dict = {}
    for input_file in input_file_list:
        delta_list = load_delta_file(input_file)
        if not sync_start(delta_list,gt_list,sync_criterion):
            print("unsync file",input_file)
            continue
        diff,errors = gen_stats(delta_list,gt_list)
        errors_dict[input_file] = errors
        diffs_dict[input_file] = diff


    from statistics import mean,stdev
    errors_lst = errors_dict.values()
    errors_lst = sorted(errors_lst)[0:100] #Remove outliers
    prcnt = [(x*100)/len(gt_list) for x in errors_lst]
    mn = mean(prcnt)
    sdev = stdev(prcnt)
    print(errors_lst)
    print(prcnt)
    print (mn,sdev)

#TODO: This was written in a hurry
    good_files = sorted(errors_dict.keys(),key= lambda x : errors_dict[x])[0:100]
    all_diffs = []
    for fl in good_files:
        print(fl)
        all_diffs.extend(diffs_dict[fl])

    all_diffs = [x for x in all_diffs if x < 20]

    hist, bins = np.histogram(all_diffs,bins=20,range=(0,20+2))
    hist = hist / len(all_diffs)
    data = np.array([np.arange(20), bins[:-1], hist]).T
    np.savetxt("keystroke_attack_distribution_difference.csv", data, delimiter=';', header="index;bins;hist")

    repr,_ = min(errors_dict.items(),key = lambda x : x[1])
    save_time_dist(load_delta_file(repr),gt_list,'keystroke_attack_result_time.csv')
    

if __name__ == '__main__':
    main()
