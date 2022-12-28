#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

wl_simkill=${wl_simkill:-optane_sec23_ae/common/workload_simulator/end_bg.sh}

echo "Killing all wl_sims"
ssh $remote_user@$remote_ip "$wl_simkill" || echo "Failure"