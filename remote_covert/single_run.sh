#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

kv_ip=${kv_ip:-10.0.0.1}
kv_port=${kv_port:-1111}
kv_file=${kv_file:-/mnt/pmem0/wearlevel-covert}
rep_count=${rep_count:-30}
score_threshold=${score_threshold:-1}

result=${result:-result}
echo Saving to $result

start_remote_server() {
    kv_port=$kv_port remote_ip=$kv_ip kv_backing_file=$kv_file ../common/remote_commands/start_kv_server.sh
}
stop_remote_server() {
    remote_ip=$kv_ip ../common/remote_commands/kill_kv_server.sh
}

make receiver sender

stop_remote_server
start_remote_server
trap stop_remote_server EXIT

echo "Using threshold: $score_threshold"

echo 'error_rate,time(s),packets' > $result
for ((i=0;i<rep_count;i++)) ; do
    echo Sleep start
    sleep 1
    echo Sleep end
    echo "Iteration: $i"
    echo "Running receiver"
    taskset -c 10 ./receiver $kv_ip $kv_port > recvr_temp_log_$i &
    echo "Running sender"
    time_start=$(date +%s.%N)
    taskset -c 16-23 ./sender $kv_ip $kv_port > sender_temp_log_$i
    time_end=$(date +%s.%N)
    time=$(echo "$time_end - $time_start" | bc)
    wait
    accuracy=$(./calc_accuracy.py recvr_temp_log_$i sender_temp_log_$i $score_threshold)
    packets=$(tail -n1 sender_temp_log_$i)
    echo $accuracy,$time,$packets | tee -a $result
done

rm sender_temp_log_* recvr_temp_log_*
