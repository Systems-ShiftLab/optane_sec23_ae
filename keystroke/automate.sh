#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

target_backing_file=${target_backing_file:-/mnt/pmem0/ws-server-keystroke}
ws_port=${ws_port:-4321}

export remote_ip=${remote_ip:-10.0.0.1}
export remote_user=${remote_user:-suraajks}
remote_cmd () {
    ../common/remote_commands/generic.sh $*
}

server_cmd=${prober_cmd:-optane_sec23_se/keystroke/server/start_bg.sh}
server_kill=${prober_cmd:-optane_sec23_se/keystroke/server/end_bg.sh}

prober_cmd=${prober_cmd:-optane_sec23_se/keystroke/prober/rmw_prober.sh}
prober_kill=${prober_kill:-killall rmw_prober}
prober_dir=$(dirname $prober_cmd)
prober_trace=$prober_dir/rmw_trace


exit_function() {
    echo "Exiting automate script"
    remote_cmd $prober_kill
    exit
}
trap exit_function SIGINT

[[ -d keystrokes/keystroke100 ]] || ./download_dataset.sh

remote_cmd "$prober_kill; $prober_cmd $target_backing_file 2>errlog > $prober_trace &"
remote_cmd "$server_kill; $server_cmd $target_backing_file $ws_port"

sleep 10
echo "Starting client"
python3 ./client/client.py --websocket_url "ws://$remote_ip:$ws_port" --keystroke_dir keystrokes/keystroke100/ || exit_function
remote_cmd "$prober_kill"
remote_cmd "$server_kill"
echo "Copying results"
scp $remote_user@$remote_ip $prober_trace .
