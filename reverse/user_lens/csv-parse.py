#!/usr/bin/python3
import csv
from sys import argv

with open(argv[1]) as  ifile:
    res = list(csv.DictReader(ifile))
    print ("region_size,average_latency")
    for row in res:
        latency = int(row[argv[2]]) - int (row[argv[3]])
        latency /= int(row["count"])
        region_size = int(row["region_size"])
        block_size = int(row["block_size"])
        avg = int((latency / region_size) * block_size)
        print (f"{region_size},{avg}")
