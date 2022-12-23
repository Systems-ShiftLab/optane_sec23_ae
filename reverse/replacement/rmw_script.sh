#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

res_dir=${res_dir:-result/rmw}
rep_count=${rep_count:-128}
make rmw

echo Running cyclic tests...
tmp=$res_dir/cyclic
mkdir -p $tmp
resfile=$tmp/$(date +%s)
for ((i=1;i<rep_count;i=i+1)) ; do
    echo $i
    ./rmw $i 0 >> $resfile
done

echo Running reverse tests..
tmp=$res_dir/reverse
mkdir -p $tmp
resfile=$tmp/$(date +%s)
for ((i=1;i<rep_count;i=i+1)) ; do
    echo $i
    ./rmw $i 1 >> $resfile
done

echo Running random tests..
tmp=$res_dir/random
mkdir -p $tmp
resfile=$tmp/$(date +%s)
for ((i=1;i<rep_count;i=i+1)) ; do
    echo $i
    ./rmw $i 2 >> $resfile
done
