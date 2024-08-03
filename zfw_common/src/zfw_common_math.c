#include <zfw_common.h>

#include <string.h>

float zfw_get_angle_diff(const float a, const float b)
{
    float diff = a - b;

    while (diff < -ZFW_PI)
    {
        diff += ZFW_PI * 2.0f;
    }

    while (diff > ZFW_PI)
    {
        diff -= ZFW_PI * 2.0f;
    }

    return diff;
}

zfw_orientation_id_t zfw_get_orientation(const zfw_vec_2d_t a, const zfw_vec_2d_t b, const zfw_vec_2d_t c)
{
    const zfw_vec_2d_t ab = zfw_get_vec_2d_diff(a, b);
    const zfw_vec_2d_t bc = zfw_get_vec_2d_diff(b, c);

    // Let the area of the parallelogram of vectors ab and bc indicate the orientation.
    const float cross = zfw_get_vec_2d_cross_prod(bc, ab);

    if (cross == 0.0f)
    {
        return ZFW_ORIENTATION_ID__COLLINEAR;
    }

    return cross > 0.0f ? ZFW_ORIENTATION_ID__CLOCKWISE : ZFW_ORIENTATION_ID__COUNTERCLOCKWISE;
}

void zfw_init_line(zfw_line_t *const line, const zfw_vec_2d_t pos, const float len, const float dir)
{
    line->a = pos;

    line->b.x = line->a.x + (len * cosf(dir));
    line->b.y = line->a.y + (len * -sinf(dir));
}

void zfw_init_line_rect_f(zfw_rect_f_t *const rect, const zfw_line_t *const line)
{
    if (line->a.x <= line->b.x)
    {
        rect->x = line->a.x;
        rect->w = line->b.x - line->a.x;
    }
    else
    {
        rect->x = line->b.x;
        rect->w = line->a.x - line->b.x;
    }

    if (line->a.y <= line->b.y)
    {
        rect->y = line->a.y;
        rect->h = line->b.y - line->a.y;
    }
    else
    {
        rect->y = line->b.y;
        rect->h = line->a.y - line->b.y;
    }
}

zfw_bool_t zfw_is_pt_in_line_rect(const zfw_vec_2d_t pt, const zfw_line_t *const line)
{
    const float line_x_min = ZFW_MIN(line->a.x, line->b.x);
    const float line_y_min = ZFW_MIN(line->a.y, line->b.y);

    const float line_x_max = ZFW_MAX(line->a.x, line->b.x);
    const float line_y_max = ZFW_MAX(line->a.y, line->b.y);

    return pt.x >= line_x_min && pt.y >= line_y_min && pt.x < line_x_max && pt.y < line_y_max;
}

zfw_bool_t zfw_do_lines_inters(const zfw_line_t *const l1, const zfw_line_t *const l2)
{
    const zfw_orientation_id_t o_l1a_l1b_l2a = zfw_get_orientation(l1->a, l1->b, l2->a);
    const zfw_orientation_id_t o_l1a_l1b_l2b = zfw_get_orientation(l1->a, l1->b, l2->b);

    const zfw_orientation_id_t o_l2a_l2b_l1a = zfw_get_orientation(l2->a, l2->b, l1->a);
    const zfw_orientation_id_t o_l2a_l2b_l1b = zfw_get_orientation(l2->a, l2->b, l1->b);

    // If there are differences in orientation, the lines must be intersecting.
    if (o_l1a_l1b_l2a != o_l1a_l1b_l2b && o_l2a_l2b_l1a != o_l2a_l2b_l1b)
    {
        return ZFW_TRUE;
    }

    // Check if l2a resides along l1.
    if (o_l1a_l1b_l2a == ZFW_ORIENTATION_ID__COLLINEAR && zfw_is_pt_in_line_rect(l2->a, l1))
        return ZFW_TRUE;

    // Check if l2b resides along l1.
    if (o_l1a_l1b_l2a == ZFW_ORIENTATION_ID__COLLINEAR && zfw_is_pt_in_line_rect(l2->b, l1))
        return ZFW_TRUE;

    // Check if l1a resides along l2.
    if (o_l1a_l1b_l2a == ZFW_ORIENTATION_ID__COLLINEAR && zfw_is_pt_in_line_rect(l1->a, l2))
        return ZFW_TRUE;

    // Check if l1b resides along l2.
    if (o_l1a_l1b_l2a == ZFW_ORIENTATION_ID__COLLINEAR && zfw_is_pt_in_line_rect(l1->a, l2))
        return ZFW_TRUE;
    //

    return ZFW_FALSE;
}

zfw_bool_t zfw_does_line_inters_rect_f(const zfw_line_t *const line, const zfw_rect_f_t *const rect)
{
    // Check if the line intersects any of the edges of the rectangle.
    zfw_line_t rect_edges[4];
    zfw_get_rect_f_edges(rect, rect_edges);

    for (int i = 0; i < 4; ++i)
    {
        if (zfw_do_lines_inters(line, &rect_edges[i]))
        {
            return ZFW_TRUE;
        }
    }

    return ZFW_FALSE;
}

void zfw_init_rect(zfw_rect_t *const rect, const int x, const int y, const int width, const int height)
{
    rect->x = x;
    rect->y = y;
    rect->w = width;
    rect->h = height;
}

void zfw_init_rect_f(zfw_rect_f_t *const rect, const float x, const float y, const float w, const float height)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = height;
}

void zfw_get_rect_f_edges(const zfw_rect_f_t *const rect, zfw_line_t edges[4])
{
    // Go around clockwise starting from the top-left.
    edges[0].a.x = rect->x;
    edges[0].a.y = rect->y;
    edges[0].b.x = rect->x + rect->w;
    edges[0].b.y = rect->y;

    for (int i = 1; i < 4; ++i)
    {
        edges[i].a = edges[i - 1].b;

        edges[i].b = edges[i].a;

        switch (i)
        {
            case 1:
                edges[i].b.y += rect->h;
                break;

            case 2:
                edges[i].b.x -= rect->w;
                break;

            case 3:
                edges[i].b.y -= rect->h;
                break;
        }
    }
}

void zfw_init_identity_matrix_4x4(zfw_matrix_4x4_t *const mat)
{
    memset(mat, 0, sizeof(*mat));

    for (int i = 0; i < 4; i++)
    {
        mat->elems[i][i] = 1.0f;
    }
}

void zfw_init_ortho_matrix_4x4(zfw_matrix_4x4_t *const mat, const float left, const float right, const float bottom,
                               const float top, const float near, const float far)
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

