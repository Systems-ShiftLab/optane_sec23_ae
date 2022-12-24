#! /bin/bash
set -e
print_help() {
    echo "Usage: bitmask_list=?? $0 "
}

#if [ $# -ne 1 ] && [ $# -ne 3 ]; then
#    print_help
#    exit 1
#fi
#
result_dir=result-ae

mkdir -p $result_dir
gcc bit_pc.c aux.c -O2 -lpmem -o bit_pc_bin

count_0=(1 2 4  8 16 32 64 128)
count_1=($(seq -s ' ' $((2 ** 8)) $((2 ** 6)) $((2 ** 10))))
count_2=($(seq -s ' ' $((2 ** 12)) $((2 ** 12)) $((2 ** 16))))
count_3=($(seq -s ' ' $((2 ** 17)) $((2 ** 16)) $((2 ** 19))))

count_array=("${count_0[@]}" "${count_1[@]}" "${count_2[@]}" "${count_3[@]}")
echo Test counts: "${count_array[@]}"

run_cnt() {

    echo "Generating sizes"
    (
    echo size
    for count in ${count_array[@]}; do
        echo  "$((count * 64))"
    done 
    ) | tee $result_dir/size
}
run_exp() {
    local bitmask_inp=$1
    local resfile_name=$2

    echo "Bitmask: $bitmask_inp"
    (
    echo $resfile_name
    for count in ${count_array[@]}; do
        lat=$(./bit_pc_bin $count $bitmask_inp )
        avg=$(( lat / count))
        echo  "$avg"
    done 
    ) | tee $result_dir/$resfile_name
}


hacky_helper1(){
    local inp=$1
    local temp=$((inp-6))
    local mask=$((2 ** temp))

    run_exp $mask mask_$inp
}

hacky_helper2(){
    local small=$1
    local big=$2
    local tmp1=$((big-5))
    local tmp2=$((small-6))
    local mask=$(( (2**tmp1) - (2**tmp2)))

    run_exp $mask mask_$small-$big
}



#Running all experiments for artefact evaluation
run_cnt
run_exp 0 mask_clean
for ((i = 8 ; i<= 21 ; i = i+ 1)); do
    hacky_helper1 $i
done
hacky_helper2 12 13
hacky_helper2 12 14
hacky_helper2 12 19
hacky_helper2 12 20

cd $result_dir
paste size mask_clean mask_12 mask_13 mask_12-13 mask_14 mask_12-14 mask_19 mask_20 mask_21 mask_12-19 mask_12-20 -d ',' > ait_assoc.csv
paste size mask_clean mask_8 mask_9 mask_10 mask_11 mask_12 mask_13 mask_14 mask_15 mask_16 mask_17 mask_18 mask_19 mask_20 mask_21 mask_12-19 -d ',' > rmw_assoc.csv
