#include <zfw_common.h>

#include <string.h>
#include <math.h>

zfw_vec_2d_t zfw_create_vec_2d(const float x, const float y)
{
    const zfw_vec_2d_t vec = { x, y };
    return vec;
}

zfw_vec_2d_i_t zfw_create_vec_2d_i(const int x, const int y)
{
    const zfw_vec_2d_i_t vec = { x, y };
    return vec;
}

zfw_vec_2d_t zfw_get_vec_2d_sum(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b)
{
    return zfw_create_vec_2d(vec_a.x + vec_b.x, vec_a.y + vec_b.y);
}

zfw_vec_2d_t zfw_get_vec_2d_scaled(const zfw_vec_2d_t vec, const float scalar)
{
    return zfw_create_vec_2d(vec.x * scalar, vec.y * scalar);
}

zfw_vec_2d_i_t zfw_get_vec_2d_i_scaled(const zfw_vec_2d_i_t vec, const float scalar)
{
    return zfw_create_vec_2d_i((int)(vec.x * scalar), (int)(vec.y * scalar));
}

zfw_vec_2d_t zfw_get_vec_2d_cross_prod(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b)
{
    return zfw_create_vec_2d(vec_a.x * vec_b.x, vec_a.y * vec_b.y);
}

zfw_vec_2d_t zfw_get_vec_2d_normalized(const zfw_vec_2d_t vec)
{
    const float vec_mag = sqrt((vec.x * vec.x) + (vec.y * vec.y));
    return zfw_create_vec_2d(vec.x / vec_mag, vec.y / vec_mag);
}

float zfw_get_dist(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b)
{
    const float diff_hor = vec_b.x - vec_a.x;
    const float diff_ver = vec_b.y - vec_a.y;

    return sqrtf((diff_hor * diff_hor) + (diff_ver * diff_ver));
}

float zfw_get_dir(const zfw_vec_2d_t vec_a, const zfw_vec_2d_t vec_b)
{
    return atan2f(vec_b.y - vec_a.y, vec_b.x - vec_a.x);
}

void zfw_init_rect(zfw_rect_t *const rect, const int x, const int y, const int width, const int height)
{
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
}

void zfw_init_rect_f(zfw_rect_f_t *const rect_f, const float x, const float y, const float width, const float height)
{
    rect_f->x = x;
    rect_f->y = y;
    rect_f->width = width;
    rect_f->height = height;
}

zfw_vec_2d_i_t zfw_get_rect_pos(zfw_rect_t *const rect)
{
    return zfw_create_vec_2d_i(rect->x, rect->y);
}

zfw_vec_2d_t zfw_get_rect_f_pos(zfw_rect_f_t *const rect_f)
{
    return zfw_create_vec_2d(rect_f->x, rect_f->y);
}

zfw_vec_2d_i_t zfw_get_rect_size(zfw_rect_t *const rect)
{
    return zfw_create_vec_2d_i(rect->width, rect->height);
}

zfw_vec_2d_t zfw_get_rect_f_size(zfw_rect_f_t *const rect_f)
{
    return zfw_create_vec_2d(rect_f->width, rect_f->height);
}

zfw_bool_t zfw_is_vec_2d_in_rect(const zfw_vec_2d_t vec, const zfw_rect_t *const rect)
{
    return vec.x >= rect->x && vec.y >= rect->y && vec.x < rect->x + rect->width && vec.y < rect->y + rect->height;
}

zfw_bool_t zfw_do_rects_collide(const zfw_rect_t *const rect_a, const zfw_rect_t *const rect_b)
{
    return rect_a->x + rect_a->width >= rect_b->x && rect_a->y + rect_a->height >= rect_b->y && rect_a->x < rect_b->x + rect_b->width && rect_a->y < rect_b->y + rect_b->height;
}

zfw_bool_t zfw_do_rect_fs_collide(const zfw_rect_f_t *const rect_f_a, const zfw_rect_f_t *const rect_f_b)
{
    return rect_f_a->x + rect_f_a->width >= rect_f_b->x && rect_f_a->y + rect_f_a->height >= rect_f_b->y && rect_f_a->x < rect_f_b->x + rect_f_b->width && rect_f_a->y < rect_f_b->y + rect_f_b->height;
}

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const mat)
{
    memset(mat, 0, sizeof(*mat));

    for (int i = 0; i < 4; i++)
    {
        mat->elems[i][i] = 1.0f;
    }
}

void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const mat, const float left, const float right, const float bottom, const float top, const float near, const float far)
{
    memset(mat, 0, sizeof(*mat));

    mat->elems[0][0] = 2.0f / (right - left);
    mat->elems[1][1] = 2.0f / (top - bottom);
    mat->elems[2][2] = -2.0f / (far - near);
    mat->elems[3][0] = -(right + left) / (right - left);
    mat->elems[3][1] = -(top + bottom) / (top - bottom);
    mat->elems[3][2] = -(far + near) / (far - near);
    mat->elems[3][3] = 1.0f;
}
