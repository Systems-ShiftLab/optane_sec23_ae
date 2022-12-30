#!/usr/bin/env bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_dir"

if [[ $# -ne 1 ]] ; then 
    echo "Usage: $0 <pmem_file>"
    exit 1
fi

make counter_cleaner


./counter_cleaner  $1
