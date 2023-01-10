#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

rep_count=${rep_count:-100}
msg_size=${msg_size:-1000}
exps=${exps:-'cont ait rmw'}
res_dir=${res_dir:-results}

rm -rf $res_dir
mkdir -p $res_dir

make receiver_epoch sender_epoch

tsc_mhz=$(dmesg | grep "tsc: Detected" | grep -o  '[0-9]*.[0-9]* MHz' | awk '{print $1}')
tsc_mhz=${tsc_mhz:-2100}

tsc=$(echo "$tsc_mhz * (10 ^ 6)" | bc -l)

for exp in $exps ; do
    for ((i=0;i<rep_count;i++)) ; do
        echo "Running exp: $exp, Iteration: $i"
        echo "Running receiver"
        ./receiver_epoch $exp $msg_size > recvr_temp_log &
        sleep 3 #Measured init time
        echo "Running sender"
        ./sender_epoch $exp $msg_size > sender_temp_log
        wait
        rcvr_epoch=$(grep "Target epoch" recvr_temp_log)
        snder_epoch=$(grep "Target epoch" sender_temp_log)
        [[ $rcvr_epoch == $snder_epoch ]] || { echo Sync mistake ;  continue ; }
        correct=$(tail -n1 recvr_temp_log)
        cycles=$(tail -n1 sender_temp_log)

        bw=$(echo "($tsc * $msg_size) / ($cycles) " | bc -l )
        accuracy=$(echo "($correct) / $msg_size" | bc -l )
        echo $accuracy,$bw | tee -a $res_dir/$exp
    done
done

rm sender_temp_log recvr_temp_log
./gen_final.py $res_dir/ait $res_dir/rmw $res_dir/cont $res_dir/covert_results.dat
