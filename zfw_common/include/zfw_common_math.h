#ifndef __ZFW_COMMON_MATH_H__
#define __ZFW_COMMON_MATH_H__

#include <math.h>

#define ZFW_PI 3.1415926535897932384626433832795028841971f

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

zfw_vec_2d_t zfw_create_vec_2d(const float x, const float y);
zfw_vec_2d_i_t zfw_create_vec_2d_i(const int x, const int y);
zfw_vec_2d_t zfw_get_vec_2d_sum(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
zfw_vec_2d_t zfw_get_vec_2d_scaled(const zfw_vec_2d_t vec, const float scalar);
float zfw_get_dist(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
float zfw_get_dir(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b);
int zfw_is_vec_2d_in_rect(const zfw_vec_2d_t vec, const zfw_rect_t *const rect);

int zfw_do_rects_collide(const zfw_rect_t *const rect_a, const zfw_rect_t *const rect_b);

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const mat);
void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const mat, const float left, const float right, const float bottom, const float top, const float near, const float far);
void scale_matrix_4x4(zfw_matrix_4x4_t *const mat, const float scale);

#endif
