#!/usr/bin/env python3

from gen_ground_truth import *

def sync_start(observed_list,ground_truth_list,sync_criterion):
    MAX_JITTER_MS=10
    def check_bnd(exp_val,truth):
        diff = abs(exp_val - truth)
        return diff <= MAX_JITTER_MS

    def strict_crit():
        i0 = observed_list[0]
        i1 = observed_list[1]
        i2 = observed_list[2]
        t0 = ground_truth_list[0]
        t1 = ground_truth_list[1]
        t2 = ground_truth_list[2]

        conds = []
        conds.append((check_bnd(i0,t0) and check_bnd(i1,t1)))
        conds.append((check_bnd(i0,t0+t1) and check_bnd(i1,t2)))
        conds.append((check_bnd(i0,t0) and check_bnd(i1,t1+t2)))
        return any(conds)

    def lenient_crit():
        return check_bnd(observed_list[0],ground_truth_list[0]) or check_bnd(observed_list[1],ground_truth_list[1]) or check_bnd(observed_list[2],ground_truth_list[2]) 

    crit = strict_crit
    if sync_criterion == 'lenient':
        crit = lenient_crit

    #Cleaning out a few initial spikes
    while(observed_list[0]) < 10:
        del(observed_list[0])

    TRY_REMOVE=10
    for _ in range(TRY_REMOVE):
        if crit():
            return True
        del(observed_list[0])
    return False


def main():
    if len(sys.argv) != 5:
        print(f'Usage: {sys.argv[0]} <raw_diff> <ground_truth_file> <output_file> <sync_criterion -- strict/lenient>')
        return

    input_file = sys.argv[1]
    ground_truth_file = sys.argv[2]
    output_file = sys.argv[3]
    sync_criterion = sys.argv[4]
    gt_list = load_lats(ground_truth_file)
    delta_list = load_lats(input_file)
    if not sync_start(delta_list,gt_list,sync_criterion):
        print("unsync file",input_file)
        exit(1)
    save_lats(delta_list,output_file)

if __name__ == '__main__':
    main()
