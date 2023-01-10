#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make ait rmw

res_dir=result-ae
mkdir -p $res_dir
#ait_replacement_random_order.csv  ait_replacement_reverse_order.csv  ait_replacement_same_order.csv


cal_avg () {
    awk '{s+=$1}END{print (s/NR)}' $1
}

#Runs and creates one csv
run_exp() {
    local max_node=$1
    local rep_cnt=$2
    local resfile=$3
    local exp_bin=$4
    local run_type=$5
    local ait_set=$6

    echo Iteration,Rate > $resfile

    for ((j=1;j<=max_node;j=j+1)) ; do
        echo -ne '' > tmp
        for ((i=0;i<rep_cnt;i=i+1)) ; do
           ./$exp_bin $j $ait_set $run_type >> tmp 
        done 
        evicts=$(cal_avg tmp)
        rate=$( python3 <<< "print($evicts / $j)")
        echo $j,$rate | tee -a  $resfile
    done
}

run_exp 38 4 $res_dir/ait_replacement_same_order.csv ait 0 0
run_exp 38 4 $res_dir/ait_replacement_reverse_order.csv ait 1 0
run_exp 38 4 $res_dir/ait_replacement_random_order.csv ait 2 0

run_exp 128 1 $res_dir/rmw_replacement_same_order.csv rmw 0
run_exp 128 1 $res_dir/rmw_replacement_reverse_order.csv rmw 1
run_exp 128 1 $res_dir/rmw_replacement_random_order.csv rmw 2
