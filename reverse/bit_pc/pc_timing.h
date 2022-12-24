#pragma once
#define PC_VARS                                                                \
	unsigned int c_load_start_hi, c_load_start_lo;                         \
	unsigned int c_load_end_hi, c_load_end_lo;                             \
	unsigned long c_store_start;                                           \
	unsigned long c_load_start, c_load_end;

#define PC_BEFORE_READ                                                         \
	asm volatile("rdtscp \n\t"                                             \
			"lfence \n\t"                                             \
			"mov %%edx, %[hi]\n\t"                                    \
			"mov %%eax, %[lo]\n\t"                                    \
			: [hi] "=r"(c_load_start_hi), [lo] "=r"(c_load_start_lo)  \
			:                                                         \
			: "rdx", "rax", "rcx");

#define PC_AFTER_READ                                                          \
	asm volatile("rdtscp \n\t"                                             \
			"lfence \n\t"                                             \
			"mov %%edx, %[hi]\n\t"                                    \
			"mov %%eax, %[lo]\n\t"                                    \
			: [hi] "=r"(c_load_end_hi), [lo] "=r"(c_load_end_lo)      \
			:                                                         \
			: "rdx", "rax", "rcx");                                   \

#define PC_PRINT_MEASUREMENT			                             \
	c_load_start = (((unsigned long)c_load_start_hi) << 32) | c_load_start_lo; \
	c_load_end = (((unsigned long)c_load_end_hi) << 32) | c_load_end_lo;  \
	printf("%ld\n",(c_load_end - c_load_start)/region_count);
