/** @file
 *  @brief Bluetooth subsystem logging helpers.
 */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT_LOG_H
#define __BT_LOG_H


#ifdef __cplusplus
extern "C" {
#endif

//#include "adv.h"
#include "hal_trace.h"

#if !defined(BT_DBG_ENABLED)
#define BT_DBG_ENABLED 1
#endif

#if defined(CONFIG_BT_DEBUG_MONITOR)
#include <stdio.h>

/* These defines follow the values used by syslog(2) */
#define BT_LOG_ERR      3
#define BT_LOG_WARN     4
#define BT_LOG_INFO     6
#define BT_LOG_DBG      7

__printf_like(2, 3) void bt_log(int prio, const char *fmt, ...);

#define BT_DBG(fmt, ...) \
	if (BT_DBG_ENABLED) { \
		bt_log(BT_LOG_DBG, "%s (%p): " fmt, \
		       __func__, k_current_get(), ##__VA_ARGS__); \
	}

#define BT_ERR(fmt, ...) bt_log(BT_LOG_ERR, "%s: " fmt, \
				__func__, ##__VA_ARGS__)
#define BT_WARN(fmt, ...) bt_log(BT_LOG_WARN, "%s: " fmt, \
				 __func__, ##__VA_ARGS__)
#define BT_INFO(fmt, ...) bt_log(BT_LOG_INFO, fmt, ##__VA_ARGS__)

/* Enabling debug increases stack size requirement */
#define BT_STACK_DEBUG_EXTRA	300

#elif defined(CONFIG_BT_DEBUG_LOG)

#if !defined(SYS_LOG_DOMAIN)
#define SYS_LOG_DOMAIN "bt"
#endif
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_DEBUG

#define BT_DBG(fmt, ...) \
	if (BT_DBG_ENABLED) { \
		printf(fmt"\n",\
			    ##__VA_ARGS__); \
	}

#define BT_ERR(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define BT_WARN(fmt, ...) BT_DBG(fmt"\n", ##__VA_ARGS__)
#define BT_INFO(fmt, ...) BT_DBG(fmt"\n", ##__VA_ARGS__)

/* Enabling debug increases stack size requirement considerably */
#define BT_STACK_DEBUG_EXTRA	300

#else

#define BT_DBG  TRACE
#define BT_ERR BT_DBG
#define BT_WARN BT_DBG
#define BT_INFO BT_DBG

#define BT_STACK_DEBUG_EXTRA	0

#endif



#define BT_ASSERT(cond) if (!(cond)) { \
				BT_ERR("assert: '" #cond "' failed"); \
				k_oops(); \
			}


/* This helper is only available when BT_DEBUG is enabled */
const char *bt_hex(const void *buf, size_t len);

uint8_t stringtohex(char *str, uint8_t *out, uint8_t count);
void hextostring(const uint8_t *source, char *dest, int len);

/* These helpers are only safe to be called from internal threads as they're
 * not multi-threading safe
 */


#ifdef __cplusplus
}
#endif

#endif /* __BT_LOG_H */

