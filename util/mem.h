#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <x86intrin.h>
#include "timing.h"

#define LOAD_FENCE "mfence\n"

#ifdef USE_LOAD_LFENCE
#undef LOAD_FENCE
#define LOAD_FENCE "lfence\n"
#endif

#ifdef USE_LOAD_NOFENCE
#undef LOAD_FENCE
#define LOAD_FENCE "\n"
#endif

void load_time(char *addr, uint64_t *tsp_start, uint64_t* tsp_end)
{
	asm volatile (
	"mov $0x10000,%%ecx\n"
	".LOOP%=:\n"
	"loop .LOOP%=\n"


	TSP_START
#ifdef USE_NORMAL_LOAD
	"movq 0x0(%[start_addr]), %%rcx \n"
#else
	"vmovntdqa  0x0(%[start_addr]), %%zmm0 \n" 
#endif
	LOAD_FENCE
	TSP_END

		: [tsp_start]"=&r"(*tsp_start),[tsp_end]"=&r"(*tsp_end)
		: [start_addr]"r"(addr)
		: "%rdx", "%rax","rcx", "memory");
}

void clflush_region(void* addr, size_t size)
{
	for (size_t i=0;i<size;i+=64)
		_mm_clflush((char*)addr+i);
	asm volatile ("mfence\n");
}
#define STORE_FENCE_TYPE "mfence"
#define LOAD_FENCE_TYPE "mfence"
//#define CHASING_MFENCE_TYPE   "mfence"

//#define STORE_FENCE STORE_FENCE_TYPE "\n"
//#define LOAD_FENCE LOAD_FENCE_TYPE "\n"
//#define CHASING_MFENCE   CHASING_MFENCE_TYPE   "\n"


/* 
 * Macros to generate memory access instructions.
 * Use `gcc -E chasing.h` to view macro expansion results.
 * You may use `clang-format` to format the gcc results.
 */
#define STORE_64_INNER(cl_index)                                          \
	"vmovntdq	%%zmm0, (64 * (" #cl_index "))(%%r9, %%r12)\n"

#define STORE_64(cl_base) STORE_64_INNER(cl_base)

#define STORE_128(cl_base)                                                \
	STORE_64(cl_base + 0)                                             \
	STORE_64(cl_base + 1)

#define STORE_256(cl_base)                                                \
	STORE_128(cl_base + 0)                                            \
	STORE_128(cl_base + 2)

#define STORE_512(cl_base)                                                \
	STORE_256(cl_base + 0)                                            \
	STORE_256(cl_base + 4)

#define STORE_1024(cl_base)                                               \
	STORE_512(cl_base + 0)                                            \
	STORE_512(cl_base + 8)

#define STORE_2048(cl_base)                                               \
	STORE_1024(cl_base + 0)                                           \
	STORE_1024(cl_base + 16)

#define STORE_4096(cl_base)                                               \
	STORE_2048(cl_base + 0)                                           \
	STORE_2048(cl_base + 32)


#define LOAD_64_INNER(cl_index)                                          \
	"vmovntdqa	(64 * (" #cl_index "))(%%r9, %%r12), %%zmm0\n"

#define LOAD_64(cl_base)                                                 \
	LOAD_64_INNER(cl_base)

#define LOAD_128(cl_base)                                                \
	LOAD_64(cl_base + 0)                                             \
	LOAD_64(cl_base + 1)

#define LOAD_256(cl_base)                                                \
	LOAD_128(cl_base + 0)                                            \
	LOAD_128(cl_base + 2)

#define LOAD_512(cl_base)                                                \
	LOAD_256(cl_base + 0)                                            \
	LOAD_256(cl_base + 4)

#define LOAD_1024(cl_base)                                               \
	LOAD_512(cl_base + 0)                                            \
	LOAD_512(cl_base + 8)

#define LOAD_2048(cl_base)                                               \
	LOAD_1024(cl_base + 0)                                           \
	LOAD_1024(cl_base + 16)

#define LOAD_4096(cl_base)                                               \
	LOAD_2048(cl_base + 0)                                           \
	LOAD_2048(cl_base + 32)

#endif 
