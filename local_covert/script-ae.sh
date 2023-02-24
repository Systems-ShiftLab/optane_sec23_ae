#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

export remote_ip=${remote_ip:-10.0.0.1}
export remote_user=${remote_user:-usenix}

../common/remote_commands/generic.sh optane_sec23_ae/local_covert/local-run.sh

mkdir -p result-ae
scp $remote_user@$remote_ip:optane_sec23_ae/local_covert/results/covert_results.dat result-ae
