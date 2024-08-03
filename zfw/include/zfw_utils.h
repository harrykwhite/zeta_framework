#ifndef __ZFW_UTILS_H__
#define __ZFW_UTILS_H__

#include <stdlib.h>
#include <string.h>
#include <zfw_common.h>

#define ZFW_FLOOR(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)(X) : (int)((X) - 1)))
#define ZFW_CEIL(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)((X) + 1) : (int)(X)))

#define ZFW_CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))

#define ZFW_BIT_COUNT_AS_BYTE_COUNT(X) (int)ZFW_CEIL((X) / 8.0f)

typedef unsigned char zfw_bits_t;

typedef struct
{
    unsigned char *bytes;
    int byte_count;
} zfw_bitset_t;

int zfw_get_int_digit_count(const int n);

int zfw_get_index_of_first_bit_with_activity_state(const zfw_bits_t *const bits, const int bit_count,
                                                   const zfw_bool_t active);

zfw_bool_t zfw_init_bitset(zfw_bitset_t *const bitset, const int bit_count, zfw_mem_arena_t *const mem_arena);
int zfw_get_first_inactive_bitset_bit_index(const zfw_bitset_t *const bitset);
int zfw_get_first_inactive_bitset_bit_index_in_range(const zfw_bitset_t *const bitset, const int begin_bit_index,
                                                     const int end_bit_index);
zfw_bool_t zfw_is_bitset_fully_active(const zfw_bitset_t *const bitset);
zfw_bool_t zfw_is_bitset_clear(const zfw_bitset_t *const bitset);

// Generates a random float number between 0 and 1 inclusive.
inline float zfw_gen_rand_num()
{
    return (float)rand() / RAND_MAX;
}

inline float zfw_gen_rand_num_in_range(const float min, const float max)
{
    return min + ((max - min) * zfw_gen_rand_num());
}

inline void zfw_activate_bit(const int bit_index, zfw_bits_t *const bits)
{
    bits[bit_index / 8] |= 1 << (bit_index % 8);
}

inline void zfw_deactivate_bit(const int bit_index, zfw_bits_t *const bits)
{
    bits[bit_index / 8] &= ~(1 << (bit_index % 8));
}

inline zfw_bool_t zfw_is_bit_active(const int bit_index, const zfw_bits_t *const bits)
{
    const zfw_bits_t bitmask = 1 << (bit_index % 8);
    return (bits[bit_index / 8] & bitmask) != 0;
}

inline void zfw_activate_bitset_bit(zfw_bitset_t *const bitset, const int bit_index)
{
    bitset->bytes[bit_index / 8] |= (unsigned char)1 << (bit_index % 8);
}

inline void zfw_deactivate_bitset_bit(zfw_bitset_t *const bitset, const int bit_index)
{
    bitset->bytes[bit_index / 8] &= ~((unsigned char)1 << (bit_index % 8));
}

inline void zfw_clear_bitset(zfw_bitset_t *const bitset)
{
    memset(bitset->bytes, 0, bitset->byte_count);
}

inline zfw_bool_t zfw_is_bitset_bit_active(const zfw_bitset_t *const bitset, const int bit_index)
{
    return (bitset->bytes[bit_index / 8] & ((unsigned char)1 << (bit_index % 8))) != 0;
}

#endif

