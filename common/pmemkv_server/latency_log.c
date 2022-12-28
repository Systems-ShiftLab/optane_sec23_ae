#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_REC 1000000 

struct log_record {
    int s_no;
    uint64_t latency;
    char meta;
};

static struct log_record* record_array;
static _Atomic int global_record_no;

void reset_records()
{
    global_record_no = 0;
}

int init_records()
{
    record_array = malloc(NUM_REC * sizeof(struct log_record));
    if(!record_array) return -1;
    reset_records();
    return 0;
}

void update_records(uint64_t op_latency,char desc)
{
    int curr = global_record_no++;
    int arr_idx = curr%NUM_REC;
    struct log_record* current = &record_array[arr_idx];
    current -> s_no = curr;
    current -> latency = op_latency;
    current -> meta = desc;;
}

int dump_records(char* logname)
{
    FILE* f = fopen(logname,"w");
    if (!f) return -1;

    fprintf(f,"s_no\tlatency\tdesc\n");

    int iter_count = global_record_no<NUM_REC?global_record_no:NUM_REC; 
    for (int i = 0; i<iter_count; i++)
    {
        struct log_record* current = &record_array[i];
        fprintf(f,"%d\t%ld\t%c\n",current->s_no, current->latency,current->meta);
    }
    fclose(f);
    return 0;
}
