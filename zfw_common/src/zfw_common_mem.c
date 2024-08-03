#include <zfw_common.h>

#include <stdlib.h>
#include <string.h>

zfw_bool_t zfw_init_mem_arena(zfw_mem_arena_t *const main_mem_arena, const int size)
{
    main_mem_arena->buf = malloc(size);
    main_mem_arena->buf_size = 0;
    main_mem_arena->buf_offs = 0;

    if (!main_mem_arena->buf)
    {
        return ZFW_FALSE;
    }

    main_mem_arena->buf_size = size;

    return ZFW_TRUE;
}

void *zfw_mem_arena_alloc(zfw_mem_arena_t *const main_mem_arena, const int size)
{
    if (main_mem_arena->buf_offs + size > main_mem_arena->buf_size)
    {
        zfw_log_error("Attempting to allocate %d bytes in a memory arena with only %d byte(s) remaining!",
                      size, main_mem_arena->buf_size - main_mem_arena->buf_offs);
        return NULL;
    }

    main_mem_arena->buf_offs += size;
    main_mem_arena->buf_last_alloc_size = size;

    return (char *)main_mem_arena->buf + (main_mem_arena->buf_offs - size);
}

void zfw_reset_mem_arena(zfw_mem_arena_t *const main_mem_arena)
{
    main_mem_arena->buf_offs = 0;
    main_mem_arena->buf_last_alloc_size = 0;
}

void zfw_rewind_mem_arena(zfw_mem_arena_t *const main_mem_arena)
{
    main_mem_arena->buf_offs -= main_mem_arena->buf_last_alloc_size;
    main_mem_arena->buf_last_alloc_size = 0;
}

void zfw_clean_mem_arena(zfw_mem_arena_t *const main_mem_arena)
{
    free(main_mem_arena->buf);
    memset(main_mem_arena, 0, sizeof(*main_mem_arena));
}

zfw_bool_t zfw_does_line_intersect_rect(const zfw_line_t *const line, const zfw_rect_t *const rect)
{
    return ZFW_TRUE;
}

