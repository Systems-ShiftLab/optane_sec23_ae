#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

if [[ $# -eq 0 ]] ;  then
    echo "Usage: $0 <command to be run remotely>"
    exit
fi
source common.sh

echo "Starting running command on $remote_ip as $remote_user"
echo ssh $remote_user@$remote_ip $* 
ssh $remote_user@$remote_ip $* || echo "Failed to run command"
