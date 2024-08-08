#ifndef __ZFW_MATH_H__
#define __ZFW_MATH_H__

#include <zfw_common_math.h>

#define ZFW_POLY_PT_LIMIT 1024

typedef struct
{
    int first_pt_index;
    int pt_count;
} zfw_poly_t;

void zfw_gen_poly(zfw_poly_t *const poly, const int pt_count);
void zfw_free_poly(zfw_poly_t *const poly);
zfw_vec_2d_t zfw_get_poly_pt(const zfw_poly_t poly, const int rel_pt_index);
void zfw_set_poly_pt(const zfw_poly_t poly, const int rel_pt_index, const zfw_vec_2d_t poly_pt);
zfw_bool_t zfw_is_pt_in_poly(const zfw_vec_2d_t pt, const zfw_poly_t poly);
zfw_bool_t zfw_do_polys_inters(const zfw_poly_t poly_a, const zfw_poly_t poly_b);

#endif
