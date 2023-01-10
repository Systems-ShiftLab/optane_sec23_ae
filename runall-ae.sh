#!/usr/bin/env bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

progress_file=${progress_file:-.overall-progress}

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
