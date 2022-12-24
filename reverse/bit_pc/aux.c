#include <stdlib.h> 
#include <stdint.h>
#include <time.h>
#include "aux.h" 

static void swap (uint64_t* t1, uint64_t* t2)
{
  uint64_t temp = *t1;
  *t1 = *t2;
  *t2 = temp;
}

unsigned int count_bits(uint64_t n)
{
    unsigned int count = 0;
    while (n) {
        n &= (n - 1);
        count++;
    }
    return count;
}

#define IS_BIT_SET(n,bp) (1<<bp) & n
static uint64_t bitmask_mapping(uint64_t input, uint64_t bitmask)
{
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
	return output^bitmask;
}
void gen_bitmask_perm(uint64_t *output, uint64_t count, uint64_t bitmask) 
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
  //Since we are fixing the last number, we can think of it as (4 2 1 3 5). We can generate a cyclic permutation from this

  for(int i=0;i<count-1;i++)
    output[bitmask_mapping(perm_array[i],bitmask)] = bitmask_mapping(perm_array[i+1],bitmask);
  output[bitmask_mapping(count - 1,bitmask)] = bitmask_mapping(perm_array[0],bitmask); //count-1 == perm_array[count-1];

  free(perm_array);
}
