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
