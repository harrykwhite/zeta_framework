#ifndef __ZFW_UTILS_H__
#define __ZFW_UTILS_H__

typedef struct
{
	unsigned char r, g, b, a;
}
zfw_color_t;

typedef struct
{
	unsigned char *bytes;
	int byte_count;
}
zfw_bitset_t;

inline zfw_color_t zfw_create_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	const zfw_color_t color = { r, g, b, a };
	return color;
}

int zfw_init_bitset(zfw_bitset_t *const bitset, const int byte_count);
void zfw_toggle_bitset_bit(zfw_bitset_t *const bitset, const int index, const int active);
void zfw_clean_bitset(zfw_bitset_t *const bitset);
int zfw_get_first_inactive_bit_index_in_bitset(const zfw_bitset_t *const bitset);
int zfw_is_bitset_fully_active(const zfw_bitset_t *const bitset);
int zfw_is_bitset_clear(const zfw_bitset_t *const bitset);

#endif
