#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

progress_file=${progress_file:-.overall-progress}

export remote_ip=${remote_ip:-10.0.0.1}
export remote_user=${remote_user:-suraajks}


local_hash=$(git rev-parse HEAD)
remote_cmd_res=$(common/remote_commands/generic.sh "cd optane_sec23_ae; git rev-parse HEAD")
remote_hash=$(echo "$remote_cmd_res" | tail -n1)

if [[ $local_hash != $remote_hash ]] ; then
    echo "Local Commit:$local_hash"
    echo "Remote Commit: $remote_hash"
    echo "Commits are different"
    echo "Please ensure that the commit versions are the same"
    exit
fi

truncate $progress_file -s0
for i in $(find -name "script-ae.sh") ; do
    echo Running $i | tee -a $progress_file
    (
        cd $(dirname $i)
        ./$(basename $i)
    )
done

mkdir -p report/results_csv
mkdir -p report/results_dat

cp $(find -name '*.csv' | grep result-ae ) report/results_csv
cp $(find -name '*.dat' | grep result-ae ) report/results_dat

echo Generating Report
cd report
make
