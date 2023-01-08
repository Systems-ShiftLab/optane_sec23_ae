#!/usr/bin/env python3

import sys
import re

def main():
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <csv_file> <output_name>' )
        return

    csvfile = sys.argv[1]
    outfile = sys.argv[2]

    accs = []
    bws = []
    packets = []

    with open(csvfile) as fd:
        for line in fd:
            try:
                acc,time,pkt = [float(x) for x in line.strip().split(',')]
            except ValueError:
                continue
            acc = acc * 100 #Percent
            accs.append(acc)
            bws.append(100/time) #Message size is 100 
            packets.append(pkt/100) 

    from statistics import mean,stdev
    acc_mean = mean(accs)
    acc_stdev = stdev(accs)
    bws_mean = mean(bws)
    bws_stdev = stdev(bws)
    packets_mean = mean(packets)
    packets_stdev = stdev(packets)

    ofd = open(outfile,'w')

    ofd.write(f'acc_mean = {acc_mean:0.2f}\n')
    ofd.write(f'acc_stdev = {acc_stdev:0.2f}\n')
    ofd.write(f'bws_mean = {bws_mean:0.2f}\n')
    ofd.write(f'bws_stdev = {bws_stdev:0.2f}\n')
    ofd.write(f'packets_per_bit_mean = {packets_mean:0.2f}\n')
    ofd.write(f'packets_per_bit_stdev = {packets_stdev:0.2f}\n')

    ofd.close()


if __name__ == '__main__':
    main()
