#!/usr/bin/env python3

import sys

from  gen_ground_truth import load_lats

def gen_stats(observed_list,ground_truth_list):
    JITTER_THRES=20
    results = [x for x in observed_list if x >= 15]
    reference = ground_truth_list

    diffs = []
    i = 0
    i_ref = 0
    false_negatives = 0
    false_positives = 0

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
            false_positives += 1
        else:
            i += 1
            i_ref += 2
            diffs.append(diff_combined_reference)
            false_negatives += 1


    return diffs, false_positives, false_negatives

def main():
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <diffsync_file> <ground_truth_file>')
        return

    observed = load_lats(sys.argv[1])
    ground_truth = load_lats(sys.argv[2])

    print(gen_stats(observed,ground_truth)[1:])


if __name__ == '__main__':
    main()
