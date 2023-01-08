#!/usr/bin/env python3

def main():
    '''
    Usage: ./calc_accuracy recvr_log sender_log score_param
    '''
    from sys import argv
    import re

    rcvr_log_file = argv[1]
    sndr_log_file = argv[2]
    score_param = int(argv[3])

    rcvr_input = {}
    sndr_input = {}

    for line in open(rcvr_log_file):
        if not '-' in line: continue
        tsp,bit_score = [int(x) for x in line.split('-')]
        #if bit_score>100: bit_score=50
        rcvr_input[tsp] = bit_score

    for line in open(sndr_log_file):
        if not '-' in line: continue
        tsp,bit_val = [int(x) for x in line.split('-')]
        sndr_input[tsp] = bit_val


    msg_len = 100
    common_tsps = list(set(rcvr_input).intersection(set(sndr_input)))[:msg_len]
    


    mis_guesses = 0
    for tsp in common_tsps:
        score = rcvr_input[tsp]
        bit = sndr_input[tsp]
        guessed_bit = 1
        if score <= score_param : guessed_bit = 0
        if not guessed_bit == bit:
            mis_guesses+=1

    error_rate = mis_guesses/(len(common_tsps))
    acc = 1 - error_rate
    print(acc)

if __name__=='__main__':
    main()
