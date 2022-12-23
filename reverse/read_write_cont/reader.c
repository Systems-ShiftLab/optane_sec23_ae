#include "util.h"
#include "experiment.h"
#include <sys/time.h>

int main(int argc, char **argv){

	if(argc != 2)
	{
		fprintf(stderr,"Usage: %s <pool_path>",argv[0]);
		exit(0);
	}
    
    uint64_t init_start, init_end;

    void* pmem_area = create_pmem(argv[1], REGION_SIZE);

    gen_chasing_list(pmem_area, REGION_SIZE / RMWL_SIZE, RMWL_SIZE);
    uint64_t cont_curr_offset = 0;

    // Flush all areas
    cache_flush(pmem_area, REGION_SIZE);

    fprintf(stderr, "Init complete\n");

    uint64_t chase_start, chase_end, chase_cycle;

    while(1){
        chase_start = get_cycle();
        cont_curr_offset = load_list_count(pmem_area, cont_curr_offset, 200);
        chase_end = get_cycle();
        chase_cycle = chase_end - chase_start;
        printf("%ld\n",chase_cycle/200);
    }

    pmem_unmap(pmem_area, REGION_SIZE);
}
