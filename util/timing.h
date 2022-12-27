#pragma once
#define TSP_READ(x) \
  "rdtscp\n" \
  "mov %%rdx,%[" #x "]\n" \
  "shl $32,%[" #x "]\n"   \
  "or %%rax,%[" #x "]\n"

#define TSP_START \
  TSP_READ(tsp_start)

#define TSP_END \
  TSP_READ(tsp_end)

struct result_tuple{
	uint64_t tsp_start;
	uint64_t tsp_end;
	uint64_t offset;
};


#define PRINT_RESULT \
	printf("Offset\tLatency\n"); \
	for(int i=0;i<node_count;i++) \
	{ \
		printf("%lu\t%lu\n",result[i].offset ,result[i].tsp_end - result[i].tsp_start); \
	} 

void sleep_ticks(uint64_t tick_count);
