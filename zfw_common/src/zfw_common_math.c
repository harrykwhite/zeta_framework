#include <zfw_common.h>

#include <string.h>

void zfw_init_rect(zfw_rect_t *const rect, const int x, const int y, const int width, const int height)
{
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
}

void zfw_init_rect_f(zfw_rect_f_t *const rect, const float x, const float y, const float width, const float height)
{
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
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
