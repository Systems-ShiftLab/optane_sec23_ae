#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make all

result_dir=result-ae

mkdir -p $result_dir

./flush_read 100 > $result_dir/read_w_clflush.csv
./no_flush_read 100 > $result_dir/read_wo_clflush.csv
