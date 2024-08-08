#include "zfw_common_misc.h"

#include "zfw_common_debug.h"

zfw_bool_t zfw_check_data_type_sizes()
{
    if (CHAR_BIT != 8)
    {
        zfw_log_error("Chars must have a size of 8 bits!");
        return ZFW_FALSE;
    }

    if (sizeof(short) != 2)
    {
        zfw_log_error("Shorts must have a size of 2 bytes!");
        return ZFW_FALSE;
    }

    if (sizeof(int) != 4)
    {
        zfw_log_error("Ints must have a size of 4 bytes!");
        return ZFW_FALSE;
    }

    if (sizeof(long long) != 8)
    {
        zfw_log_error("Long longs must have a size of 8 bytes!");
        return ZFW_FALSE;
    }

    return ZFW_TRUE;
}
