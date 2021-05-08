#include <stdint.h>

// options to control how MicroPython is built

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)

#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_QSTR_EXTRA_POOL     mp_qstr_frozen_const_pool
#define MICROPY_ALLOC_PATH_MAX      (256)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (16)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_ENABLE_GC           (0)
#define MICROPY_GC_ALLOC_THRESHOLD  (0)
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_REPL_AUTO_INDENT    (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT      (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (0)
#define MICROPY_PY_BUILTINS_ENUMERATE (0)
#define MICROPY_PY_BUILTINS_FILTER  (0)
#define MICROPY_PY_BUILTINS_FROZENSET (0)
#define MICROPY_PY_BUILTINS_REVERSED (0)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (0)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (1)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_MODULE_FROZEN_MPY   (1)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_PY_UTIME            (1)
#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_MODULE_WEAK_LINKS   (1)
#define MICROPY_READER_POSIX        (1)
#define MICROPY_REPL_EMACS_KEYS     (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
#define MICROPY_PY_SDCARD           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_THREAD           (1)
#define MICROPY_ENABLE_SCHEDULER    (1)

#if 1
/*sys path for file system to import py*/
#define MICROPY_PY_SYS              (1)
/*enable muti thread*/
#define MICROPY_PY_THREAD_GIL               (1)
#define MICROPY_PY_THREAD_GIL_VM_DIVISOR    (32)
/*enable import py*/
#define MICROPY_ENABLE_EXTERNAL_IMPORT (1)
/*use posix API as open to read file*/
#define MICROPY_READER_POSIX (1)
/*print debug info*/
#define MICROPY_DEBUG_VERBOSE (0)
#endif

// type definitions for the specific machine

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

// This port is intended to be 32-bit, but unfortunately, int32_t for
// different targets may be defined in different ways - either as int
// or as long. This requires different printf formatting specifiers
// to print such value. So, we avoid int32_t and use int directly.
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size

typedef long mp_off_t;

#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#ifndef MICROPY_HW_BOARD_NAME
#define MICROPY_HW_BOARD_NAME "HaaS100"
#endif

#ifndef MICROPY_HW_MCU_NAME
#define MICROPY_HW_MCU_NAME ""
#endif

#ifdef __thumb__
#define MICROPY_MIN_USE_CORTEX_CPU (1)
#define MICROPY_MIN_USE_STM32_MCU (1)
#define MICROPY_MIN_USE_XM510_MCU (1)

#endif

#if MICROPY_PY_THREAD
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(false); \
        MP_THREAD_GIL_EXIT(); \
        MP_THREAD_GIL_ENTER(); \
    } while (0);
#else
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(false); \
        krhino_task_sleep(1); \
    } while (0);

#define MICROPY_THREAD_YIELD()
#endif


#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8];


#ifdef MICROPY_PY_UTIME
#define MICROPY_PY_UTIME_DEF
#define MICROPY_PY_UTIME_DEF_WEAK_LINKS
#else
#define MICROPY_PY_UTIME_DEF
#define MICROPY_PY_UTIME_DEF_WEAK_LINKS
#endif

extern const struct _mp_obj_module_t utime_module;
extern const struct _mp_obj_module_t minicv_module;
extern const struct _mp_obj_module_t driver_module;
extern const struct _mp_obj_module_t sdcard_module;
#if MICROPY_PY_SDCARD
#define MICROPY_PY_SDCARD_DEF {MP_ROM_QSTR(MP_QSTR_sdcard), MP_ROM_PTR(&sdcard_module)},
#else
#define MICROPY_PY_SDCARD_DEF
#endif


#if PY_BUILD_OSS
extern const struct _mp_obj_module_t oss_module;
#define MICROPY_PY_OSS_DEF { MP_ROM_QSTR(MP_QSTR_OSS), MP_ROM_PTR(&oss_module) },
#else
#define MICROPY_PY_OSS_DEF
#endif

#if PY_BUILD_HTTP
extern const struct _mp_obj_module_t http_module;
#define MICROPY_PY_HTTP_DEF { MP_ROM_QSTR(MP_QSTR_http), MP_ROM_PTR(&http_module) },
#else
#define MICROPY_PY_HTTP_DEF
#endif

#if PY_BUILD_TCP
extern const struct _mp_obj_module_t tcp_module;
#define MICROPY_PY_TCP_DEF { MP_ROM_QSTR(MP_QSTR_tcp), MP_ROM_PTR(&tcp_module) },
#else
#define MICROPY_PY_TCP_DEF
#endif

#if PY_BUILD_UDP
extern const struct _mp_obj_module_t udp_module;
#define MICROPY_PY_UDP_DEF { MP_ROM_QSTR(MP_QSTR_udp), MP_ROM_PTR(&udp_module) },
#else
#define MICROPY_PY_UDP_DEF
#endif


#if PY_BUILD_NETMGR
extern const struct _mp_obj_module_t netmgr_module;
#define MICROPY_PY_NETMGR_DEF { MP_ROM_QSTR(MP_QSTR_netmgr), MP_ROM_PTR(&netmgr_module) },
#else
#define MICROPY_PY_NETMGR_DEF
#endif

#if PY_BUILD_BLENETCONFIG
extern const struct _mp_obj_module_t blenetconfig_module;
#define MICROPY_PY_BLE_NETCONFIG_DEF { MP_ROM_QSTR(MP_QSTR_ble_netconfig), MP_ROM_PTR(&blenetconfig_module) },
#else
#define MICROPY_PY_BLE_NETCONFIG_DEF
#endif

#if PY_BUILD_MQTT
extern const struct _mp_obj_module_t mqtt_module;
#define MICROPY_PY_MQTT_DEF { MP_ROM_QSTR(MP_QSTR_mqtt), MP_ROM_PTR(&mqtt_module) },
#else
#define MICROPY_PY_MQTT_DEF
#endif

#if PY_BUILD_LINKSDK
extern const struct _mp_obj_module_t linkkit_module;
#define MICROPY_PY_LINKSDK_DEF { MP_ROM_QSTR(MP_QSTR__linkkit), MP_ROM_PTR(&linkkit_module) },
#else
#define MICROPY_PY_LINKSDK_DEF
#endif


#if PY_BUILD_USOCKET
extern const struct _mp_obj_module_t mp_module_usocket;
#define MICROPY_PY_USOCKET_DEF { MP_ROM_QSTR(MP_QSTR_usocket), MP_ROM_PTR(&mp_module_usocket) },
#else
#define MICROPY_PY_USOCKET_DEF
#endif


#if PY_BUILD_UOS
extern const struct _mp_obj_module_t mp_module_os;
#define MICROPY_PY_UOS_DEF { MP_ROM_QSTR(MP_QSTR_uos), MP_ROM_PTR(&mp_module_os) },
#else
#define MICROPY_PY_UOS_DEF
#endif


#if PY_BUILD_AUDIO
extern const struct _mp_obj_module_t audio_module;
#define MICROPY_PY_AUDIO_DEF {MP_ROM_QSTR(MP_QSTR_audio), MP_ROM_PTR(&audio_module)},
#else
#define MICROPY_PY_AUDIO_DEF
#endif

#define MICROPY_PORT_BUILTIN_MODULES \
        MICROPY_PY_UTIME_DEF \
        {MP_ROM_QSTR(MP_QSTR_utime), MP_ROM_PTR(&utime_module)}, \
        {MP_ROM_QSTR(MP_QSTR_minicv), MP_ROM_PTR(&minicv_module)}, \
        {MP_ROM_QSTR(MP_QSTR_driver), MP_ROM_PTR(&driver_module)}, \
        MICROPY_PY_LINKSDK_DEF \
        MICROPY_PY_HTTP_DEF \
        MICROPY_PY_UDP_DEF \
        MICROPY_PY_TCP_DEF \
        MICROPY_PY_NETMGR_DEF \
        MICROPY_PY_BLE_NETCONFIG_DEF \
        MICROPY_PY_MQTT_DEF \
        MICROPY_PY_USOCKET_DEF \
        MICROPY_PY_AUDIO_DEF \
        MICROPY_PY_UOS_DEF \
        MICROPY_PY_SDCARD_DEF \
        MICROPY_PY_OSS_DEF \





#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
        MICROPY_PY_UTIME_DEF_WEAK_LINKS \


