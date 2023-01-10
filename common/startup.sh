#!/usr/bin/env bash
set -e

if [ "$EUID" -ne 0 ] ; then 
    echo "Please run as root"
    exit
fi

#Check if mounted already!!
for pmemdev in /dev/pmem* ; do
    mtpt=/mnt/${pmemdev##*/}
    mkdir -p "$mtpt"
    mount -o dax "$pmemdev" "$mtpt"
    ramdisk=$mtpt
    done

ln=/mnt/ramdisk
rm $ln
ln -s "$ramdisk" $ln
