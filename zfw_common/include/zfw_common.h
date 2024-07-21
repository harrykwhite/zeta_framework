#ifndef __ZFW_COMMON_H__
#define __ZFW_COMMON_H__

#define ZFW_FALSE 0
#define ZFW_TRUE 1

#define ZFW_SIZE_IN_BITS(X) (8 * sizeof(X))

#define ZFW_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

#define ZFW_MIN(X, Y) ((Y) < (X) ? (Y) : (X))
#define ZFW_MAX(X, Y) ((Y) > (X) ? (Y) : (X))

#define ZFW_PI 3.1415926535897932384626433832795028841971f

#define ZFW_RADS_AS_DEGS(X) ((X) * (180.0f / ZFW_PI))
#define ZFW_DEGS_AS_RADS(X) ((X) * (ZFW_PI / 180.0f))

#define ZFW_ASSETS_FILE_NAME "assets.zfwdat"
#define ZFW_TEX_CHANNEL_COUNT 4
#define ZFW_SHADER_SRC_BUF_SIZE 2048
#define ZFW_FONT_CHAR_RANGE_BEGIN 32
#define ZFW_FONT_CHAR_RANGE_SIZE 95
#define ZFW_FONT_TEX_CHANNEL_COUNT 4

typedef int zfw_bool_t; // IDEA: Make this an unsigned char?

typedef char font_char_hor_offs_t;
typedef short font_char_vert_offs_t;
typedef short font_char_hor_advance_t;
typedef short font_char_kerning_t;

typedef struct
{
    void *buf;
    int buf_size;
    int buf_offs;
    int buf_last_alloc_size; // The size of the most recent allocation, stored for rewinding functionality.
} zfw_mem_arena_t;

typedef struct
{
    float x, y;
} zfw_vec_2d_t;

typedef struct
{
    int x, y;
} zfw_vec_2d_int_t;

typedef struct
{
    int x, y;
    int width, height;
} zfw_rect_t;

typedef struct
{
    float x, y;
    float width, height;
} zfw_rect_f_t;

typedef struct
{
    float elems[4][4];
} zfw_matrix_4x4_t;

typedef struct
{
    unsigned short x, y;
    unsigned short width, height;
} font_char_src_rect_t;

void zfw_log(const char *const msg, ...);
void zfw_log_error(const char *const msg, ...);
void zfw_log_warning(const char *const msg, ...);

zfw_bool_t zfw_check_data_type_sizes();

zfw_bool_t zfw_init_mem_arena(zfw_mem_arena_t *const main_mem_arena, const int size);
void *zfw_mem_arena_alloc(zfw_mem_arena_t *const main_mem_arena, const int size);
void zfw_reset_mem_arena(zfw_mem_arena_t *const main_mem_arena);
void zfw_rewind_mem_arena(zfw_mem_arena_t *const main_mem_arena);
void zfw_clean_mem_arena(zfw_mem_arena_t *const main_mem_arena);

zfw_vec_2d_t zfw_create_vec_2d(const float x, const float y);
zfw_vec_2d_int_t zfw_create_vec_2d_i(const int x, const int y);
zfw_vec_2d_t zfw_get_vec_2d_sum(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
zfw_vec_2d_t zfw_get_vec_2d_scaled(const zfw_vec_2d_t vec, const float scalar);
zfw_vec_2d_int_t zfw_get_vec_2d_i_scaled(const zfw_vec_2d_int_t vec, const float scalar);
zfw_vec_2d_t zfw_get_vec_2d_cross_prod(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
float zfw_get_vec_2d_mag(const zfw_vec_2d_t vec);
zfw_vec_2d_t zfw_get_vec_2d_normalized(const zfw_vec_2d_t vec);
float zfw_get_dist(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
float zfw_get_dist_i(const zfw_vec_2d_int_t vec_a, const zfw_vec_2d_int_t vec_b);
float zfw_get_dir(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);

void zfw_init_rect(zfw_rect_t *const rect, const int x, const int y, const int width, const int height);
void zfw_init_rect_f(zfw_rect_f_t *const rect_f, const float x, const float y, const float width, const float height);
zfw_vec_2d_int_t zfw_get_rect_pos(zfw_rect_t *const rect);
zfw_vec_2d_t zfw_get_rect_f_pos(zfw_rect_f_t *const rect_f);
zfw_vec_2d_int_t zfw_get_rect_size(zfw_rect_t *const rect);
zfw_vec_2d_t zfw_get_rect_f_size(zfw_rect_f_t *const rect_f);
zfw_bool_t zfw_is_vec_2d_in_rect(const zfw_vec_2d_t vec, const zfw_rect_t *const rect);
zfw_bool_t zfw_do_rects_collide(const zfw_rect_t *const rect_a, const zfw_rect_t *const rect_b);
zfw_bool_t zfw_do_rect_fs_collide(const zfw_rect_f_t *const rect_f_a, const zfw_rect_f_t *const rect_f_b);

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const mat);
void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);

#endif
