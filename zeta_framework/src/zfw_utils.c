#include "zfw_utils.h"

#include <stdlib.h>
#include <string.h>
#include <zfw_common.h>

float zfw_gen_rand()
{
	return (float)rand() / RAND_MAX;
}

int zfw_init_bitset(zfw_bitset_t *const bitset, const int byte_count)
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

void zfw_clean_bitset(zfw_bitset_t *const bitset)
{
	free(bitset->bytes);
	bitset->bytes = NULL;

	bitset->byte_count = 0;
}

int zfw_get_first_inactive_bit_index_in_bitset(const zfw_bitset_t *const bitset)
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
