#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

start_wl() {
    local util=$1
    ../common/remote_commands/start_wl_sim.sh /mnt/pmem0/wlsim  $util
}

end_wl() {
    ../common/remote_commands/kill_wl_sim.sh
}

result_dir=${result_dir:-result-ae}
mkdir -p $result_dir


end_wl
result=$result_dir/no-wl.csv kv_file=/mnt/pmem0/wlcovert ./single_run.sh

start_wl 0.1
result=$result_dir/lo-wl.csv kv_file=/mnt/pmem0/wlcovert ./single_run.sh
end_wl

start_wl 0.4
result=$result_dir/med-wl.csv kv_file=/mnt/pmem0/wlcovert ./single_run.sh
end_wl

start_wl 0.7
result=$result_dir/hi-wl.csv kv_file=/mnt/pmem0/wlcovert ./single_run.sh
end_wl
