/*
 * Contains code for
 * Flushing cache
 * Getting rdtsc ticks
 * Getting gettimeofday ticks
 * Spin waits
 * Cyclic permutation generation
 * TODO: Make a clear persistent memory initialization function
 *
 */
#include "util.h"

extern inline __attribute__((always_inline))
void cache_flush(void* addr, uint64_t size) {
	for (uint64_t i = 0; i < size; i += CL_SIZE) {
		_mm_clflush((void*)((uint64_t)addr + i));
	}
}

/* code from UIUC */
extern inline __attribute__((always_inline))
uint64_t rdtsc() {
    uint64_t a, d;
    asm volatile ("lfence");
    asm volatile ("rdtsc" : "=a" (a), "=d" (d));
    asm volatile ("lfence");
    return (d << 32) | a;
}

// Return cycle (high precision)
inline uint64_t get_cycle() {
    return rdtsc();
}

// Return usec (low precision)
extern inline __attribute__((always_inline))
uint64_t get_usec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * MICRO_PER_SEC + tv.tv_usec;
}

extern inline __attribute__((always_inline)) //TODO: We don't need this attribute though. Try without it
uint64_t sync_cycle(uint64_t sync_mask,uint64_t sync_jitter) {
	uint64_t start, end;

    start = get_cycle();
    while ((get_cycle() & sync_mask) > sync_jitter) {}
    end = get_cycle();

    // printf("Sync cycle = %lu\n", end - start);

    return end;
}

extern inline __attribute__((always_inline))
uint64_t sync_usec(uint64_t sync_mask,uint64_t sync_jitter) {
    uint64_t start, end;

    start = get_usec();
    while ((get_usec() % sync_mask) > sync_jitter) {}
    end = get_usec();

//	 printf("Sync cycle = %lu\n", end - start);
//	 printf("Time = %lu\n", end);
	return end;
}

inline void spin_wait_cycle(uint64_t time_cycle) {
    uint64_t start = get_cycle();
    while (get_cycle() - start < time_cycle) {}
}

inline void spin_wait_usec(uint64_t time_usec) {
    uint64_t start = get_usec();
    while (get_usec() - start < time_usec) {}
}

static inline void swap (uint64_t* t1, uint64_t* t2) {
	uint64_t temp = *t1;
	*t1 = *t2;
	*t2 = temp;
}

inline int get_rand(uint64_t *rd, uint64_t range) {
#ifdef USE_STDLIB_RAND
    *rd = rand() % (range + 1);
    return 0;
#else
    uint8_t ok;
    int i = 0;
    for (i = 0; i < 25; i++) {
        asm volatile("rdrand %0; setc %1\n\t"
                     : "=r"(*rd), "=qm"(ok));
        if (ok) {
            *rd = *rd % range;
			return 0;
        }
    }
	return 1;
#endif
}

//Generates a cyclic perm,  the order of perm is n
void gen_chasing_index(uint64_t* cindex,uint64_t count) 
{
  uint64_t* perm_array = malloc ((count)*sizeof(uint64_t));

  gen_perm(perm_array,count);
  //Now we have an array like this 4 2 1 3 5. The last number will be the index

  for(int i=0;i<count-1;i++)
    cindex[perm_array[i]] = perm_array[i+1];
  cindex[count - 1] = perm_array[0]; //count-1 == perm_array[count-1];

  free(perm_array);
}

void store_list(char *start_addr, uint64_t node_count, uint64_t *cindex, uint64_t node_size) {
	uint64_t pmem_size = node_count * node_size;

	asm volatile (
		"lea	(%[start_addr]), %%r9 \n"
		"movq	%[node_size], %%r13\n"
		"xor	%%r10, %%r10 \n"			
		"xor	%%r12, %%r12 \n"				

"LOOP_ST%=: \n"
		"movq	(%[cindex], %%r12, 8), %%r11\n"
		"movq 	%%r11,  0x0(%%r9, %%r10) \n"
		"movq	%%r11,  0x40(%%r9, %%r10) \n"
		"movq	%%r11,  0x80(%%r9, %%r10) \n"
		"movq	%%r11,  0xC0(%%r9, %%r10) \n"
		"clflush 	0x0(%%r9, %%r10)\n"
		"add    %%r13, %%r10\n"      
		"inc	%%r12\n"		

		"cmp	%[accesssize], %%r10 \n"  //accesssize is the number of elements in the cindex array
		"jl		LOOP_ST%= \n"
		"mfence \n"
		: 
		: [start_addr]"r"(start_addr), [accesssize]"r"(pmem_size), [cindex]"r"(cindex), [node_size]"r"(node_size)
		:"r10", "r9", "r12", "r11", "r13");
}

int gen_perm(uint64_t* perm_array,uint64_t count)
{
//    srand(time(NULL));
    for(uint64_t i = 0; i < count; i++) {
        perm_array[i] = i;
    }

    for(int i =count-2; i >= 0; i--) {
        uint64_t temp;
        get_rand(&temp, i + 1);
        swap(&perm_array[i], &perm_array[temp]);
    }

  return 0;
}

//Calls gen_chasing_index to generate the cyclic permutation, and stores the list as well
void gen_chasing_list(void* pmem_ptr, int count, int node_size) {
    uint64_t* cindex = (uint64_t*)malloc(sizeof(uint64_t) * count);

	gen_chasing_index(cindex, count);
	for (int i = 0; i < count; ++i) {
		cindex[i] *= node_size;
		// printf("%lu\n", cindex[i]);
	}
    store_list(pmem_ptr, count, cindex, node_size);
    free(cindex);
}

static uint64_t bitmask_mapping(uint64_t input, uint64_t bitmask,uint64_t bitmask_val)
{
#define IS_BIT_SET(n,bp) ((1<<bp) & n)
	uint64_t output=bitmask;
	int curr_bit_pos=0;
	while(input)
	{
		if(IS_BIT_SET(bitmask,curr_bit_pos))
		{
			curr_bit_pos++;
			continue;
		}
		int target_bit = input&1;
		output |= target_bit<<curr_bit_pos;
		
		input>>=1;
		curr_bit_pos++;
	}
	bitmask_val = bitmask & bitmask_val;
	return (output^bitmask)|bitmask_val;
}

//bitmask argument will specify which bits to keep constant, and bitmask_val will specify the exact value of those bits
//bitmask|bitmask_val must be 0 because any bit which is not set in the bitmask will change anyway, and is not controlled
//Eg. A bitmask_val of 1 (01) with a bitmask of 3 (11) will ensure that the lowest 2 bits are 01.
//Eg. A bitmask_val of 2 (10) with a bitmask of 3 (11) will ensure that the lowest 2 bits are 10.
void gen_bitmask_chasing_index(uint64_t *cindex, uint64_t count, uint64_t bitmask,uint64_t bitmask_val) 
{
  	uint64_t* perm_array = (uint64_t*)malloc((count)*sizeof(uint64_t));
    gen_perm(perm_array,count);

	for(int i = 0; i < count - 1; i++) {
    	cindex[bitmask_mapping(perm_array[i], bitmask, bitmask_val)] 
					= bitmask_mapping(perm_array[i+1], bitmask, bitmask_val);
	}
	cindex[bitmask_mapping(count - 1, bitmask, bitmask_val)] 
				= bitmask_mapping(perm_array[0],bitmask,bitmask_val); //count-1 == perm_array[count-1];
	
	free(perm_array);
}

//TODO: This shouldn't be in a common directory
void gen_ait_chasing_list(void* pmem_ptr, int count, int node_size, uint64_t ait_index_bits) {
	uint64_t bitmask_val = ait_index_bits;
	uint64_t bitmask = AIT_MASK >> 12; // generate cindex without including the last 12 bits
	
	int total_count = count << 8; // only one set in AIT will be used
    //Here, total_count will cover the entire memory
	uint64_t* cindex = (uint64_t*)malloc(sizeof(uint64_t) * total_count);
	memset(cindex, 0 , sizeof(uint64_t) * total_count);
	gen_bitmask_chasing_index(cindex, count, bitmask, bitmask_val); //The size of cindex array MUST be >= the number of nodes. This is because entries in cindex where the bitmask is not equal will not be part of the pointerchasing chain

	for (int i = 0; i < total_count; ++i) {
		cindex[i] <<= 12;
	}
	
	// Store cindex to PM
	for (int i = 0; i < total_count; ++i) {
		if (cindex[i] != 0) {
			for (int j = 0; j < AITL_SIZE; j += CL_SIZE) {
				*((uint64_t*)(pmem_ptr + i * AITL_SIZE + j)) = cindex[i]; //TODO + j;
			}
		}
	}
}

uint64_t load_list_count_flush(void *list_addr, uint64_t start_offset, uint64_t node_count) {
						
	uint64_t end_offset;
	
	asm volatile(
		"xor    %%r10, %%r10 \n"
		"mov    %[start_offset], %%r12 \n"

		// "rdtscp\n"
		// "lfence\n"
		// "mov 	%%rdx, %[time_start]\n"
		// "shl 	$32, %[time_start]\n"
		// "or 	%%rax, %[time_start]\n"

		"LOOP_INNER3: \n"
		"vmovntdqa (%[list_addr], %%r12), %%zmm0\n"

		"clflush 0x0(%[list_addr], %%r12)\n"
		// "mfence\n"

		"movq	%%xmm0, %%r12\n"

		"inc	%%r10\n"
		"cmp	%[count], %%r10\n"
		"jl LOOP_INNER3 \n"

		// "rdtscp\n"
		// "lfence\n"
		// "mov 	%%rdx, %[time_end]\n"
		// "shl 	$32, %[time_end]\n"
		// "or 	%%rax, %[time_end]\n"
		// "mfence\n" // Needed to enforce cache flush in NUMA systems

		"mov	%%r12, %[end_offset]\n"

		: [end_offset]"=&r"(end_offset) /*, [time_end]"=&r"(end), [time_start]"=&r"(start) */
		: [list_addr]"r"(list_addr), [start_offset]"r"(start_offset), [count]"r"(node_count)
		: "r10", "r12", "rax", "rdx", "rcx", "zmm0");
	// *tsp_start = start;
	// *tsp_end = end;

	return end_offset;
}

uint64_t load_list_count(void *list_addr, uint64_t start_offset, uint64_t node_count) {
							// uint64_t* tsp_start, uint64_t* tsp_end)  {
						
	uint64_t end_offset; //start, end, ;
	
	asm volatile(
		"xor    %%r10, %%r10 \n"
		"mov    %[start_offset], %%r12 \n"

		// "rdtscp\n"
		// "lfence\n"
		// "mov 	%%rdx, %[time_start]\n"
		// "shl 	$32, %[time_start]\n"
		// "or 	%%rax, %[time_start]\n"

		"LOOP_INNER1: \n"
		"vmovntdqa (%[list_addr],%%r12), %%zmm0\n"
		"movq	%%xmm0, %%r12\n"
		// "mfence\n"

		"inc	%%r10\n"
		"cmp	%[count], %%r10\n"
		"jl LOOP_INNER1 \n"

		// "rdtscp\n"
		// "lfence\n"
		// "mov 	%%rdx, %[time_end]\n"
		// "shl 	$32, %[time_end]\n"
		// "or 	%%rax, %[time_end]\n"

		"mov	%%r12, %[end_offset]\n"

		: /*[time_start]"=&r"(start), [time_end]"=&r"(end),*/ [end_offset]"=&r"(end_offset)
		: [list_addr]"r"(list_addr), [start_offset]"r"(start_offset), [count]"r"(node_count)
		: "r10", "r12", "rax", "rdx", "rcx", "zmm0");
	// *tsp_start = start;
	// *tsp_end = end;

	return end_offset;
}

uint64_t load_list_time(void *list_addr, uint64_t start_offset, uint64_t cycle) {
	uint64_t end_offset;

	asm volatile(
		"mov    %[start_offset], %%r12 \n"

		"rdtscp\n"
		"lfence\n"
		"shl 	$32, %%rdx\n"
		"or 	%%rax, %%rdx\n"
		"mov 	%%rdx, %%r10\n"

		"LOOP_INNER2: \n"
		"vmovntdqa (%[list_addr],%%r12), %%zmm0\n"
		"movq	%%xmm0, %%r12\n"
		
		"rdtscp\n"
		"lfence\n"
		"shl 	$32, %%rdx\n"
		"or 	%%rax, %%rdx\n"
		"mov 	%%rdx, %%r11\n"

		"sub 	%%r10, %%r11\n"
		"cmp 	%[time], %%r11\n"
		"jl LOOP_INNER2\n"

		"mov	%%r12, %[end_offset]\n"

		: [end_offset]"=&r"(end_offset)
		: [start_offset]"r"(start_offset), [list_addr]"r"(list_addr), [time]"r"(cycle)
		: "r10", "r11", "r12", "r13", "rax", "rdx", "rcx", "zmm0");

	return end_offset;
}

void* create_pmem(char* file_name, uint64_t size) {
    size_t len;
	int is_pmem;
    void* pmem_ptr = pmem_map_file(file_name, size, PMEM_FILE_CREATE, 0666, &len, &is_pmem);
	if(!is_pmem) {
		fprintf(stderr, "Not pmem region");
		exit(1);
	}
	if(len != size) {
		fprintf(stderr, "Region size small");
		exit(1);
	}
    pmem_memset_persist(pmem_ptr, 0, size);

    return pmem_ptr;
}

void warmup_cpu()
{
  for(int i=0;i<100000000;i++);
}
