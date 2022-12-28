#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <libpmemkv.h>


int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s file utilization\n", argv[0]);
        exit(1);
    }

    pmemkv_config *cfg = pmemkv_config_new();
    assert(cfg != NULL);

    double util = atof(argv[2]);
    double wait_ratio = (1.0/util) - 1.0;

    int s = pmemkv_config_put_path(cfg, argv[1]);
    assert(s == PMEMKV_STATUS_OK);
    s = pmemkv_config_put_size(cfg, 1024*1024*32);
    assert(s == PMEMKV_STATUS_OK);
	s = pmemkv_config_put_create_if_missing(cfg, true);
	assert(s == PMEMKV_STATUS_OK);

    pmemkv_db *db = NULL;
    s = pmemkv_open("cmap", cfg, &db);
    printf("%d\n",s);
    assert(s == PMEMKV_STATUS_OK);
    assert(db != NULL);

    char key0[] = "wsim0";
    char key1[] = "wsim1";
    char key2[] = "wsim2";
    char key3[] = "wsim3";
    size_t keylen = 5;
    size_t valuelen = 512;
    char *value = malloc(valuelen);
    memset(value,'a',valuelen);

#define PUT(x) do {\
s = pmemkv_put(db, key##x, keylen, value, valuelen); \
assert(s == PMEMKV_STATUS_OK); \
} while(0)

#define GET(x) do {\
s = pmemkv_get_copy(db, key##x, keylen, value, valuelen, NULL); \
assert(s == PMEMKV_STATUS_OK); \
} while(0)


    while(1)
    {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        PUT(0);
        GET(0);
        GET(0);
        PUT(1);
        GET(1);
        GET(1);
        GET(1);
        GET(1);
        GET(0);
        GET(0);
        GET(1);
        GET(0);
        GET(1);

        PUT(2);
        GET(2);
        GET(2);
        PUT(3);
        GET(3);
        GET(3);
        GET(3);
        GET(3);
        GET(2);
        GET(2);
        GET(3);
        GET(2);
        GET(3);
        gettimeofday(&end, NULL);
        unsigned long time_diff = (end.tv_sec - start.tv_sec) * 1000000  + end.tv_usec - start.tv_usec;
        unsigned long wait_time = time_diff * wait_ratio;
        printf("Execution time: %ld. Wait time: %ld\n",time_diff,wait_time);
        usleep(wait_time);

    }

    pmemkv_close(db);
    return 0;
}
