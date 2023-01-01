#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

if [[ "$#" -ne 2 ]]; then
    echo "Usage: $0 <pmem_file> <trace_output>"
    exit 1
fi

file=$1
trace_file=$2

make rmw_prober


(
    nohup ./rmw_prober $file > $trace_file & 
    echo $! > .server_pid  #SUBSHELL!!
) > /dev/null 2>&1

server_pid=$(cat .server_pid)
rm .server_pid

sleep 0.2

[[ -e /proc/$server_pid ]] || { 
    echo Error
    exit 1
}

echo "Starting rmw_prober on file $file, trace file: $trace_file"
