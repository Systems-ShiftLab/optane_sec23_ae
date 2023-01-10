#include<time.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#include "timing.h"
#include "experiment.h"
#include "mem.h"
#include "util.h"
#include "wear_util.h"


int main(int argc, char* argv[])
{

	if(argc != 4)
	{
		fprintf(stderr,"Usage: %s <num_locs - e.g. 33> <init_num_write - e.g. 5000> <exp_reps - e.g. 10>",argv[0]);
		exit(0);
	}

    int num_locs = atoi(argv[1]);
    int init_num_write = atoi(argv[2]);
    int reps_per_loc = atoi(argv[3]);
	pmem_exp_init("/mnt/ramdisk/pmem_overwr",REGION_SIZE,true);


    printf("offset,writes_to_wl\n");
	for(int i=0;i<num_locs;i++)
	{
        int offset = i * RMWL_SIZE;
        int sum = 0;
        for(int j=0;j<reps_per_loc;j++)
        {
            for (int x=0;x<init_num_write;x++)
            {
                uint64_t tsp_end,tsp_start;
                write_nt256_time(mapped_area, &tsp_start, &tsp_end);
            }

            for(int x=0;;x++)
            {
                uint64_t tsp_end,tsp_start;
                write_nt256_time(mapped_area+offset, &tsp_start, &tsp_end);
                uint64_t lat = tsp_end - tsp_start;
                if (lat > WEAR_LEVEL_LATENCY )
                    break;
            }

            for(int x=0;;x++)
            {
                uint64_t tsp_end,tsp_start;
                write_nt256_time(mapped_area, &tsp_start, &tsp_end);
                uint64_t lat = tsp_end - tsp_start;
                if (lat > WEAR_LEVEL_LATENCY )
                {
                    sum += x;
                    break;
                }
            }
        }
        int avg = sum / reps_per_loc;
        printf("%d,%d\n",offset,avg);
	}

	pmem_exp_end();
	return 0;
}
