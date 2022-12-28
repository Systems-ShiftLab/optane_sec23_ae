#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

if [[ "$#" -ne 2 ]]; then
    echo "Usage: $0 <pmem_file> <port>"
    exit 1
fi

file=$1
port=$2

make pmemkv_server

log_file=${log_file:-log}
errlog_file=${errlog_file:-errlog}

(
    export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64
    nohup ./pmemkv_server $file $port > $log_file 2> $errlog_file & 
    echo $! > .server_pid 
) > /dev/null 2>&1

server_pid=$(cat .server_pid)
rm .server_pid

sleep 0.2

[[ -e /proc/$server_pid ]] || { 
    echo Connection error
    exit 1
}

echo "Starting server on port $port, with file $file"
echo "Saving stdout to $log_file"
echo "Saving stderr to $errlog_file"
