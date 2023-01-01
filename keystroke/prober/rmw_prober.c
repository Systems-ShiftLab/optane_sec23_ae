#include "util.h"

#include "config.h"
#include "misc.h"

#define REGION_SIZE (1l*1024*1024*1024)
#include <stdio.h>
#include <sys/time.h>

char *rmw_area;
uint64_t   rmw_curr_offset, rmw_next_offset;
         

int probe_rmw() {
//    warmup_cpu();
    rmw_curr_offset = rmw_next_offset;
    // prime
    
    rmw_curr_offset = load_fwd_list2(rmw_area, rmw_curr_offset, PROBE_CNT - 1);

    load_fwd_list2(rmw_area, rmw_next_offset, PRIME_CNT);
//    load_fwd_list1(rmw_area, rmw_next_offset, PRIME_CNT);
    rmw_next_offset = load_fwd_list1(rmw_area, rmw_next_offset, PRIME_CNT);


//pmem_memset_persist(pmem2_area,0,200);
//    _mm_mfence();
//    pmem2_area[0] = 0;
//    cache_flush(pmem2_area,64);

    spin_wait_cycle(PR_PRB_WAIT);

    int rmw_misses = 0;
    for (int i=0; i<PROBE_CNT; i++) {
        uint64_t start, end;
        uint64_t  lat;
        
        start = get_cycle();
        // Shift by 64B and run pointer-chasing
        rmw_curr_offset = load_bkwd_list1(rmw_area, rmw_curr_offset, 1);
        end = get_cycle();
        lat = end-start;
        if (lat > RMW_LAT_THRES)
            rmw_misses++;
    }
    return rmw_misses;
    //rmw_curr_offset = load_fwd_list1(rmw_area, rmw_curr_offset, 2*PRIME_CNT); //Just leaving space
}


void listen_channel_keystroke() {
	while(1) {
        uint64_t now = get_usec();
        int evict_cnt = probe_rmw();
        printf("%lu;%d\n", now, evict_cnt);
        fflush(stdout);
	}
}

int main(int argc, char **argv){


	if (argc != 2) {
		printf("usage: %s file-name\n", argv[0]);
		return 1;
	}

    rmw_area = create_pmem(argv[1], REGION_SIZE);
    

    gen_fwd_rev_chasing_list(rmw_area, REGION_SIZE / RMWL_SIZE);
    rmw_curr_offset = 0;

    // Flush all areas
    cache_flush(rmw_area, REGION_SIZE);

    fprintf(stderr, "Init complete\n");

    listen_channel_keystroke(); //This should loop forever

    pmem_unmap(rmw_area, REGION_SIZE);
}
