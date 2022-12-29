#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

kv_bgcommand=${kv_bgcommand:-optane_sec23_ae/common/pmemkv_server/start_bg.sh}
kv_port=${kv_port:-1234}
kv_backing_file=${kv_backing_file:-/mnt/pmem0/kv_server}

echo "Using backing file: $kv_backing_file"
echo "Using port : $kv_port"
./generic.sh $kv_bgcommand $kv_backing_file $kv_port
