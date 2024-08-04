#ifndef __ZFW_COMMON_MEM_H__
#define __ZFW_COMMON_MEM_H__

#include "zfw_common_misc.h"

#define ZFW_SIZE_IN_BITS(X) (8 * sizeof(X))
#define ZFW_BIT_COUNT_AS_BYTE_COUNT(X) (int)ZFW_CEIL((X) / 8.0f)

typedef struct
{
    void *buf;
    int buf_size;
    int buf_offs;
    int buf_last_alloc_size; // The size of the most recent allocation, stored for rewinding functionality.
} zfw_mem_arena_t;

zfw_bool_t zfw_init_mem_arena(zfw_mem_arena_t *const main_mem_arena, const int size);
void *zfw_mem_arena_alloc(zfw_mem_arena_t *const main_mem_arena, const int size);
void zfw_reset_mem_arena(zfw_mem_arena_t *const main_mem_arena);
void zfw_rewind_mem_arena(zfw_mem_arena_t *const main_mem_arena);
void zfw_clean_mem_arena(zfw_mem_arena_t *const main_mem_arena);

#endif

