/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdint.h>

#include "amp_system.h"
#include "amp_defines.h"
#include "amp_hal_can.h"
#include "board_mgr.h"
#include "amp_task.h"
#include "be_inl.h"

#define MOD_STR "CAN"
#define CAN_TIMEOUT (0xFFFFFF)
#define MAX_CAN_RECV_LEN    8
#define MAX_CAN_PORT    2

typedef struct {
    can_dev_t *can_device;
    int js_cb_ref;
} can_recv_param_t;

typedef struct {
    uint8_t buf[MAX_CAN_RECV_LEN];
    can_frameheader_t rx_header;
    int js_cb_ref;
} can_recv_notify_param_t;

static int close_flag = 0;
extern int update_flag;
can_dev_t g_can_handle[MAX_CAN_PORT];

static duk_ret_t native_open(duk_context *ctx)
{
    int8_t ret = -1;
    item_handle_t can_handle;
    can_handle.handle     = NULL;
    can_dev_t *can_device = NULL;

    if (!duk_is_string(ctx, 0)) {
        amp_warn(MOD_STR, "parameter must be string");
        goto out;
    }
    const char *id = duk_get_string(ctx, 0);
    ret            = board_attach_item(MODULE_CAN, id, &can_handle);
    if (0 != ret) {
        amp_error(MOD_STR, "board_attach_item fail!");
        goto out;
    }
    amp_debug(MOD_STR, "can handle:%u", can_handle.handle);
    can_device = board_get_node_by_handle(MODULE_CAN, &can_handle);
    if (NULL == can_device) {
        amp_error(MOD_STR, "board_get_node_by_handle fail!");
        goto out;
    }

    ret = amp_hal_can_init(can_device);
    if (0 != ret) {
        amp_error(MOD_STR, "amp_hal_can_init fail!");
        goto out;
    }

    g_can_handle[can_device->port] = *can_device;

out:
    if (0 != ret) {
        duk_push_pointer(ctx, NULL);
        board_disattach_item(MODULE_CAN, &can_handle);
    } else {
        duk_push_pointer(ctx, (void *)can_handle.handle);
    }
    return 1;
}

static duk_ret_t native_close(duk_context *ctx)
{
    int8_t ret = -1;
    item_handle_t can_handle;
    can_dev_t *can_device = NULL;

    if (!duk_is_pointer(ctx, 0)) {
        amp_warn(MOD_STR, "parameter must be handle");
        goto out;
    }
    can_handle.handle = duk_get_pointer(ctx, 0);
    can_device        = board_get_node_by_handle(MODULE_CAN, &can_handle);
    if (NULL == can_device) {
        amp_error(MOD_STR, "board_get_node_by_handle fail!");
        goto out;
    }
    ret = amp_hal_can_finalize(can_device);
    if (0 != ret) {
        amp_error(MOD_STR, "amp_hal_spi_finalize fail!");
        goto out;
    }
    board_disattach_item(MODULE_CAN, &can_handle);
    close_flag = 1;

out:
    duk_push_int(ctx, ret);
    return 1;
}

static duk_ret_t native_send(duk_context *ctx)
{
    int8_t ret    = -1;
    uint8_t *data = NULL;
    uint32_t len  = 0;
    uint32_t i    = 0;
    item_handle_t can_handle;
    can_dev_t *can_device = NULL;
    can_frameheader_t tx_header;
    int arr_idx;
    uint32_t id;
    uint8_t rtr, dlc;
    int err = -1;

    if (!duk_is_pointer(ctx, 0) || !duk_is_object(ctx, 1) ||
        !duk_is_array(ctx, 2)) {
        amp_warn(MOD_STR, "parameter must be handle object array");
        goto out;
    }

    can_handle.handle = duk_get_pointer(ctx, 0);
    can_device        = board_get_node_by_handle(MODULE_CAN, &can_handle);
    if (NULL == can_device) {
        amp_error(MOD_STR, "board_get_node_by_handle fail!");
        goto out;
    }

    /* get can_frame header */
    duk_get_prop_string(ctx, 1, "id");
    duk_get_prop_string(ctx, 1, "rtr");
    duk_get_prop_string(ctx, 1, "dlc");

    if (!duk_is_number(ctx, -3) || !duk_is_number(ctx, -2) ||
        !duk_is_number(ctx, -1))
    {
        amp_warn(MOD_STR, 
            "Parameter 1 must be an object like {id: number, rtr: number, dlc: number");
        ret = -2;
        goto out;
    }

    id    = duk_get_uint(ctx, -3);
    rtr   = duk_get_uint(ctx, -2);
    if (rtr > 1) {
        amp_warn(MOD_STR, "rtr is invalid");
        goto out;
    }

    dlc  = duk_get_uint(ctx, -1);
    if (dlc > 8) {
        amp_warn(MOD_STR, "dlc is invalid");
        goto out;
    }

    duk_pop_3(ctx);

    tx_header.id = id;
    tx_header.rtr = rtr;
    tx_header.dlc = dlc;

    arr_idx = duk_normalize_index(ctx, 2);
    len     = duk_get_length(ctx, arr_idx);
    data    = (uint8_t *)amp_malloc(sizeof(uint8_t) * len);
    if (NULL == data) {
        amp_warn(MOD_STR, "allocate memory failed");
        goto out;
    }
    for (i = 0; i < len; i++) {
        duk_get_prop_index(ctx, arr_idx, i);
        if (!duk_is_number(ctx, -1)) {
            amp_warn(MOD_STR, "data is not number, index: %d", i);
            duk_pop(ctx);
            goto out;
        }
        data[i] = (uint8_t)duk_get_int(ctx, -1);
        duk_pop(ctx);
    }

    ret = amp_hal_can_send(can_device, &tx_header, data, CAN_TIMEOUT);
    if (-1 == ret) {
        amp_error(MOD_STR, "amp_hal_can_send fail!");
        goto out;
    }
    err = 0;
out:
    amp_free(data);
    duk_push_int(ctx, err);
    return 1;
}

static void can_recv_notify(void *pdata)
{
    int i                      = 0;
    can_recv_notify_param_t *p = (can_recv_notify_param_t *)pdata;
    duk_context *ctx           = be_get_context();

    be_push_ref(ctx, p->js_cb_ref);
    duk_push_uint(ctx, p->rx_header.rtr);
    duk_push_uint(ctx, p->rx_header.id);
    int arr_idx = duk_push_array(ctx);
    if(p->rx_header.rtr == 0) {
        for (i = 0; i < p->rx_header.dlc; i++) {
            duk_push_int(ctx, p->buf[i]);
            duk_put_prop_index(ctx, arr_idx, i);
        }
    }
    if (duk_pcall(ctx, 3) != DUK_EXEC_SUCCESS) {
        amp_console("%s", duk_safe_to_stacktrace(ctx, -1));
    }
    
    duk_pop(ctx);
    duk_gc(ctx, 0);
    // amp_free(p);
}

/*************************************************************************************
 * Function:    udp_recv_routine
 * Description: create a task for blocking recvfrom call
 * Called by:
 **************************************************************************************/
static void *can_recv_routine(void *arg)
{
    int ret = -1;
    can_recv_param_t *recv_param = (can_recv_param_t *)arg;
    can_dev_t *can_device = (can_dev_t *)recv_param->can_device;

    can_recv_notify_param_t *p = amp_calloc(1, sizeof(*p));
    if (!p) {
        amp_warn(MOD_STR, "allocate memory failed");
        duk_context *ctx = be_get_context();
        be_unref(ctx, recv_param->js_cb_ref);
        goto out;
    }
    close_flag = 0;

    while(1) {
        ret = amp_hal_can_recv(can_device, &p->rx_header, p->buf, CAN_TIMEOUT);
        if (ret == 0) {
            p->js_cb_ref = recv_param->js_cb_ref;
            amp_task_schedule_call(can_recv_notify, p);
        }

        if (update_flag || close_flag) {
            duk_context *ctx = be_get_context();
            be_unref(ctx, recv_param->js_cb_ref);
            amp_free(p);
            break;
        }
        
        HAL_SleepMs(10);
    }
    ret = amp_hal_can_finalize(can_device);
    if (ret != 0) {
        amp_error(MOD_STR, "hal can finalize failed");
    }

out:
    amp_free(recv_param);
    HAL_ThreadDelete(NULL);

    return NULL;
}

static duk_ret_t native_receive(duk_context *ctx)
{
    int8_t ret    = -1;
    uint8_t *data = NULL;
    item_handle_t can_handle;
    can_dev_t *can_device = NULL;
    can_recv_param_t *recv_param;
    void *can_recv_task;
    int can_recv_task_stack_used;
    amp_os_thread_param_t can_recv_task_params = {0};

    if (!duk_is_pointer(ctx, 0) || !duk_is_function(ctx, 1)) {
        amp_warn(MOD_STR, "parameter must be handle and function");
        goto out;
    }

    can_handle.handle = duk_get_pointer(ctx, 0);
    can_device        = board_get_node_by_handle(MODULE_CAN, &can_handle);
    if (NULL == can_device) {
        amp_error(MOD_STR, "board_get_node_by_handle fail!");
        goto out;
    }
amp_debug(MOD_STR, "can handle %p", can_device);
    recv_param = (can_recv_param_t *)amp_calloc(1, sizeof(*recv_param));
    if (!recv_param) {
        amp_warn(MOD_STR, "allocate memory failed");
        goto out;
    }

    duk_dup(ctx, 1);
    recv_param->js_cb_ref = be_ref(ctx);
    recv_param->can_device = can_device;

    can_recv_task_params.name = "amp can recv task";
    can_recv_task_params.priority = ADDON_TSK_PRIORRITY;
    can_recv_task_params.stack_size = 1024 * 4;

    ret = HAL_ThreadCreate(&can_recv_task, can_recv_routine, recv_param, &can_recv_task_params, &can_recv_task_stack_used);
    if (ret != 0) {
        amp_warn(MOD_STR, "tcp recv task error");
        goto out;
    }

out:
    duk_push_int(ctx, ret);
    return 1;
}

void module_can_register(void)
{
    amp_debug(MOD_STR, "module_can_register");
    duk_context *ctx = be_get_context();

    duk_push_object(ctx);

    duk_push_c_function(ctx, native_open, 1);
    duk_put_prop_string(ctx, -2, "open");

    duk_push_c_function(ctx, native_send, 3);
    duk_put_prop_string(ctx, -2, "send");

    duk_push_c_function(ctx, native_receive, 2);
    duk_put_prop_string(ctx, -2, "receive");

    duk_push_c_function(ctx, native_close, 1);
    duk_put_prop_string(ctx, -2, "close");

    duk_put_prop_string(ctx, -2, "CAN");
}
