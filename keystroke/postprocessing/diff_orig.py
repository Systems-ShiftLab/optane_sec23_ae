#!/usr/bin/python3

from locale import normalize
import numpy as np
import os
import pandas as pd
import sys
import re
import matplotlib.pyplot as plt
import seaborn as snb

freq = 2.1 * 1e9
thresh = 20

#https://gist.github.com/SeanSyue/8c8ff717681e9ecffc8e43a686e68fd9
def sorted_alphanumeric(data):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)] 
    return sorted(data, key=alphanum_key)


# load the reference
directories = sorted_alphanumeric(os.listdir(sys.argv[2]))
reference = []
for dir_name in directories:
    reference_user = np.loadtxt(sys.argv[2] + "/"+ dir_name + "/latency.txt")
    for row in reference_user:
        for lat in row:
            ground_truth_val = lat
            reference.append(ground_truth_val)


print(f"data set contains: {len(reference)} keystroke differences")

def postprocess(file):
    try:
        results = (np.loadtxt(file,delimiter=";")[:,0] / freq) * 1000
    except:
        results = (np.loadtxt(file,delimiter=";",skiprows=1)[:,0] / freq) * 1000

    #remove "fake" keystrokes (we send 2 times durign the end see source)
    results = results[results >= 15.0]

    diffs = []
    i = 0
    i_ref = 0
    errors = 0
    while i < len(results)-1 and i_ref < len(reference):
        res = results[i]
        ref = reference[i_ref]
        d = abs(res - ref)
        
        if d < thresh:
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

    return results, diffs, errors


def print_result(results, diffs):
    N = len(diffs)

    fig, axis = plt.subplots(3,1)

    axis[0].plot(results[:N],label="RMW")
    axis[0].plot(reference[:N],label="Reference")
    axis[0].legend()
    axis[0].grid(True)
    axis[0].set_ylabel("Keystroke Timing in ms")
    axis[0].set_xlabel("Keystroke Index")
    axis[0].set_title("Keystroke Attack")

    axis[1].plot(diffs,label="Difference")
    axis[1].legend()
    axis[1].grid(True)
    axis[1].set_ylabel("Difference in ms")
    axis[1].set_xlabel("Keystroke Index")

    snb.distplot(diffs,ax=axis[2],label="Distribution")

    #axis[2].plot(bins[:-1],hist,label="Distribution")
    axis[2].legend()
    axis[2].grid(True)
    axis[2].set_xlabel("Difference in ms")
    axis[2].set_ylabel("Count")

    data = np.array([np.arange(N), results[:N], reference[:N]]).T
    print(data.shape)
    np.savetxt("result_time.csv", data, delimiter=';', header="index;result;reference")



errors_overall = 0
keystrokes_overall = 0

all_diffs = []

for i, result_file in enumerate(os.listdir(sys.argv[1])):
    result, diff, errors = postprocess(sys.argv[1] + "/" + result_file)

    errors_overall += errors
    keystrokes_overall += len(reference)

    all_diffs.extend(diff)

    if i == 0:
        print_result(result, diff)
        


    print(f"{i:3d} error rate: {errors_overall/keystrokes_overall*100:4.2f}%\r")

print("")

all_diffs = np.array(all_diffs)

#
all_diffs = all_diffs[all_diffs < thresh]

plt.figure()

plt.subplot(211)

hist, bins = np.histogram(all_diffs,bins=thresh,range=(0,thresh+2))
hist = hist / np.max(hist)
data = np.array([np.arange(thresh), bins[:-1], hist]).T
np.savetxt("keystroke_attack_distribution_difference.csv", data, delimiter=';', header="index;bins;hist")


snb.distplot(all_diffs,label="Distribution Measurement Difference")

#plt.stem(bins[:-1], hist,label="Distribution")

plt.legend()
plt.grid(True)
plt.xlabel("Difference in ms")
plt.ylabel("Count")
plt.title("Distribution over all measurements")


plt.subplot(212)

snb.distplot(reference,label="Distribution All Typers")


hist, bins = np.histogram(reference,bins=100)
hist = hist / np.max(hist)

from scipy.stats import sem


print(f"ref:   Mean={np.mean(reference)} std={np.std(reference)} stderr={sem(reference)}")
print(f"diffs: Mean={np.mean(all_diffs)} std={np.std(all_diffs)} stderr={sem(all_diffs)}")

data = np.array([np.arange(100), bins[:-1], hist]).T
np.savetxt("keystroke_attack_distribution_reference.csv", data, delimiter=';', header="index;bins;hist")

#snb.displot(all_diffs,label="Distribution",kde=False)

#plt.stem(bins[:-1], hist,label="Distribution")

plt.legend()
plt.grid(True)
plt.xlabel("Difference in ms")
plt.ylabel("Count")
plt.title("Distribution of all the typers")

plt.show()