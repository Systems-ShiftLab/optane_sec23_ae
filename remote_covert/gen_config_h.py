#!/usr/bin/env python3
# Generates a header file

#Change these as required -- This is the number of data bits
SRVER_MSG_SIZE = 512 
COVERT_MSG_SIZE = 110
MSG_BIT_INTERVAL = 90000
KV_KEY = 831 #Random number
SNDR_THREAD_CNT = 4
WEAR_LVL_LAT_LO = 95
WEAR_LVL_LAT_HI = 130

def get_string(bits):
    import random
    num = random.getrandbits(bits)
    s = format(num, 'b').zfill(bits)
    return  s


def main():
    random_message=get_string(COVERT_MSG_SIZE)
    sender2server_msg = f"put-{KV_KEY}-{'a'*SRVER_MSG_SIZE}\\n"
    ofile_name = 'config.h'
    ofile = open(ofile_name,'w')

    ofile.write("#pragma once\n\n")
    ofile.write(f"#define MSG_BIT_INTERVAL {MSG_BIT_INTERVAL}\n")
    ofile.write(f"#define COVERT_MSG_SIZE {COVERT_MSG_SIZE}\n")
    ofile.write(f"#define SNDR_THREAD_CNT {SNDR_THREAD_CNT}\n\n")
    ofile.write(f"#define WEAR_LVL_LAT_LO {WEAR_LVL_LAT_LO}\n\n")
    ofile.write(f"#define WEAR_LVL_LAT_HI {WEAR_LVL_LAT_HI}\n\n")
    ofile.write(f"char sender2server_msg[] =  \"{sender2server_msg}\";\n")
    ofile.write(f'char global_msg[] = "{random_message}";\n')

    ofile.close()

if __name__ == '__main__':
    main()
