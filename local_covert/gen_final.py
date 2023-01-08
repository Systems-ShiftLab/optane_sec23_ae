#!/usr/bin/env python3

import sys
import re

def main():
    if len(sys.argv) != 5:
        print(f'Usage: {sys.argv[0]} <ait_stats> <rmw_stats> <cont_stats> <output_name>' )
        return

    aitfile = sys.argv[1]
    rmwfile = sys.argv[2]
    contfile = sys.argv[3]
    outfile = sys.argv[4]

    def gen_stats(ifile):
        from statistics import mean,stdev
        with open(ifile) as fd:
            acclst = []
            bwlst = []
            for line in fd:
                try:
                    acc,bw = [float(x) for x in line.strip().split(',')]
                except ValueError:
                    continue
                acc = acc * 100 #Percent
                bw = bw / 1000 #kbps
                acclst.append(acc)
                bwlst.append(bw)
            acc_mean = mean(acclst)
            acc_stdev = stdev(acclst)
            bw_mean = mean(bwlst)
            bw_stdev = stdev(bwlst)
            return acc_mean,acc_stdev,bw_mean,bw_stdev


    ofd = open(outfile,'w')

    def helper(nametag,filename):
        tmp = gen_stats(filename)
        ofd.write(f'{nametag}_acc_mean = {tmp[0]:0.2f}\n')
        ofd.write(f'{nametag}_acc_stdev = {tmp[1]:0.2f}\n')
        ofd.write(f'{nametag}_kbps_mean = {tmp[2]:0.2f}\n')
        ofd.write(f'{nametag}_kbps_stdev = {tmp[3]:0.2f}\n')

    helper("ait",aitfile)
    helper("rmw",rmwfile)
    helper("cont",contfile)
        
    ofd.close()


if __name__ == '__main__':
    main()
