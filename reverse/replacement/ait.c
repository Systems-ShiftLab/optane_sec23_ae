#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<libpmem.h>

#include "experiment.h"
#include "util.h"
#include "mem.h" //TODO

#define AIT_SET_COUNT 256

int main(int argc, char* argv[])
{

	if(argc != 4)
	{
		fprintf(stderr,"Usage: %s [num_node] [set] [pattern-0/1/2]",argv[0]);
		exit(-1);
	}

	int node_count = atoi(argv[1]);
	int set_number = atoi(argv[2]);
    int access_pattern = atoi(argv[3]);

	PMEM_EXP_INIT
	
	uint64_t  *result = calloc(node_count*2, sizeof(uint64_t));

	uint64_t* perm_array = malloc (node_count* sizeof(uint64_t));
	gen_perm(perm_array,node_count);

	warmup_cpu();
	for(int i=0;i<node_count;i++)
	{
        uint64_t start,end;
		uint64_t offset = (perm_array[i]*AIT_SET_COUNT + set_number) * AITL_SIZE;
		load_time(&mapped_area[offset],&start,&end);
		result[i] = end-start;
	}

    if(access_pattern == 1)
        for (int low = 0, high = node_count - 1; low < high; low++, high--)
        {
            uint64_t temp = perm_array[low];
            perm_array[low] = perm_array[high];
            perm_array[high] = temp;
        }

    if(access_pattern == 2)
        gen_perm(perm_array,node_count);


	for(int i=0;i<node_count;i++)
	{
        uint64_t start,end;
		uint64_t offset = (perm_array[i]*AIT_SET_COUNT + set_number) * AITL_SIZE + RMWL_SIZE;
		load_time(&mapped_area[offset],&start,&end);
		result[node_count+i] = end-start;
	}

#define AIT_MISS_LAT 860
    int miss_count=0;
	for(int i=node_count;i<node_count*2;i++)
        if(result[i] > AIT_MISS_LAT) miss_count++;

    printf("%d\n",miss_count);

    PMEM_EXP_END
	return 0;
}
