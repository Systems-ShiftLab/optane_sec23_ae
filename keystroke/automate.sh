#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

exp_rep=${exp_rep:-3}

target_backing_file=${target_backing_file:-/mnt/pmem0/ws-server-keystroke}
prober_file=${prober_file:-/mnt/pmem0/rmw-prober-keystroke}
ws_port=${ws_port:-4321}

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
prober_trace=rmw-trace-$(date +%s)


exit_function() {
    echo "Exiting automate script"
    remote_cmd $prober_kill
    exit
}
trap exit_function SIGINT

[[ -d keystrokes/keystroke100 ]] || ./download-dataset.sh


single_run() {
    local result=$1
    #Kill existing servers and probers
    remote_cmd $prober_kill
    remote_cmd $server_kill

    echo Starting prober
    remote_cmd $prober_cmd $target_backing_file $prober_trace
    sleep 10

    echo Starting server
    remote_cmd $server_cmd $target_backing_file $ws_port

    echo Starting client
    python3 ./client/client.py --websocket_url "ws://$remote_ip:$ws_port" --keystroke_dir keystrokes/keystroke100/ || exit_function

    remote_cmd "$prober_kill"
    remote_cmd "$server_kill"
    echo "Copying results"
    scp $remote_user@$remote_ip:$prober_dir/$prober_trace $result
}

multi_run() {
    trace_dir=$1
    rm -rf $trace_dir
    mkdir $trace_dir

    for ((i=0;i<exp_rep;i=i+1)) ; do 
        single_run $trace_dir/$i.txt
    done
    ./postprocessing/gen_ground_truth.py keystrokes/keystroke100 $trace_dir/ground_truth 
    ./postprocessing/gen_delta.py $trace_dir 10
    ./postprocessing/gen_final_stats.py $trace_dir $trace_dir/ground_truth lenient
    mv keystroke_attack_result_time.csv keystroke_attack_distribution_difference.csv $trace_dir
}

multi_run results-ae
