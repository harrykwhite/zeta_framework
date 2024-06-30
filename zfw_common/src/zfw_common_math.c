#include "zfw_common_math.h"

#include <string.h>

zfw_vec_t zfw_create_vec(const float x, const float y)
{
	const zfw_vec_t vec = { x, y };
	return vec;
}

zfw_vec_i_t zfw_create_vec_i(const int x, const int y)
{
	const zfw_vec_i_t vec = { x, y };
	return vec;
}

zfw_vec_t zfw_get_vec_sum(const zfw_vec_t vec_a, const zfw_vec_t vec_b)
{
	const zfw_vec_t vec_sum = { vec_a.x + vec_b.x, vec_a.y + vec_b.y };
	return vec_sum;
}

zfw_vec_t zfw_get_vec_scaled(const zfw_vec_t vec, const float scalar)
{
	const zfw_vec_t vec_scaled = { vec.x * scalar, vec.y * scalar };
	return vec_scaled;
}

float zfw_get_distance(const zfw_vec_t vec_a, const zfw_vec_t vec_b)
{
	const float diff_hor = vec_b.x - vec_a.x;
	const float diff_ver = vec_b.y - vec_a.y;

	return sqrtf((diff_hor * diff_hor) + (diff_ver * diff_ver));
}

float zfw_get_direction(const zfw_vec_t vec_a, const zfw_vec_t vec_b)
{
	return atan2f(vec_b.y - vec_a.y, vec_b.x - vec_a.x);
}

int zfw_is_vec_in_rect(const zfw_vec_t vec, const zfw_rect_t *const rect)
{
	return vec.x >= rect->x && vec.y >= rect->y && vec.x < rect->x + rect->width && vec.y < rect->y + rect->height;
}

int zfw_do_rectangles_collide(const zfw_rect_t *const rect_a, const zfw_rect_t *const rect_b)
{
	return rect_a->x + rect_a->width >= rect_b->x && rect_a->y + rect_a->height >= rect_b->y && rect_a->x < rect_b->x + rect_b->width && rect_a->y < rect_b->y + rect_b->height;
}

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const matrix)
{
	memset(matrix, 0, sizeof(*matrix));

	for (int i = 0; i < 4; i++)
	{
		matrix->elems[i][i] = 1.0f;
	}
}

void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const matrix, const float left, const float right, const float bottom, const float top, const float near, const float far)
{
	memset(matrix, 0, sizeof(*matrix));

	matrix->elems[0][0] = 2.0f / (right - left);
	matrix->elems[1][1] = 2.0f / (top - bottom);
	matrix->elems[2][2] = -2.0f / (far - near);
	matrix->elems[3][0] = -(right + left) / (right - left);
	matrix->elems[3][1] = -(top + bottom) / (top - bottom);
	matrix->elems[3][2] = -(far + near) / (far - near);
	matrix->elems[3][3] = 1.0f;
}

void scale_matrix_4x4(zfw_matrix_4x4_t *const matrix, const float scale)
{
	for (int i = 0; i < 3; i++)
	{
		matrix->elems[i][i] *= scale;
	}
}
