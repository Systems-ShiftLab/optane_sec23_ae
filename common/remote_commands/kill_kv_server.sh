#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"


source common.sh

kv_killcommand=${kv_killcommand:-optane_sec23_ae/common/pmemkv_server/end_bg.sh}

echo "Killing servers"
./generic.sh $kv_killcommand
