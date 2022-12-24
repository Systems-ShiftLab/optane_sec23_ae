#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make all

result_dir=result-ae

mkdir -p $result_dir

./counter_gran 17 5000 10 > $result_dir/wearlevel_counter.csv

./wear_gran 33 5000 10 > $result_dir/wearlevel_gran.csv
