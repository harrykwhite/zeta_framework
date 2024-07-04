#include <stdlib.h>
#include <string.h>
#include "zfw.h"

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
	mem_arena->buf_alloc_size_last = size;

	return (char *)mem_arena->buf + (mem_arena->buf_offs - size);
}

void zfw_reset_mem_arena(zfw_mem_arena_t *const mem_arena)
{
	mem_arena->buf_offs = 0;
	mem_arena->buf_alloc_size_last = 0;
}

void zfw_rewind_mem_arena(zfw_mem_arena_t *const mem_arena)
{
	mem_arena->buf_offs -= mem_arena->buf_alloc_size_last;
	mem_arena->buf_alloc_size_last = 0;
}

void zfw_clean_mem_arena(zfw_mem_arena_t *const mem_arena)
{
	free(mem_arena->buf);
	memset(mem_arena, 0, sizeof(*mem_arena));
}

zfw_bool_t zfw_init_bitset(zfw_bitset_t *const bitset, const int byte_count)
{
	bitset->bytes = malloc(byte_count);
	bitset->byte_count = byte_count;

	if (!bitset->bytes)
	{
		return ZFW_FALSE;
	}

	memset(bitset->bytes, 0, byte_count);

	return ZFW_TRUE;
}

void zfw_toggle_bitset_bit(zfw_bitset_t *const bitset, const int index, const int active)
{
	unsigned char bitmask = ((unsigned char)1 << (index % 8));
	const int byte_ind = index / 8;

	if (active)
	{
		bitset->bytes[byte_ind] |= bitmask;
	}
	else
	{
		bitset->bytes[byte_ind] &= ~bitmask;
	}
}

void zfw_clear_bitset(zfw_bitset_t *const bitset)
{
	for (int i = 0; i < bitset->byte_count; i++)
	{
		bitset->bytes[i] = 0x00;
	}
}

void zfw_clean_bitset(zfw_bitset_t *const bitset)
{
	free(bitset->bytes);
	bitset->bytes = NULL;

	bitset->byte_count = 0;
}

int zfw_get_first_inactive_bitset_bit_index(const zfw_bitset_t *const bitset)
{
	for (int i = 0; i < bitset->byte_count; i++)
	{
		if (bitset->bytes[i] != 0xFF)
		{
			for (int j = 0; j < 8; j++)
			{
				if (!(bitset->bytes[i] & ((unsigned char)1 << j)))
				{
					return (8 * i) + j;
				}
			}
		}
	}

	return -1;
}

int zfw_get_first_inactive_bitset_bit_index_in_range(const zfw_bitset_t *const bitset, const int begin_index, const int end_index)
{
	const int begin_byte_index = begin_index / 8;
	const int end_byte_index = (end_index / 8) + 1;

	for (int i = begin_byte_index; i < end_byte_index; i++)
	{
		int begin_byte_bit_index = 0;

		if (i == begin_byte_index)
		{
			begin_byte_bit_index = begin_index % 8;
		}

		int end_byte_bit_index = 8;

		if (i == end_byte_index - 1)
		{
			end_byte_bit_index = end_index % 8;
		}

		for (int j = begin_byte_bit_index; j < end_byte_bit_index; j++)
		{
			if (!(bitset->bytes[i] & ((unsigned char)1 << j)))
			{
				return (8 * i) + j;
			}
		}
	}

	return -1;
}

int zfw_is_bitset_fully_active(const zfw_bitset_t *const bitset)
{
	for (int i = 0; i < bitset->byte_count; i++)
	{
		if (bitset->bytes[i] != 0xFF)
		{
			return ZFW_FALSE;
		}
	}

	return ZFW_TRUE;
}

int zfw_is_bitset_clear(const zfw_bitset_t *const bitset)
{
	for (int i = 0; i < bitset->byte_count; i++)
	{
		if (bitset->bytes[i] != 0x00)
		{
			return ZFW_FALSE;
		}
	}

	return ZFW_TRUE;
}

float zfw_gen_rand_num()
{
	return (float)rand() / RAND_MAX;
}
