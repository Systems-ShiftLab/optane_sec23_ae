#!/usr/bin/env bash
set -e
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

kv_ip=${kv_ip:-10.0.0.1}
kv_port=${kv_port:-1111}
kv_file=${kv_file:-/mnt/pmem0/wearlevel-covert}
rep_count=${rep_count:-3}
score_threshold=${score_threshold:-1}

start_wl() {
    local util=$1
    ../common/remote_commands/start_wl_sim.sh /mnt/pmem0/wlsim  $util
}

end_wl() {
    ../common/remote_commands/kill_wl_sim.sh
}

start_remote_server() {
    kv_port=$kv_port remote_ip=$kv_ip kv_backing_file=$kv_file ../common/remote_commands/start_kv_server.sh
}
stop_remote_server() {
    remote_ip=$kv_ip ../common/remote_commands/kill_kv_server.sh
}

exit_fn() {
    stop_remote_server
    end_wl
}
trap exit_fn EXIT

single_run() {
    local result=$1
    echo Saving to $result
    mkdir -p tmp
    tmp_r=tmp/$(date +%s).csv
    

    make receiver sender

    stop_remote_server
    start_remote_server

    echo "Using threshold: $score_threshold"

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
        echo $accuracy,$time,$packets | tee -a $tmp_r
    done

    rm sender_temp_log_* recvr_temp_log_*
    ./gen_final.py $tmp_r $result
}

result_dir=${result_dir:-result-ae}
mkdir -p $result_dir
echo "exp_count = $rep_count"  | tee $result_dir/remote-covert-config.dat


end_wl
single_run  $result_dir/remote-covert-result.dat

#start_wl 0.1
#single_run  $result_dir/remote-covert-lo.dat
#end_wl
#
#start_wl 0.4
#single_run  $result_dir/remote-covert-med.dat
#end_wl
#
#start_wl 0.7
#single_run  $result_dir/remote-covert-hi.dat
#end_wl
