#include <zfw_common_bits.h>

#include <zfw_common_math.h>

int zfw_get_index_of_first_bit_with_activity_state(const zfw_bits_t *const bits, const int bit_count, const zfw_bool_t active)
{
    for (int i = 0; i < bit_count; i++)
    {
        const zfw_bits_t bitmask = 1 << (i % 8);

        if (((bits[i / 8] & bitmask) != 0) == active)
        {
            return i;
        }
    }

    return -1;
}

zfw_bool_t zfw_init_bitset_in_mem_arena(zfw_bitset_t *const bitset, const int bit_count, zfw_mem_arena_t *const mem_arena)
{
    const int byte_count = ZFW_CEIL(bit_count / 8.0f);

    bitset->bytes = zfw_mem_arena_alloc(mem_arena, byte_count);

    if (!bitset->bytes)
    {
        return ZFW_FALSE;
    }

    memset(bitset->bytes, 0, byte_count);

    bitset->byte_count = byte_count;

    return ZFW_TRUE;
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

int zfw_get_first_inactive_bitset_bit_index_in_range(const zfw_bitset_t *const bitset, const int begin_bit_ind, const int end_bit_ind)
{
    const int begin_byte_index = begin_bit_ind / 8;
    const int end_byte_index = (end_bit_ind / 8) + 1;

    for (int i = begin_byte_index; i < end_byte_index; i++)
    {
        int begin_byte_bit_index = 0;

        if (i == begin_byte_index)
        {
            begin_byte_bit_index = begin_bit_ind % 8;
        }

        int end_byte_bit_index = 8;

        if (i == end_byte_index - 1)
        {
            end_byte_bit_index = end_bit_ind % 8;
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

zfw_bool_t zfw_is_bitset_fully_active(const zfw_bitset_t *const bitset)
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

zfw_bool_t zfw_is_bitset_clear(const zfw_bitset_t *const bitset)
{
    for (int i = 0; i < bitset->byte_count; i++)
    {
        if (bitset->bytes[i])
        {
            return ZFW_FALSE;
        }
    }

    return ZFW_TRUE;
}
