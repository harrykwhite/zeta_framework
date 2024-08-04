#ifndef __ZFW_COMMON_MATH_H__
#define __ZFW_COMMON_MATH_H__

#include <math.h>
#include "zfw_common_misc.h"

#define ZFW_MIN(X, Y) ((Y) < (X) ? (Y) : (X))
#define ZFW_MAX(X, Y) ((Y) > (X) ? (Y) : (X))

#define ZFW_FLOOR(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)(X) : (int)((X) - 1)))
#define ZFW_CEIL(X) ((int)(X) == (X) ? (X) : ((X) > 0 ? (int)((X) + 1) : (int)(X)))

#define ZFW_CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))

#define ZFW_ABS(X) ((X) >= 0 ? (X) : -(X))

#define ZFW_PI 3.1415926535897932384626433832795028841971f

#define ZFW_RADS_AS_DEGS(X) ((X) * (180.0f / ZFW_PI))
#define ZFW_DEGS_AS_RADS(X) ((X) * (ZFW_PI / 180.0f))

typedef enum
{
    ZFW_ORIENTATION_ID__COLLINEAR,
    ZFW_ORIENTATION_ID__CLOCKWISE,
    ZFW_ORIENTATION_ID__COUNTERCLOCKWISE
} zfw_orientation_id_t;

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

