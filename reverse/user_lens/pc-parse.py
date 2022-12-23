#!/usr/bin/python
import re
import csv
from sys import argv

def parse_result(src_file):
    with open(src_file, 'r') as f:
        content = f.read()
    stat_line=r'\n\[.*'
    stats = re.findall(stat_line, content)

    res = []
    for stat in stats:
        stat = stat.strip()
        stat_entries = stat.split(',')
        res_entry = {}
        for se in stat_entries:
            for column in se.split(','):
                c = column.lstrip().rstrip().rstrip('.').split(' ')
                if c[0].startswith('['):
                    # The first column, for e.g. '[ 9871.210131] {0}[pointer-chasing-4096] region_size 67108864'
                    res_entry['task_name'] = c[-3].split('[')[1].rstrip(']')
                    res_entry[c[-2]] = int(c[-1])
                elif c[0] == 'cycle':
                    res_entry['cycle_write_start'] = int(c[1])
                    res_entry['cycle_read_start'] = int(c[3])
                    res_entry['cycle_read_end'] = int(c[5])
                else:
                    res_entry[c[0]] = c[1]
        res.append(res_entry)
    return res



def pointer_chasing(src_file, out_file):
    '''Parse the pointer_chasing test result.'''
    data = parse_result(src_file)
    with open(out_file, 'w') as f:
        writer = csv.DictWriter(f, fieldnames=data[0].keys())
        writer.writeheader()
        writer.writerows(data)

def main():
    pointer_chasing(argv[1],argv[2])

if __name__ == "__main__":
    main() 
