# 手撕mmalloc

参考：[自己动手实现一个malloc内存分配器 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/367060283)

如果你不能理解malloc之类内存分配器实现原理的话，那你可能就写不出高性能程序，写不出高性能程序就很难参与核心项目，参与不了核心项目那么很难升职加薪，很难升级加薪就无法走向人生巅峰，没想到内存分配竟如此关键，为了走上人生巅峰你也要势必读完本文。皮一下。。。

## 设计思路

<img title="" src="file:///E:/Task/Task1/mmalloc/mmalloc.svg" alt="" data-align="center">

通过上图中的block块记录申请块的大小，f/a代表此块有没有被使用，并通过header与tailer实现快速回收。总体思路是采用了First Fit分配算法，找到一个满足要求的内存块后会进行切分，剩下的作为新的内存块；同时当释放内存时会立即合并相邻的空闲内存块，同时为加快合并速度，引入了Donald Knuth的设计方法，为每个内存块增加tailer信息。

## 代码解析

```c
#include <stdio.h>
#include <stdint.h>

#define MMALLOC_MAX_MEM 0x6400000

typedef struct MMallocInfo
{
    uint32_t used;
} MMallocInfo;

static uint8_t block[MMALLOC_MAX_MEM];
static MMallocInfo minfo;

static void *creat_block(uint8_t *start, uint32_t len, uint8_t used)
{
    uint32_t *ptr = (uint32_t *)start;

    *ptr = (len << 1) | used;
    *(ptr + len / sizeof(uint32_t) + 1) = (len << 1) | used;
    return (start + sizeof(uint32_t));
}

static uint8_t *find_useful_mem(uint32_t size)
{
    uint32_t i, *ptr = NULL;

    for (i = 0; i < MMALLOC_MAX_MEM - 1;)
    {
        ptr = (uint32_t *)&block[i];
        if (((*ptr & 0x1) == 0) && ((*ptr >> 1) >= (size + 2 * sizeof(uint32_t))))
        {
            creat_block(&block[i + size + 2 * sizeof(uint32_t)], (*ptr >> 1) - size - 2 * sizeof(uint32_t), 0);
            return (uint8_t *)ptr;
        }
        else
        {
            i += (*ptr >> 1) + 2 * sizeof(uint32_t);
        }
    }
    return NULL;
}

void print_malloc_info(void)
{
    uint32_t i, *ptr = NULL, block_cnt = 0, max_block = 0, size;

    ptr = (uint32_t *)&block[0];
    for (i = 0; i < MMALLOC_MAX_MEM;)
    {
        size = *ptr >> 1;
        if (size > max_block)
            max_block = size;
        printf("find block at %x, size %x, use %d\n", i, size, *ptr & 0x1);
        i += (size + 2 * sizeof(uint32_t));
        if (i > MMALLOC_MAX_MEM)
            break;
        ptr += (size / 4 + 2);
        block_cnt++;
    }
    printf("Total malloc space %dkb, total block %d\r\n", MMALLOC_MAX_MEM / 1024, block_cnt);
    printf("tmax block %dkb, used mem %dkb, avail mem %dkb\n\n", max_block / 1024, minfo.used / 1024, (MMALLOC_MAX_MEM - minfo.used) / 1024);
}

void memory_init(void)
{
    uint32_t len;

    len = MMALLOC_MAX_MEM - 2 * sizeof(uint32_t);
    creat_block(block, len, 0);
    minfo.used = 0;
}

void *mmalloc(uint32_t size)
{
    uint32_t avail_mem = MMALLOC_MAX_MEM - minfo.used;
    uint8_t *start = NULL;

    if (size % 4)
    {
        size += (4 - size % 4);
    }
    if (avail_mem < size)
    {
        printf("\tcan not mmalloc %d\r\n", size);
        print_malloc_info();
        return NULL;
    }
    start = find_useful_mem(size);
    if (start == NULL)
    {
        printf("\tcan not find useful block\r\n");
        return NULL;
    }
    minfo.used += size;
    return creat_block(start, size, 1);
}

void mfree(void *p)
{
    uint32_t *ptr = (uint32_t *)p, cur_len, last_len, next_len;
    uint8_t *base = (uint8_t *)p;

    //creat free block
    creat_block(base - sizeof(uint32_t), *(ptr - 1) >> 1, 0);
    cur_len = *(ptr - 1) >> 1;
    last_len = *(ptr - 2) >> 1;
    // combine cur & last block
    if (((base - 4) != block) && ((*(ptr - 2) & 0x1) == 0))
    {
        creat_block((uint8_t *)(ptr - 3 - last_len / 4), cur_len + last_len + 2 * sizeof(uint32_t), 0);
        minfo.used -= (last_len + 2 * sizeof(uint32_t));
        ptr -= (2 + last_len / 4);
        cur_len += (last_len + 2 * sizeof(uint32_t));
    }

    // combine cur & next block
    if ((*(ptr + cur_len / 4 + 1) & 0x1) == 0)
    {
        next_len = *(ptr + cur_len / 4 + 1) >> 1;
        creat_block((uint8_t *)(ptr - 1), cur_len + next_len + 2 * sizeof(uint32_t), 0);
        minfo.used -= (next_len + 2 * sizeof(uint32_t));
    }
}


```

- 代码使用一段0x6400000的数组，在实际操作中可以放到具体的物理地址上，并做好隔离
  
  - 数据结构只记录了已经使用的内存大小

- memory_init，初始化数组，制作第一个内存块，并置标志位为0，not used
  
  - creat_block，在给定的地址上写入size，flag，并return header后第一个地址

- mmalloc，做size检查，通过find_use_ful_men寻找到合适的块，并制作一个合适的块，返回分配内存块的payload区域地址
  
  - find_useful_mem试图寻找size匹配可用的内存块，并做分割，返回第一个可用的块

- mfree，做回收，并在每次回收时判断相邻内存块是否空闲，若空闲则合并，减少内存碎片

- print_malloc_info，打印每一个块的信息

最后贴一个测试代码

```c
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
```
