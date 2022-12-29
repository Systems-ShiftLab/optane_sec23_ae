#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"


wl_simcommand=${wl_simcommand:-optane_sec23_ae/common/workload_simulator/start_bg.sh}

if [[ $# -ne 2 ]] ;  then
    echo "Usage: $0 <file> <util>"
    exit
fi

wl_file=$1
wl_util=$2

echo "Starting wl simulator remotely"
echo "wl_file: $wl_file"
echo "wl_util: $wl_util"
./generic.sh $wl_simcommand $wl_file $wl_util
