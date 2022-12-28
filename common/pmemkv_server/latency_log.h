#pragma once
#include <stdint.h>
int init_records();
void reset_records();
int dump_records(char* server_logname);
void update_records(uint64_t op_latency,char desc);
