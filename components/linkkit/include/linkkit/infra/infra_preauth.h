/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __INFRA_PREAUTH__
#define __INFRA_PREAUTH__

#include "linkkit/infra/infra_defs.h"
#include "linkkit/infra/infra_types.h"

#ifdef INFRA_LOG
#include "linkkit/infra/infra_log.h"
#define preauth_err(...)   log_err("preauth", __VA_ARGS__)
#define preauth_info(...)  log_info("preauth", __VA_ARGS__)
#define preauth_debug(...) log_debug("preauth", __VA_ARGS__)
#else
#define preauth_err(...)         \
    do {                         \
        HAL_Printf(__VA_ARGS__); \
        HAL_Printf("\r\n");      \
    } while (0)
#define preauth_info(...)        \
    do {                         \
        HAL_Printf(__VA_ARGS__); \
        HAL_Printf("\r\n");      \
    } while (0)
#define preauth_debug(...)       \
    do {                         \
        HAL_Printf(__VA_ARGS__); \
        HAL_Printf("\r\n");      \
    } while (0)
#endif

int preauth_get_connection_info(iotx_mqtt_region_types_t region,
                                iotx_dev_meta_info_t *dev_meta,
                                const char *sign, const char *device_id,
                                iotx_sign_mqtt_t *preauth_output);

#endif /* #ifndef __INFRA_PREAUTH__ */

