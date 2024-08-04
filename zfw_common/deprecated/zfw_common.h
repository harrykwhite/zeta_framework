#if 0

#ifndef __ZFW_COMMON_H__
#define __ZFW_COMMON_H__

#include <math.h>

#define ZFW_FALSE 0
#define ZFW_TRUE 1

#define ZFW_SIZE_IN_BITS(X) (8 * sizeof(X))
#define ZFW_BIT_COUNT_AS_BYTE_COUNT(X) (int)ZFW_CEIL((X) / 8.0f)

#define ZFW_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

#define ZFW_MIN(X, Y) ((Y) < (X) ? (Y) : (X))
#define ZFW_MAX(X, Y) ((Y) > (X) ? (Y) : (X))

#define ZFW_FLOOR(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)(X) : (int)((X) - 1)))
#define ZFW_CEIL(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)((X) + 1) : (int)(X)))

#define ZFW_CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))

#define ZFW_ABS(X) ((X) >= 0 ? (X) : -(X))

#define ZFW_PI 3.1415926535897932384626433832795028841971f

#define ZFW_RADS_AS_DEGS(X) ((X) * (180.0f / ZFW_PI))
#define ZFW_DEGS_AS_RADS(X) ((X) * (ZFW_PI / 180.0f))

#define ZFW_ASSETS_FILE_NAME "assets.zfwdat"
#define ZFW_TEX_CHANNEL_COUNT 4
#define ZFW_SHADER_SRC_BUF_SIZE 2048
#define ZFW_FONT_CHAR_RANGE_BEGIN 32
#define ZFW_FONT_CHAR_RANGE_SIZE 95
#define ZFW_FONT_TEX_CHANNEL_COUNT 4

typedef enum
{
    ZFW_ORIENTATION_ID__COLLINEAR,
    ZFW_ORIENTATION_ID__CLOCKWISE,
    ZFW_ORIENTATION_ID__COUNTERCLOCKWISE
} zfw_orientation_id_t;

typedef int zfw_bool_t; // IDEA: Make this an unsigned char?

typedef unsigned char zfw_bits_t;

typedef char font_char_hor_offs_t;
typedef short font_char_vert_offs_t;
typedef short font_char_hor_advance_t;
typedef short font_char_kerning_t;

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

typedef struct
{
    float x, y;
} zfw_vec_2d_t;

typedef struct
{
    int x, y;
} zfw_vec_2d_i_t;

typedef struct
{
    zfw_vec_2d_t a;
    zfw_vec_2d_t b;
} zfw_line_t;

typedef struct
{
    int x, y, w, h;
} zfw_rect_t;

typedef struct
{
    float x, y, w, h;
} zfw_rect_f_t;

typedef struct
{
    int first_pt_index;
    int pt_count;
} zfw_poly_t;

typedef struct
{
    float elems[4][4];
} zfw_matrix_4x4_t;

typedef struct
{
    unsigned short x, y, w, h;
} font_char_src_rect_t;


zfw_orientation_id_t zfw_get_orientation(const zfw_vec_2d_t a, const zfw_vec_2d_t b, const zfw_vec_2d_t c);

void zfw_init_line(zfw_line_t *const line, const zfw_vec_2d_t pos, const float len, const float dir);
void zfw_init_line_rect_f(zfw_rect_f_t *const rect, const zfw_line_t *const line);
zfw_bool_t zfw_is_pt_in_line_rect(const zfw_vec_2d_t pt, const zfw_line_t *const line);
zfw_bool_t zfw_do_lines_inters(const zfw_line_t *const l1, const zfw_line_t *const l2);
zfw_bool_t zfw_does_line_inters_rect_f(const zfw_line_t *const line, const zfw_rect_f_t *const rect);

void zfw_init_rect(zfw_rect_t *const rect, const int x, const int y, const int w, const int h);
void zfw_init_rect_f(zfw_rect_f_t *const rect, const float x, const float y, const float width, const float height);
void zfw_get_rect_f_edges(const zfw_rect_f_t *const rect, zfw_line_t edges[4]);

#define POLY_PT_LIMIT 1024

zfw_vec_2d_t g_poly_pts[POLY_PT_LIMIT];
zfw_bits_t g_poly_pt_activity[ZFW_BIT_COUNT_AS_BYTE_COUNT(POLY_PT_LIMIT)];

void gen_poly()
{
}

zfw_bool_t is_pt_in_poly(const zfw_vec_2d_t pt, const zfw_poly_t poly)
{
    zfw_bool_t in_poly = ZFW_FALSE;

    for (int i = 0; i < poly.pt_count; ++i)
    {
        const zfw_line_t line = {
            g_poly_pts[poly.first_pt_index + i],
            g_poly_pts[poly.first_pt_index + ((i + 1) % poly.pt_count)]
        };

        const float line_y_min = ZFW_MIN(line.a.y, line.b.y);

        if (pt.y < line_y_min)
        {
            continue;
        }

        const float line_y_max = ZFW_MAX(line.a.y, line.b.y);
        
        if (pt.y > line_y_max)
        {
            continue;
        }

        const float line_x_min = ZFW_MIN(line.a.x, line.b.x);
        const float line_width = ZFW_ABS(line.a.x - line.b.x);

        const float line_inters_y_perc = (pt.y - line_y_min) * (line_y_max - line_y_min);
        const float line_inters_x = line_x_min + (line_inters_y_perc * line_width);

        if (pt.x < line_inters_y_perc)
        {
            in_poly = !in_poly;
        }
    }

    return in_poly;
}

zfw_bool_t do_polys_inters(const zfw_poly_t poly_a, const zfw_poly_t poly_b)
{
    // Check whether any of the points of the first polygon are inside the second.
    for (int i = 0; i < poly_a.pt_count; ++i)
    {
        if (is_pt_in_poly(g_poly_pts[poly_a.first_pt_index + i], poly_b))
        {
            return ZFW_TRUE;
        }
    }

    // Do the same but for the second polygon.
    for (int i = 0; i < poly_b.pt_count; ++i)
    {
        if (is_pt_in_poly(g_poly_pts[poly_b.first_pt_index + i], poly_a))
        {
            return ZFW_TRUE;
        }
    }
    //

    return ZFW_FALSE;
}

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const mat);
void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const mat, const float left, const float right, const float bottom,
                               const float top,const float near, const float far);

float zfw_get_angle_diff(const float a, const float b);

inline zfw_vec_2d_t zfw_create_vec_2d(const float x, const float y)
{
    const zfw_vec_2d_t vec = {x, y};
    return vec;
}

inline zfw_vec_2d_i_t zfw_create_vec_2d_i(const int x, const int y)
{
    const zfw_vec_2d_i_t vec = {x, y};
    return vec;
}

inline zfw_vec_2d_t zfw_get_vec_2d_sum(const zfw_vec_2d_t v1, const zfw_vec_2d_t v2)
{
    return zfw_create_vec_2d(v1.x + v2.x, v1.y + v2.y);
}

inline zfw_vec_2d_i_t zfw_get_vec_2d_i_sum(const zfw_vec_2d_i_t v1, const zfw_vec_2d_i_t v2)
{
    return zfw_create_vec_2d_i(v1.x + v2.x, v1.y + v2.y);
}

inline zfw_vec_2d_t zfw_get_vec_2d_diff(const zfw_vec_2d_t v1, const zfw_vec_2d_t v2)
{
    return zfw_create_vec_2d(v1.x - v2.x, v1.y - v2.y);
}

inline zfw_vec_2d_t zfw_get_vec_2d_scaled(const zfw_vec_2d_t vec, const float scalar)
{
    return zfw_create_vec_2d(vec.x * scalar, vec.y * scalar);
}

inline zfw_vec_2d_i_t zfw_get_veci_2d_scaled(const zfw_vec_2d_i_t vec, const float scalar)
{
    return zfw_create_vec_2d_i((int)(vec.x * scalar), (int)(vec.y * scalar));
}

inline float zfw_get_vec_2d_cross_prod(const zfw_vec_2d_t v1, const zfw_vec_2d_t v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}

inline float zfw_get_vec_2d_mag(const zfw_vec_2d_t vec)
{
    return sqrt((vec.x * vec.x) + (vec.y * vec.y));
}

inline zfw_vec_2d_t zfw_get_vec_2d_normalized(const zfw_vec_2d_t v)
{
    const float vm = zfw_get_vec_2d_mag(v);
    return zfw_create_vec_2d(v.x / vm, v.y / vm);
}

inline float zfw_get_dist(const zfw_vec_2d_t v1, const zfw_vec_2d_t v2)
{
    const float diff_hor = v2.x - v1.x;
    const float diff_ver = v2.y - v1.y;

    return sqrtf((diff_hor * diff_hor) + (diff_ver * diff_ver));
}

inline float zfw_get_dist_i(const zfw_vec_2d_i_t v1, const zfw_vec_2d_i_t v2)
{
    const int diff_hor = v2.x - v1.x;
    const int diff_ver = v2.y - v1.y;

    return sqrtf((diff_hor * diff_hor) + (diff_ver * diff_ver));
}

inline float zfw_get_dir(const zfw_vec_2d_t v1, const zfw_vec_2d_t v2)
{
    return atan2f(-(v2.y - v1.y), v2.x - v1.x);
}

inline float zfw_get_line_gradient(const zfw_line_t *const line)
{
    return (line->b.y - line->a.y) / (line->b.x - line->a.x);
}

inline float zfw_get_line_y_intercept(const zfw_line_t *const line)
{
    return line->a.y - (zfw_get_line_gradient(line) * line->a.x);
}

inline zfw_vec_2d_i_t zfw_get_rect_pos(const zfw_rect_t *const rect)
{
    return zfw_create_vec_2d_i(rect->x, rect->y);
}

inline void zfw_set_rect_pos(zfw_rect_t *const rect, const zfw_vec_2d_i_t pos)
{
    rect->x = pos.x;
    rect->y = pos.y;
}

inline zfw_vec_2d_t zfw_get_rect_f_pos(const zfw_rect_f_t *const rect)
{
    return zfw_create_vec_2d(rect->x, rect->y);
}

inline void zfw_set_rect_f_pos(zfw_rect_f_t *const rect, const zfw_vec_2d_t pos)
{
    rect->x = pos.x;
    rect->y = pos.y;
}

inline zfw_vec_2d_i_t zfw_get_rect_size(const zfw_rect_t *const rect)
{
    return zfw_create_vec_2d_i(rect->w, rect->h);
}

inline void zfw_set_rect_size(zfw_rect_t *const rect, const zfw_vec_2d_i_t size)
{
    rect->w = size.x;
    rect->h = size.y;
}

inline zfw_vec_2d_t zfw_get_rect_f_size(const zfw_rect_f_t *const rect)
{
    return zfw_create_vec_2d(rect->w, rect->h);
}

inline void zfw_set_rect_f_size(zfw_rect_f_t *const rect, const zfw_vec_2d_t size)
{
    rect->w = size.x;
    rect->h = size.y;
}

inline zfw_vec_2d_i_t zfw_get_rect_center(const zfw_rect_f_t *const rect)
{
    return zfw_create_vec_2d_i(rect->x + (rect->w / 2), rect->y + (rect->h / 2));
}

inline zfw_vec_2d_t zfw_get_rect_f_center(const zfw_rect_f_t *const rect)
{
    return zfw_create_vec_2d(rect->x + (rect->w / 2.0f), rect->y + (rect->h / 2.0f));
}

inline zfw_bool_t zfw_is_vec_2d_in_rect(const zfw_vec_2d_t vec, const zfw_rect_t *const rect)
{
    return vec.x >= rect->x && vec.y >= rect->y && vec.x < rect->x + rect->w && vec.y < rect->y + rect->h;
}

inline zfw_bool_t zfw_is_vec_2d_in_rect_f(const zfw_vec_2d_t vec, const zfw_rect_f_t *const rect)
{
    return vec.x >= rect->x && vec.y >= rect->y && vec.x < rect->x + rect->w && vec.y < rect->y + rect->h;
}

inline zfw_bool_t zfw_do_rects_collide(const zfw_rect_t *const r1, const zfw_rect_t *const r2)
{
    return r1->x + r1->w > r2->x
        && r1->y + r1->h > r2->y
        && r1->x < r2->x + r2->w
        && r1->y < r2->y + r2->h;
}

inline zfw_bool_t zfw_do_rect_fs_collide(const zfw_rect_f_t *const r1, const zfw_rect_f_t *const r2)
{
    return r1->x + r1->w > r2->x
        && r1->y + r1->h > r2->y
        && r1->x < r2->x + r2->w
        && r1->y < r2->y + r2->h;
}

#endif

#endif

