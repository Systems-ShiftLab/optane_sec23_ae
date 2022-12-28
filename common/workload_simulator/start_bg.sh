#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make workload_simulator  

file=$1 
util=$2 

log_file=log
errlog_file=errlog

(
    nohup ./workload_simulator $file $util > $log_file 2> $errlog_file & 
    sim_pid=$!
) > /dev/null 2>&1

sleep 0.2

[[ -e /proc/$sim_pid ]] || { 
    echo Connection error
    exit 1
}

echo "Started wl_simulator with utilization $util"
echo "Saving stdout to $log_file"
echo "Saving stderr to $errlog_file"
