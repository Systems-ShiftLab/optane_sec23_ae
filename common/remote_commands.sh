#!/usr/bin/env bash

remote_ip=${remote_ip:-10.0.0.1} 
remote_user=${remote_user:-suraajks}

kv_bgcommand=${kv_bgcommand:-optane_sec23_ae/common/pmemkv_server/start_bg.sh}
kv_killcommand=${kv_killcommand:-optane_sec23_ae/common/pmemkv_server/end_bg.sh}
kv_port=${kv_port:-1234}
kv_backing_file=${kv_backing_file:-/mnt/pmem0/kv_server}

start_server() {
    echo "Using backing file: $kv_backing_file"
    echo "Starting server on $remote_ip:$kv_port"
    ssh $remote_user@$remote_ip "$kv_bgcommand $kv_backing_file $kv_port"  || echo "Cannot create server!"
}

kill_servers() {
    echo "Killing servers"
    ssh $remote_user@$remote_ip "$kv_killcommand" || echo "No existing servers"
}

cmd=$1

case $cmd in
    start)
        start_server
        ;;

    end)
        kill_servers
        ;;

    *)
        echo "Usage: $0 <start/end>"
        ;;
esac
