#include <zfw_utils.h>

int zfw_get_int_digit_count(const int n)
{
    if (n > -10 && n < 10)
    {
        return 1;
    }

    return 1 + zfw_get_int_digit_count(n / 10);
}

zfw_bool_t zfw_init_bitset(zfw_bitset_t *const bitset, const int bit_count, zfw_mem_arena_t *const main_mem_arena)
{
    bitset->byte_count = (bit_count / 8) + 1;
    bitset->bytes = zfw_mem_arena_alloc(main_mem_arena, bitset->byte_count);

    if (!bitset->bytes)
    {
        return ZFW_FALSE;
    }

    memset(bitset->bytes, 0, bitset->byte_count);

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
        if (bitset->bytes[i] != 0x00)
        {
            return ZFW_FALSE;
        }
    }

    return ZFW_TRUE;
}
