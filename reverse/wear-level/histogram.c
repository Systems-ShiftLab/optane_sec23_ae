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

	if(argc != 3)
	{
		fprintf(stderr,"Usage: %s <num_wl> <case 0/1 (write/write_read)>",argv[0]);
		exit(0);
	}

    int num_wl = atoi(argv[1]);
    int cse = atoi(argv[2]);
	pmem_exp_init("/mnt/ramdisk/pmem_overwr_pool",REGION_SIZE,false);


	for(int i=0;i<num_wl;)
	{

        for(int x=0;;x++)
        {
            uint64_t tsp_end,tsp_start;
            if (cse == 0)
                write_nt256_time(mapped_area, &tsp_start, &tsp_end);
            else if (cse == 1)
                write_read_nt256_time(mapped_area, &tsp_start, &tsp_end);
            uint64_t lat = tsp_end - tsp_start;
            if (lat > WEAR_LEVEL_LATENCY )
            {
                i++;
                printf("%d %ld\n",x,lat);
                break;
            }
        }
	}

	pmem_exp_end();
	return 0;
}
