#ifndef __ZFW_COMMON_MISC_H__
#define __ZFW_COMMON_MISC_H__

#include <stdlib.h>

#define ZFW_FALSE 0
#define ZFW_TRUE 1

#define ZFW_STATIC_ARRAY_LEN(X) (sizeof(X) / sizeof((X)[0]))

typedef int zfw_bool_t;

inline float zfw_gen_rand_perc()
{
    return (float)rand() / RAND_MAX;
}

inline int zfw_gen_rand_int_in_range(const int min, const int max)
{
    return min + ((max - min) * zfw_gen_rand_perc());
}

inline float zfw_gen_rand_float_in_range(const float min, const float max)
{
    return min + ((max - min) * zfw_gen_rand_perc());
}

#endif

