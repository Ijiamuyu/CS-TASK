#include "mmalloc.h"

#define TEST_SIZE1 209
#define TEST_SIZE2 10240
#define TEST_SIZE3 10087


int main(void)
{
    uint32_t i;
    uint8_t *ptr1, *ptr2, *ptr3;

    printf("start malloc test\n");
    memory_init();
    printf("after memory init\n");
    print_malloc_info();
    ptr1 = mmalloc(TEST_SIZE1);
    if (ptr1 == NULL)
    {
        printf("malloc test fail\n");
        return 0;
    }
    for (i = 0; i < TEST_SIZE1; i++)
    {
        ptr1[i] = (uint8_t)i;
    }
    printf("after mmalloc ptr1\n");
    print_malloc_info();

    ptr2 = mmalloc(TEST_SIZE2);
    if (ptr2 == NULL)
    {
        printf("malloc test fail\n");
        return 0;
    }

    for (i = 0; i < TEST_SIZE2; i++)
    {
        ptr2[i] = (uint8_t)i;
    }

    printf("after mmalloc ptr2\n");
    print_malloc_info();
    mfree(ptr2);
    printf("after mfree ptr2\n");
    print_malloc_info();
    ptr3 = mmalloc(TEST_SIZE3);
    if (ptr3 == NULL)
    {
        printf("malloc test fail\n");
        return 0;
    }
    for (i = 0; i < TEST_SIZE3; i++)
    {
        ptr3[i] = (uint8_t)i;
    }
    printf("after mmalloc ptr3\n");
    print_malloc_info();
    mfree(ptr1);
    printf("after mfree ptr1\n");
    print_malloc_info();
    mfree(ptr3);
    printf("after mfree ptr3\n");
    print_malloc_info();
    printf("end malloc test\n");
    return 0;
}
