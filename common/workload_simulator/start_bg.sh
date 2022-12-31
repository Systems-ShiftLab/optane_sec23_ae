#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make workload_simulator  

file=$1 
util=$2 


(
    nohup ./workload_simulator $file $util > /dev/null 2>&1  & 
    echo $! > .pid
) > /dev/null 2>&1


sleep 0.2
sim_pid=$(cat .pid)
echo $sim_pid

[[ -e /proc/$sim_pid ]] || { 
    echo "Error"
    exit 1
}

echo "Started wl_simulator with utilization $util"
