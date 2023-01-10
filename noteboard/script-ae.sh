#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

exp_cnt=${exp_cnt:-10}
resdir=${resdir:-result-ae}

sender_msg=$(cat msg.txt)
sender_msg_size=$(( $(wc -c < msg.txt) - 1 ))
redundancy=${redundancy:-10}

#These are experimental values
postprocess_clm=${postprocess_clm:-2}
#postprocess_ths=${postprocess_ths:-4} #Use this if redundancy 4
postprocess_ths=${postprocess_ths:-6} #Use this if redundancy 10

dimm_mnt=${dimm_mnt:-/mnt/pmem1}

kv_port=${kv_port:-8080}
remote_ip=${remote_ip:-10.0.0.1}
remote_user=${remote_user:-suraajks}

export kv_port
export remote_ip
export remote_user

clean_mnt() {
    ../common/remote_commands/generic.sh find $dimm_mnt -type f -delete
}

start_server() {
    local remote_noteboard_file=$1
    kv_backing_file=$remote_noteboard_file ../common/remote_commands/start_kv_server.sh
}

kill_servers() {
    ../common/remote_commands/kill_kv_server.sh
}

send_noteboard() {
    local remote_noteboard_file=$1

    kill_servers
    start_server $remote_noteboard_file

    echo "Setting up all locations"
    make setup && ./setup $remote_ip $kv_port $((redundancy * sender_msg_size))

    echo "Ensuring noteboard is fresh"
    clean_command=optane_sec23_ae/noteboard/wl_counter_cleaner/counter_clean.sh
    ../common/remote_commands/generic.sh $clean_command $remote_noteboard_file 

    echo "Sending message"
    make sender && ./sender $remote_ip $kv_port $sender_msg $redundancy

    kill_servers
}

receive_noteboard() {
    local remote_noteboard_file=$1
    local result_file=$2

    kill_servers
    start_server $remote_noteboard_file

    echo "Receiving message"
    make receiver && stdbuf --output=0 ./receiver $remote_ip $kv_port $sender_msg_size $redundancy | tee $result_file

    kill_servers
}


single_run() {
    # Deleting all files on the mount point
    local final_res=$1
    tmp_dir=tmp/$redundancy/traces_$(date +%s)
    mkdir -p $tmp_dir

    for ((i = 0 ; i < exp_cnt ; i = i+1 )); do
        send_noteboard $dimm_mnt/noteboard_$i
        receive_noteboard $dimm_mnt/noteboard_$i $tmp_dir/$i
    done

    ./postprocess.py "$tmp_dir/*" $postprocess_clm $postprocess_ths  $final_res
}

rm -rf result-ae
mkdir -p result-ae

echo "exp_count = $exp_cnt
redundancy = $redundancy
message_size = $sender_msg_size"  | tee result-ae/noteboard-config.dat

../common/remote_commands/kill_wl_sim.sh

##Running hi-noise experiment
#clean_mnt
#../common/remote_commands/start_wl_sim.sh $dimm_mnt/wl-sim 0.9
#single_run result-ae/noteboard-hi-noise.dat
#../common/remote_commands/kill_wl_sim.sh
#
##Running med-noise experiment
#clean_mnt
#../common/remote_commands/start_wl_sim.sh $dimm_mnt/wl-sim 0.5
#single_run result-ae/noteboard-med-noise.dat
#../common/remote_commands/kill_wl_sim.sh
#
##Running lo-noise experiment
#clean_mnt
#../common/remote_commands/start_wl_sim.sh $dimm_mnt/wl-sim 0.1
#single_run result-ae/noteboard-low-noise.dat
#../common/remote_commands/kill_wl_sim.sh

#Running no-noise experiment
clean_mnt
single_run result-ae/noteboard-result.dat
