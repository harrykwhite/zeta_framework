#ifndef __ZFW_COMMON_H__
#define __ZFW_COMMON_H__

#include "zfw_common_math.h" // NOTE: This should maybe be removed from here.

#define ZFW_MIN(X, Y) (Y < X ? Y : X)
#define ZFW_MAX(X, Y) (Y > X ? Y : X)

#define ZFW_SIZE_IN_BITS(X) (sizeof(X) * 8)

#define ZFW_ARRAY_LEN(X) (sizeof(X) / sizeof(X[0]))

#define ZFW_FALSE 0
#define ZFW_TRUE 1

#define ZFW_ASSETS_FILE_NAME "assets.zfwdat"
#define ZFW_TEX_CHANNEL_COUNT 4
#define ZFW_SHADER_SRC_MAX_LEN 2048

typedef int zfw_bool_t;

void zfw_log(const char *const msg, ...);
void zfw_log_error(const char *const msg, ...);

zfw_bool_t zfw_check_data_type_sizes();

#endif
