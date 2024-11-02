#include <zfw_math.h>

#include <zfw_common_bits.h>

#define POLY_PT_ALIGNMENT 4

static zfw_vec_2d_t g_poly_pts[ZFW_POLY_PT_LIMIT];
static zfw_bits_t g_poly_pt_activity[ZFW_BIT_COUNT_AS_BYTE_COUNT(ZFW_POLY_PT_LIMIT)];

void zfw_gen_poly(zfw_poly_t *const poly, const int pt_count)
{
    // TODO: Improve this silly algorithm. Would be better to just check whether entire bytes are free instead of checking one bit at a time. Have the alignment correlate with byte size and count.
    poly->pt_count = pt_count;

    for (int i = 0; i < ZFW_POLY_PT_LIMIT; i += POLY_PT_ALIGNMENT)
    {
        zfw_bool_t can_use = ZFW_TRUE;

        for (int j = 0; j < poly->pt_count; ++j)
        {
            if (zfw_is_bit_active(i + j, g_poly_pt_activity))
            {
                can_use = ZFW_FALSE;
                break;
            }
        }

        if (can_use)
        {
            poly->first_pt_index = i;

            for (int j = 0; j < poly->pt_count; ++j)
            {
                zfw_activate_bit(poly->first_pt_index + j, g_poly_pt_activity);
            }

            break;
        }
    }
}

void zfw_free_poly(zfw_poly_t *const poly)
{
    for (int i = 0; i < poly->pt_count; ++i)
    {
        zfw_deactivate_bit(poly->first_pt_index + i, g_poly_pt_activity);
    }

    poly->pt_count = 0;
    poly->first_pt_index = 0;
}

zfw_vec_2d_t zfw_get_poly_pt(const zfw_poly_t poly, const int rel_pt_index)
{
    return g_poly_pts[poly.first_pt_index + rel_pt_index];
}

void zfw_set_poly_pt(const zfw_poly_t poly, const int rel_pt_index, const zfw_vec_2d_t pt)
{
    g_poly_pts[poly.first_pt_index + rel_pt_index] = pt;
}

zfw_bool_t zfw_is_pt_in_poly(const zfw_vec_2d_t pt, const zfw_poly_t poly)
{
    // Count the number of intersections there are with the polygon's edges when a line is cast from the point to the right. The point is in the polygon if and
    // only if the count is odd.
    zfw_bool_t in_poly = ZFW_FALSE;

    for (int i = 0; i < poly.pt_count; ++i)
    {
        const zfw_line_t line = {g_poly_pts[poly.first_pt_index + i], g_poly_pts[poly.first_pt_index + ((i + 1) % poly.pt_count)]};

        zfw_vec_2d_t top_line_pt, bottom_line_pt;

        if (line.b.y >= line.a.y)
        {
            top_line_pt = line.a;
            bottom_line_pt = line.b;
        }
        else
        {
            top_line_pt = line.b;
            bottom_line_pt = line.a;
        }

        if (pt.y < top_line_pt.y || pt.y > bottom_line_pt.y)
        {
            continue;
        }

        // TODO: Account for possible division by zero.

        const float line_inters_y_perc = (pt.y - top_line_pt.y) / (bottom_line_pt.y - top_line_pt.y);
        const float line_inters_x = top_line_pt.x + (line_inters_y_perc * (bottom_line_pt.x - top_line_pt.x));

        if (pt.x <= line_inters_x)
        {
            in_poly = !in_poly;
        }
    }

    return in_poly;
}

zfw_bool_t zfw_do_polys_inters(const zfw_poly_t poly_a, const zfw_poly_t poly_b)
{
    // Check whether any of the points of the first polygon are inside the second.
    for (int i = 0; i < poly_a.pt_count; ++i)
    {
        if (zfw_is_pt_in_poly(g_poly_pts[poly_a.first_pt_index + i], poly_b))
        {
            return ZFW_TRUE;
        }
    }

    // Do the same but for the second polygon.
    for (int i = 0; i < poly_b.pt_count; ++i)
    {
        if (zfw_is_pt_in_poly(g_poly_pts[poly_b.first_pt_index + i], poly_a))
        {
            return ZFW_TRUE;
        }
    }

    return ZFW_FALSE;
}
