#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

make all

result_dir=result-ae

mkdir -p $result_dir

./counter_gran 17 5000 10 > $result_dir/wearlevel_counter.csv
./wear_gran 33 5000 10 > $result_dir/wearlevel_gran.csv
./overwrite 500000 > $result_dir/wearlevel_write.csv

./histogram 120 0 > tmp1
./histogram 120 1 > tmp2
tail -n 100 tmp1 | awk '{print $1}' > $result_dir/wearlevel_write_gap_dist.csv
tail -n 100 tmp1 | awk '{print $2}' > $result_dir/wearlevel_write_lat_dist.csv
tail -n 100 tmp2 | awk '{print $1}' > $result_dir/wearlevel_writeread_gap_dist.csv

rm tmp*
