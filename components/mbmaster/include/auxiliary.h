/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef AUXILIARY_H
#define AUXILIARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ulog/ulog.h>
#include "aos/kernel.h"

#define MODBUS_MOUDLE "modbusm"
/* forward declaration */
typedef enum mb_status mb_status_t;
typedef struct mb_handler mb_handler_t;

/* Commented out as esp32 doesn't support following feature */
// #define MB_CRITICAL_ALLOC() CPSR_ALLOC()
// #define MB_CRITICAL_ENTER() RHINO_CRITICAL_ENTER()
// #define MB_CRITICAL_EXIT()  RHINO_CRITICAL_EXIT()

#define MB_CRITICAL_ALLOC()
#define MB_CRITICAL_ENTER()
#define MB_CRITICAL_EXIT()

#define MB_MUTEX_T             aos_mutex_t
#define MB_MUTEX_CREATE(mutex) mb_mutex_create(mutex)
#define MB_MUTEX_LOCK(mutex)   mb_mutex_lock(mutex)
#define MB_MUTEX_UNLOCK(mutex) mb_mutex_unlock(mutex)
#define MB_MUTEX_DEL(mutex)    mb_mutex_del(mutex)

#ifndef htobe16
#define htobe16(X) (((X >> 8) & 0x00ff) | ((X << 8) & 0xff00))
#endif

#ifndef betoh16
#define betoh16(X) (((X >> 8) & 0x00ff) | ((X << 8) & 0xff00))
#endif

uint8_t *status_to_string(mb_status_t status);

mb_handler_t *mb_alloc_handler();
void mb_free_handler(mb_handler_t *handler);

mb_status_t mb_mutex_create(MB_MUTEX_T *mutex);
mb_status_t mb_mutex_lock(MB_MUTEX_T *mutex);
mb_status_t mb_mutex_unlock(MB_MUTEX_T *mutex);
mb_status_t mb_mutex_del(MB_MUTEX_T *mutex);

#ifdef __cplusplus
}
#endif

#endif /* AUXILIARY_H */
