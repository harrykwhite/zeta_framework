#include <zfw_common.h>

#include <stdlib.h>
#include <string.h>

zfw_bool_t zfw_init_mem_arena(zfw_mem_arena_t *const mem_arena, const int size)
{
    mem_arena->buf = malloc(size);
    mem_arena->buf_size = 0;
    mem_arena->buf_offs = 0;

    if (!mem_arena->buf)
    {
        return ZFW_FALSE;
    }

    mem_arena->buf_size = size;

    return ZFW_TRUE;
}

void *zfw_mem_arena_alloc(zfw_mem_arena_t *const mem_arena, const int size)
{
    if (mem_arena->buf_offs + size > mem_arena->buf_size)
    {
        zfw_log_error("Attempting to allocate %d bytes in a memory arena with only %d byte(s) remaining!", size, mem_arena->buf_size - mem_arena->buf_offs);
        return NULL;
    }

    mem_arena->buf_offs += size;
    mem_arena->buf_last_alloc_size = size;

    return (char *)mem_arena->buf + (mem_arena->buf_offs - size);
}

void zfw_reset_mem_arena(zfw_mem_arena_t *const mem_arena)
{
    mem_arena->buf_offs = 0;
    mem_arena->buf_last_alloc_size = 0;
}

void zfw_rewind_mem_arena(zfw_mem_arena_t *const mem_arena)
{
    mem_arena->buf_offs -= mem_arena->buf_last_alloc_size;
    mem_arena->buf_last_alloc_size = 0;
}

void zfw_clean_mem_arena(zfw_mem_arena_t *const mem_arena)
{
    free(mem_arena->buf);
    memset(mem_arena, 0, sizeof(*mem_arena));
}
