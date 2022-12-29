#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

source common.sh

wl_simcommand=${wl_simcommand:-optane_sec23_ae/common/workload_simulator/start_bg.sh}

wl_file=$1
wl_util=$2

echo "Starting wl simulator remotely"
ssh $remote_user@$remote_ip "$wl_simcommand" "$wl_file" "$wl_util" || echo "Failure"
