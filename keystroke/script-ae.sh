#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

exp_rep=${exp_rep:-1}

target_backing_file=${target_backing_file:-/mnt/pmem0/ws-server-keystroke}
prober_file=${prober_file:-/mnt/pmem0/rmw-prober-keystroke}
ws_port=${ws_port:-4321}

result_dir=${result_dir:-result-ae}


[[ -d keystrokes/keystroke100 ]] || ./download-dataset.sh

# Create result directory, and populate with ground truth
prepare_result() {
    echo "Preparing $result_dir"
    rm -rf $result_dir
    mkdir -p $result_dir
    cp -R keystrokes/keystroke100/* $result_dir #We can change this to control number of typists
    rm $result_dir/*/pressure.txt
    for dir in $result_dir/* ; do
        ./gen_ground_truth.py $dir/latency.txt $dir/ground_truth.txt &
    done
    wait
    rm $result_dir/*/latency.txt
}


export remote_ip=${remote_ip:-10.0.0.1}
export remote_user=${remote_user:-suraajks}
remote_cmd () {
    ../common/remote_commands/generic.sh $*
}

server_cmd=${prober_cmd:-optane_sec23_ae/keystroke/server/start_bg.sh}
server_kill=${prober_cmd:-optane_sec23_ae/keystroke/server/end_bg.sh}

prober_cmd=${prober_cmd:-optane_sec23_ae/keystroke/prober/start_bg.sh}
prober_kill=${prober_kill:-optane_sec23_ae/keystroke/prober/end_bg.sh}
prober_dir=$(dirname $prober_cmd)


exit_function() {
    echo "Exiting automate script"
    remote_cmd $prober_kill
    remote_cmd $server_kill
    exit
}
trap exit_function SIGINT

single_run() {
    local input_dir=$1
    local ground_truth=$1/ground_truth.txt
    local rmw_trace=$1/prober.trace
    local rmw_diff=$1/prober.diff
    local rmw_diff_synced=$1/prober.diffsync

    echo Killing existing servers and probers
    remote_cmd $prober_kill
    remote_cmd $server_kill

    echo Starting prober
    local remote_prober_trace=rmw-trace-$(date +%s)
    remote_cmd $prober_cmd $target_backing_file $remote_prober_trace
    sleep 10

    echo Starting server
    remote_cmd $server_cmd $target_backing_file $ws_port

    echo Starting client
    ./client.py --websocket_url "ws://$remote_ip:$ws_port" --keystroke_file $ground_truth || exit_function

    remote_cmd "$prober_kill"
    remote_cmd "$server_kill"

    echo "Copying results"
    scp $remote_user@$remote_ip:$prober_dir/$remote_prober_trace $rmw_trace
    remote_cmd "rm $prober_dir/$remote_prober_trace"
    ./gen_delta.py $rmw_trace $rmw_diff 10
    ./sync_start.py $rmw_diff $ground_truth $rmw_diff_synced strict || return 1
    return 0
}

prepare_result
for i in $result_dir/* ; do
    #Repeat
    for j in {0..2} ; do 
        echo "Running for directory $i"
        single_run $i && break
    done
done

./gen_final.py $result_dir $result_dir/keystroke_attack_result.dat $result_dir/keystroke_attack_result_time.csv
