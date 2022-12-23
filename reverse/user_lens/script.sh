#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

./pointer-chasing read_and_write | tee tmp1

./pc-parse.py tmp1 tmp2

mkdir -p result
./csv-parse.py tmp2 cycle_read_end  cycle_read_start > result/gen_hier.csv

rm tmp*

