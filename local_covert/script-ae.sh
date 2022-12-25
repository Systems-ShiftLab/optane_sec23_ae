#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

rep_count=${rep_count:-110}
exps=${exps:-'cont ait rmw'}
res_dir=${res_dir:-result-ae}

mkdir -p $res_dir

make receiver_epoch sender_epoch

for exp in $exps ; do
    echo accuracy,cycles >  $res_dir/$exp
    for ((i=0;i<rep_count;i++)) ; do
        echo "Running exp: $exp, Iteration: $i"
        echo "Running receiver"
        ./receiver_epoch $exp 1000 > recvr_temp_log &
        sleep 3 #Measured init time
        echo "Running sender"
        ./sender_epoch $exp 1000 > sender_temp_log
        wait
        rcvr_epoch=$(grep "Target epoch" recvr_temp_log)
        snder_epoch=$(grep "Target epoch" sender_temp_log)
        [[ $rcvr_epoch == $snder_epoch ]] || { echo Sync mistake ;  continue ; }
        accuracy=$(tail -n1 recvr_temp_log)
        time=$(tail -n1 sender_temp_log)
        echo $accuracy,$time | tee -a $res_dir/$exp
    done
done

rm sender_temp_log recvr_temp_log
