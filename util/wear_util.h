#pragma once

#define WEAR_LEVEL_LATENCY 100000
#define WEAR_LEVEL_GRANULARITY 4096

extern inline __attribute__((always_inline))
void write_nt256_time(char *addr,uint64_t *tsp_start, uint64_t *tsp_end)
{
	asm volatile (

	TSP_START
	"vmovntdq  %%zmm0, 0x0(%[addr]) \n" 
	"clflush 0x0(%[addr]) \n"
//	"vmovntdqa  0x0(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0x40(%[addr]) \n"
	"clflush 0x40(%[addr]) \n"
//	"vmovntdqa  0x40(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0x80(%[addr]) \n"
	"clflush 0x80(%[addr]) \n"
//	"vmovntdqa  0x80(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0xC0(%[addr]) \n"
	"clflush 0xC0(%[addr]) \n"
//	"vmovntdqa  0xC0(%[addr]), %%zmm0 \n"
    "mfence \n"
	TSP_END

	: [tsp_start]"=&r"(*tsp_start),[tsp_end]"=&r"(*tsp_end)
	: [addr]"r"(addr)
	:  "%rdx", "%rax","rcx", "memory");
}

extern inline __attribute__((always_inline))
void write_read_nt256_time(char *addr,uint64_t *tsp_start, uint64_t *tsp_end)
{
	asm volatile (

	TSP_START
	"vmovntdq  %%zmm0, 0x0(%[addr]) \n" 
	"clflush 0x0(%[addr]) \n"
	"vmovntdqa  0x0(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0x40(%[addr]) \n"
	"clflush 0x40(%[addr]) \n"
	"vmovntdqa  0x40(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0x80(%[addr]) \n"
	"clflush 0x80(%[addr]) \n"
	"vmovntdqa  0x80(%[addr]), %%zmm0 \n"
	"vmovntdq  %%zmm0, 0xC0(%[addr]) \n"
	"clflush 0xC0(%[addr]) \n"
	"vmovntdqa  0xC0(%[addr]), %%zmm0 \n"
    "mfence \n"
	TSP_END

	: [tsp_start]"=&r"(*tsp_start),[tsp_end]"=&r"(*tsp_end)
	: [addr]"r"(addr)
	:  "%rdx", "%rax","rcx", "memory");
}

extern inline __attribute__((always_inline))
void read_nt256_time(char *addr,uint64_t *tsp_start, uint64_t *tsp_end)
{
	asm volatile (

	TSP_START
	"vmovntdqa  0x0(%[addr]), %%zmm0 \n"
	"vmovntdqa  0x40(%[addr]), %%zmm0 \n"
	"vmovntdqa  0x80(%[addr]), %%zmm0 \n"
	"vmovntdqa  0xC0(%[addr]), %%zmm0 \n"
    "mfence \n"
	TSP_END

	: [tsp_start]"=&r"(*tsp_start),[tsp_end]"=&r"(*tsp_end)
	: [addr]"r"(addr)
	:  "%rdx", "%rax","rcx", "memory");
}
