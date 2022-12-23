#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

res_dir=${res_dir:-result/ait}
rep_count=${rep_count:-40}
make ait

echo Running cyclic tests...
for s in {0..255} ; do
    tmp=$res_dir/cyclic/set_$s
    mkdir -p $tmp
    resfile=$tmp/$(date +%s)
    for ((i=1;i<rep_count;i=i+1)) ; do
        echo -e "$i,$s\r"
        ./ait $i $s 0 >> $resfile
    done
done

echo Running reverse tests...
for s in {0..255} ; do
    tmp=$res_dir/reverse/set_$s
    mkdir -p $tmp
    resfile=$tmp/$(date +%s)
    ./ait $i $s 1 >> $resfile
done

echo Running random tests...
for s in {0..255} ; do
    tmp=$res_dir/random/set_$s
    mkdir -p $tmp
    resfile=$tmp/$(date +%s)
    for ((i=1;i<rep_count;i=i+1)) ; do
        echo -e "$i,$s\r"
        ./ait $i $s 2 >> $resfile
    done
done
