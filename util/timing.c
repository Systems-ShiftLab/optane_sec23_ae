#include <stdint.h>
#include <x86intrin.h>

void sleep_ticks(uint64_t tick_count)
{
	int junk;
	uint64_t tsp_start = __rdtscp(&junk);
	while((__rdtscp(&junk) - tsp_start) < tick_count);
}
