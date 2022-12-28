#!/usr/bin/env python3

def partition_list(l):
    def weight(l1,l2):
        def my_measure(l):
            from statistics import variance
            if len(l)<=1: return 0
            return variance(l)

        return my_measure(l1) + my_measure(l2)

    l = sorted(l)

    ans = (1000000,[],[])

    for i in range(len(l)):
        l1 = l[:i]
        l2 = l[i:]
        score = weight(l1,l2)
        if score < ans[0]: ans = (score,l1,l2)

    return (set(ans[1]),set(ans[2]))

def main():
    import fileinput
    import re
    parsed_list = []
    for line in fileinput.input():
        if  line.startswith("Connected"): continue
        parsed_list.append(tuple(int(x) for x in re.findall(r'[-]?\d+',line)))
    set_0,set_1 = partition_list([x[2] for x in parsed_list])

    corr_guesses = 0
    for bit,correct,score in parsed_list:
        if score in set_0:
            guess=0
        else:
            assert score in set_1
            guess=1
        GREEN = '\033[92m'
        RED = '\033[91m'
        ENDC = '\033[0m'
        color = RED
        if guess==correct:
            corr_guesses+=1
            color=GREEN
        print(f"{color}{bit} - {guess}{ENDC}")
    print(corr_guesses)





if __name__=='__main__':
    main()
