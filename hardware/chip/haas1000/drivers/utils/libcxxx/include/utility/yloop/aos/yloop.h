/**
 * @file yloop.h
 * yloop API header file.
 *
 * @version   V1.0
 * @date      2019-11-08
 * @copyright Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef AOS_YLOOP_H
#define AOS_YLOOP_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup aos_yloop yloop
 *  yloop interfaces.
 *
 *  @{
 */

#include <stdint.h>
#include <event_type_code.h>

#if defined(CONFIG_AOS_LWIP) || defined(CONFIG_VENDOR_LWIP) || defined(CONFIG_NO_LWIP) || defined(WITH_SAL)
#define POLLIN  0x1
#define POLLOUT 0x2
#define POLLERR 0x4

/**
 * @brief poll fd type
 */
struct pollfd {
    /* foll fd */
    int   fd;
    /* event */
    short events;
    /* read event */
    short revents;
};

#else  /* defined(CONFIG_AOS_LWIP) || defined(CONFIG_VENDOR_LWIP) || defined(CONFIG_NO_LWIP) || defined(WITH_SAL) */

#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif /* defined(CONFIG_AOS_LWIP) || defined(CONFIG_VENDOR_LWIP) || defined(CONFIG_NO_LWIP) || defined(WITH_SAL) */

#ifndef AOS_DOXYGEN_MODE

/* special event filter */
#define EV_ALL                       0
#define EV_FLAG_URGENT               0x8000

/* system event */
#define EV_SYS                       0x0001
#define CODE_SYS_ON_STARTING         1
#define CODE_SYS_ON_START_COMPLETED  2
#define CODE_SYS_ON_START_FAILED     4
#define CODE_SYS_ON_IDLE             3
#define CODE_SYS_ON_START_UOTA       5
#define CODE_SYS_ON_ALINK_ONLINE     6
#define CODE_SYS_ON_ALINK_OFFLINE    7
#define CODE_SYS_ON_MQTT_READ        8
#define CODE_SYS_ON_COAP_AUTHED      9

/* WiFi event */
#define  EV_WIFI                  0x0002
#define  CODE_WIFI_CMD_RECONNECT  1
#define  CODE_WIFI_ON_CONNECTED   2
#define  CODE_WIFI_ON_DISCONNECT  3
#define  CODE_WIFI_ON_PRE_GOT_IP  4
#define  CODE_WIFI_ON_GOT_IP      5

/* cellular event */
#define  EV_CELLULAR                  0x0005
#define  CODE_CELLULAR_CMD_RECONNECT  1
#define  CODE_CELLULAR_ON_CONNECTED   2
#define  CODE_CELLULAR_ON_DISCONNECT  3
#define  CODE_CELLULAR_ON_PRE_GOT_IP  4
#define  CODE_CELLULAR_ON_GOT_IP      5

/* Network Event */
#define EV_NETWORK EV_WIFI
#define CODE_ON_DISCONNECT CODE_WIFI_ON_DISCONNECT
#define CODE_ON_GOT_IP CODE_WIFI_ON_GOT_IP

/* Mesh event */
#define EV_MESH                  0x0003

/* user app start 0x1000 - 0x7fff */
#define EV_USER                  0x1000

/** uData event */
#define EV_UDATA                           0x0004
#define CODE_UDATA_DEV_READ                1
#define CODE_UDATA_DEV_IOCTL               2
#define CODE_UDATA_DEV_OPEN                3
#define CODE_UDATA_DEV_CLOSE               4
#define CODE_UDATA_DEV_ENABLE              5
#define CODE_UDATA_DEV_DISABLE             6
#define CODE_UDATA_SERVICE_SUBSRIBE        7 /* for external component */
#define CODE_UDATA_SERVICE_UNSUBSRIBE      8 /* for external component */
#define CODE_UDATA_SERVICE_PROCESS         9
#define CODE_UDATA_SERVICE_IOCTL           10
#define CODE_UDATA_REPORT_PUBLISH          11
#define CODE_UDATA_DATA_PUBLISH            12
#define CODE_UDATA_DATA_SUBSCRIB           13
#endif /* AOS_DOXYGEN_MODE */

/**
 * @brief input event type
 */
typedef struct {
    /* The time event is generated, auto filled by aos event system */
    uint32_t time;
    /* Event type, value < 0x1000 are used by aos system */
    uint16_t type;
    /* Defined according to type */
    uint16_t code;
    /* Defined according to type/code */
    unsigned long value;
    /* Defined according to type/code */
    unsigned long extra;
} input_event_t;

/**
 * Event callback.
 *
 * @param[in]  event         input event.
 * @param[in]  private_data  private data for cb.
 *
 * @return none.
 */
typedef void (*aos_event_cb)(input_event_t *event, void *private_data);

/**
 * Delayed execution callback.
 *
 * @param[in] arg argument for execution.
 *
 * @return none.
 */
typedef void (*aos_call_t)(void *arg);

/**
 * Poll event callback.
 *
 * @param[in]  fd    poll fd.
 * @param[in]  arg   argument for execution.
 *
 * @return none.
 */
typedef void (*aos_poll_call_t)(int fd, void *arg);

/**
 * Register system event filter callback.
 *
 * @param[in]  type  event type interested.
 * @param[in]  cb    system event callback.
 * @param[in]  priv  private data past to cb.
 *
 * @return 0 on success, negative on failure
 */
int aos_register_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Unregister native event callback.
 *
 * @param[in]  type  event type interested.
 * @param[in]  cb    system event callback.
 * @param[in]  priv  private data past to cb.
 *
 * @return 0 on success, negative on failure
 */
int aos_unregister_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Post local event.
 *
 * @param[in]  type   event type.
 * @param[in]  code   event code.
 * @param[in]  value  event value.
 *
 * @return 0 on success, negative on failure
 */
int aos_post_event(uint16_t type, uint16_t code, unsigned long  value);

/**
 * Register a poll event in main loop.
 *
 * @param[in]  fd      poll fd.
 * @param[in]  action  action to be executed.
 * @param[in]  param   private data past to action.
 *
 * @return 0 on success, negative on failure
 */
int aos_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Cancel a poll event to be executed in main loop.
 *
 * @param[in]  fd      poll fd.
 * @param[in]  action  action to be executed.
 * @param[in]  param   private data past to action.
 *
 * @return none.
 */
void aos_cancel_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Post a delayed action to be executed in main loop.
 *
 * @param[in]  ms      milliseconds to wait.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return 0 on success, negative on failure
 */
int aos_post_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * @brief Cancel a delayed action to be executed in main loop.
 *
 * @param[in]  ms      milliseconds to wait, -1 means don't care.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return none.
 */
void aos_cancel_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop.
 * Unlike aos_post_delayed_action,
 * this function can be called from non-aos-main-loop context.

 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return 0 on success, negative on failure
 */
int aos_schedule_call(aos_call_t action, void *arg);

/**
 * loop type
 */
typedef void *aos_loop_t;

/**
 * Init a per-task event loop.
 *
 * @return  the handler of aos_loop_t, NULL on failure, others on success.
 */
aos_loop_t aos_loop_init(void);

/**
 * Get current event loop.
 *
 * @return  default event loop.
 */
aos_loop_t aos_current_loop(void);

/**
 * Start event loop.
 *
 * @return none.
 */
void aos_loop_run(void);

/**
 * Exit event loop, aos_loop_run() will return.
 *
 * @return none.
 */
void aos_loop_exit(void);

/**
 * Free event loop resources.
 *
 * @return none.
 */
void aos_loop_destroy(void);

/**
 * Schedule a callback specified event loop.
 *
 * @param[in]  loop    event loop to be scheduled, NULL for default main loop.
 * @param[in]  action  action to be executed.
 * @param[in]  arg     private data past to action.
 *
 * @return 0 on success, negative on failure
 */
int aos_loop_schedule_call(aos_loop_t *loop, aos_call_t action, void *arg);

/**
 * Schedule a work to be executed in workqueue.
 *
 * @param[in]  ms       milliseconds to delay before execution, 0 means immediately.
 * @param[in]  action   action to be executed.
 * @param[in]  arg1     private data past to action.
 * @param[in]  fini_cb  finish callback to be executed after action is done in current event loop.
 * @param[in]  arg2     data past to fini_cb.
 *
 * @return  work handle, NULL on failure, non-NULL on success
 */
void *aos_loop_schedule_work(int ms, aos_call_t action, void *arg1,
                             aos_call_t fini_cb, void *arg2);

/**
 * Cancel a work.
 *
 * @param[in]  work    work to be cancelled.
 * @param[in]  action  action to be executed.
 * @param[in]  arg1    private data past to action.
 *
 * @return none.
 */
void aos_cancel_work(void *work, aos_call_t action, void *arg1);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AOS_YLOOP_H */

