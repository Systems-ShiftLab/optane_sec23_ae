#!/bin/bash

make reader writer

mkdir -p result-ae

run_exp() {
    local exp_type=$1
    local result_name=$2
    local wr_ratio=$3

    case $exp_type in
    
    rr)
        echo "Running reader in BG"
        ./reader /mnt/ramdisk/reader2 > /dev/zero &
        ;;

    rw)
        echo "Running writer in BG"
        ./writer $wr_ratio /mnt/ramdisk/writer &
        ;;
    esac

    echo "Running reader"
    ./reader /mnt/ramdisk/reader > tmp &
    sleep 10

    tail -n 100 tmp > result-ae/$result_name
    echo "Killing everything"
    killall reader writer
    rm tmp
}

run_exp r rw_cont_read.csv
run_exp rr rw_cont_read_read.csv
run_exp rw rw_cont_read_write_10.csv 0.1
run_exp rw rw_cont_read_write_30.csv 0.3
run_exp rw rw_cont_read_write_50.csv 0.5
run_exp rw rw_cont_read_write_100.csv 1
