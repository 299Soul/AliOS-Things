/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "amp_platform.h"
#include "aos_system.h"
#include "amp_defines.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_ntp_api.h"
#include "aiot_mqtt_api.h"
#include "module_aiot.h"
#include "quickjs_addon_common.h"

#define MOD_STR "AIOT_NTP"

/* ntp timestamp */
extern int64_t g_ntp_time;
extern int64_t g_up_time;

typedef struct {
    JSValue js_cb_ref;
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t msecond;
    uint64_t timestamp;
} aiot_ntp_notify_param_t;

static void aiot_device_ntp_notify(void *pdata)
{
    aiot_ntp_notify_param_t *param = (aiot_ntp_notify_param_t *)pdata;
    JSContext *ctx = js_get_context();

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "year", JS_NewInt32(ctx, param->year));
    JS_SetPropertyStr(ctx, obj, "month", JS_NewInt32(ctx, param->month));
    JS_SetPropertyStr(ctx, obj, "day", JS_NewInt32(ctx, param->day));
    JS_SetPropertyStr(ctx, obj, "hour", JS_NewInt32(ctx, param->hour));
    JS_SetPropertyStr(ctx, obj, "minute", JS_NewInt32(ctx, param->minute));
    JS_SetPropertyStr(ctx, obj, "second", JS_NewInt32(ctx, param->second));
    JS_SetPropertyStr(ctx, obj, "msecond", JS_NewInt32(ctx, param->msecond));
    JS_SetPropertyStr(ctx, obj, "timestamp", JS_NewInt64(ctx, param->timestamp));

    JSValue val = JS_Call(ctx, param->js_cb_ref, JS_UNDEFINED, 1, &obj);
    JS_FreeValue(ctx, val);
    JS_FreeValue(ctx, obj);
    JS_FreeValue(ctx, param->js_cb_ref);

    amp_free(param);
}

static void aiot_app_ntp_recv_handler(void *handle, const aiot_ntp_recv_t *packet, void *userdata)
{
    int res = -1;

    aiot_ntp_notify_param_t *ntp_params;

    switch (packet->type)
    {
    case AIOT_NTPRECV_LOCAL_TIME:
        /* print topic name and topic message */
        amp_debug(MOD_STR, "year: %d, month: %d, day: %d, hour: %d, min: %d, sec: %d, msec: %d, timestamp: %d",
                packet->data.local_time.year,
                packet->data.local_time.mon,
                packet->data.local_time.day,
                packet->data.local_time.hour,
                packet->data.local_time.min,
                packet->data.local_time.sec,
                packet->data.local_time.msec,
                packet->data.local_time.timestamp
        );

        g_ntp_time = packet->data.local_time.timestamp;
        g_up_time = aos_now_ms();

        ntp_params = (aiot_ntp_notify_param_t *)userdata;

        ntp_params->year        = packet->data.local_time.year;
        ntp_params->month       = packet->data.local_time.mon;
        ntp_params->day         = packet->data.local_time.day;
        ntp_params->hour        = packet->data.local_time.hour;
        ntp_params->minute      = packet->data.local_time.min;
        ntp_params->second      = packet->data.local_time.sec;
        ntp_params->msecond     = packet->data.local_time.msec;
        ntp_params->timestamp   = packet->data.local_time.timestamp;

        break;
    default:
        amp_free(ntp_params);
        return;
    }

    amp_task_schedule_call(aiot_device_ntp_notify, ntp_params);

    res = aiot_ntp_deinit(&handle);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp deinit failed");
    }
}

static void aiot_app_ntp_event_handler(void *handle, const aiot_ntp_event_t *event, void *userdata)
{
    switch (event->type)
    {
    case AIOT_NTPEVT_INVALID_RESPONSE:
        /* print topic name and topic message */
        amp_debug(MOD_STR, "ntp receive data invalid");
        break;
    case AIOT_NTPEVT_INVALID_TIME_FORMAT:
        amp_debug(MOD_STR, "ntp receive data error");
        break;
    default:
        break;
    }
}

/* ntp service */
int32_t aiot_amp_ntp_service(void *mqtt_handle, JSValue js_cb_ref)
{
    int32_t res = STATE_SUCCESS;
    int32_t time_zone = 8;
    void *ntp_handle = NULL;
    aiot_ntp_notify_param_t *ntp_params;

    if (mqtt_handle == NULL) {
        amp_error(MOD_STR, "ntp service init failed");
        return -1;
    }

    ntp_handle = aiot_ntp_init();
    if (ntp_handle == NULL) {
        amp_error(MOD_STR, "ntp service init failed");
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_MQTT_HANDLE, (void *)mqtt_handle);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp set mqtt handle failed");
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_TIME_ZONE, (void *)&time_zone);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp set time zone failed");
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_RECV_HANDLER, (void *)aiot_app_ntp_recv_handler);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp set receive handler failed");
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_EVENT_HANDLER, (void *)aiot_app_ntp_event_handler);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp set event handler failed");
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    ntp_params = amp_malloc(sizeof(aiot_ntp_notify_param_t));
    if (!ntp_params) {
        amp_error(MOD_STR, "alloc device_ntp_notify_param_t fail");
        return;
    }
    memset(ntp_params, 0, sizeof(aiot_ntp_notify_param_t));

    ntp_params->js_cb_ref = js_cb_ref;
    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_USERDATA, ntp_params);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp set user data failed");
        aiot_ntp_deinit(&ntp_handle);
        amp_free(ntp_params);
        return -1;
    }

    res = aiot_ntp_send_request(ntp_handle);
    if (res < STATE_SUCCESS) {
        amp_error(MOD_STR, "ntp send request failed");
        aiot_ntp_deinit(&ntp_handle);
        amp_free(ntp_params);
        return -1;
    }

    return res;
}