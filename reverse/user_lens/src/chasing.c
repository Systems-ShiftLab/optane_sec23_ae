#include <stdlib.h> 
#include <time.h>
#include "chasing.h" 

void swap (uint64_t* t1, uint64_t* t2)
{
  uint64_t temp = *t1;
  *t1 = *t2;
  *t2 = temp;
}

int init_chasing_index(uint64_t *cindex, uint64_t count) 
{
  srand(time(NULL));
  uint64_t* perm_array = malloc ((count)*sizeof(uint64_t));
  for(uint64_t i=0;i<count;i++)
    perm_array[i] = i;

  for(int i=count-2;i>=0;i--)
  {
    uint64_t temp = rand() % (i+1);
    swap(&perm_array[i],&perm_array[temp]);
  }
  //Now we have an array like this 4 2 1 3 5. The last number will be the index

  for(int i=0;i<count-1;i++)
    cindex[perm_array[i]] = perm_array[i+1];
  cindex[count - 1] = perm_array[0]; //count-1 == perm_array[count-1];

  free(perm_array);
  return 0;
}

int chasing_find_func(unsigned long block_size) {
	int ret = -1;
	int i;
	int len = sizeof(chasing_func_list) / sizeof(chasing_func_entry_t);

	for (i = 0; i < len; i++) {
		if (chasing_func_list[i].block_size == block_size) {
			ret = i;
			break;
		}
	}

	return ret;
}
