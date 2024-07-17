#include <zfw.h>

#include <stdlib.h>
#include <string.h>

const zfw_color_t zfw_g_builtin_color_white = { 1.0f, 1.0f, 1.0f, 1.0f };
const zfw_color_t zfw_g_builtin_color_black = { 0.0f, 0.0f, 0.0f, 1.0f };
const zfw_color_t zfw_g_builtin_color_red = { 1.0f, 0.0f, 0.0f, 1.0f };
const zfw_color_t zfw_g_builtin_color_green = { 0.0f, 1.0f, 0.0f, 1.0f };
const zfw_color_t zfw_g_builtin_color_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
const zfw_color_t zfw_g_builtin_color_yellow = { 1.0f, 1.0f, 0.0f, 1.0f };

float zfw_get_clamped_num(const float num, const float min, const float max)
{
    if (num < min)
    {
        return min;
    }

    if (num > max)
    {
        return max;
    }

    return num;
}

int zfw_get_int_digit_count(const int i)
{
    if (i > -10 && i < 10)
    {
        return 1;
    }

    return 1 + zfw_get_int_digit_count(i / 10);
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

void zfw_toggle_bitset_bit(zfw_bitset_t *const bitset, const int bit_index, const int bit_active)
{
    unsigned char bitmask = ((unsigned char)1 << (bit_index % 8));
    const int byte_ind = bit_index / 8;

    if (bit_active)
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
    memset(bitset->bytes, 0, bitset->byte_count);
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

int zfw_get_first_inactive_bitset_bit_index_in_range(const zfw_bitset_t *const bitset, const int begin_bit_index, const int end_bit_index)
{
    const int begin_byte_index = begin_bit_index / 8;
    const int end_byte_index = (end_bit_index / 8) + 1;

    for (int i = begin_byte_index; i < end_byte_index; i++)
    {
        int begin_byte_bit_index = 0;

        if (i == begin_byte_index)
        {
            begin_byte_bit_index = begin_bit_index % 8;
        }

        int end_byte_bit_index = 8;

        if (i == end_byte_index - 1)
        {
            end_byte_bit_index = end_bit_index % 8;
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

zfw_bool_t zfw_is_bitset_bit_active(const zfw_bitset_t *const bitset, const int bit_index)
{
    return (bitset->bytes[bit_index / 8] & ((unsigned char)1 << (bit_index % 8))) != 0;
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

void zfw_init_color(zfw_color_t *const color, const float r, const float g, const float b, const float a)
{
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;
}

float zfw_gen_rand_num()
{
    return (float)rand() / RAND_MAX;
}

float zfw_gen_rand_num_in_range(const float min, const float max)
{
    return min + ((max - min) * zfw_gen_rand_num());
}
