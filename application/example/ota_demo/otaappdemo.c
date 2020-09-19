/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ulog/ulog.h"
#include "aos/yloop.h"
#include <aos/cli.h>

#include "linkkit/infra/infra_compat.h"
#include "linkkit/mqtt_api.h"
#include "netmgr.h"

#ifdef WITH_SAL
#include <atcmd_config_module.h>
#endif

#include "ota/ota_agent.h"

#ifdef CSP_LINUXHOST
#include <signal.h>
#endif

static char mqtt_started = 0;
static ota_service_t ctx = {0};

static int ota_upgrade_cb(ota_service_t* pctx, char *ver, char *url)
{
    int ret = -1;
    LOG("ota version:%s is coming, if OTA upgrade or not ?\n", ver);
    void *thread = NULL;
    if(pctx != NULL) {
        ret = ota_thread_create(&thread, (void *)ota_service_start, (void *)pctx, NULL, 1024 * 6);
    }
    if (ret < 0) {
        LOG("ota thread err;%d ", ret);
    }
    return ret;
}

void mqtt_event_cb(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    LOG("msg->event_type : %d", msg->event_type);
}

int mqtt_main(void *param)
{
    void             *pclient = NULL;
    int               res = 0;
    int               loop_cnt = 0;
    iotx_mqtt_param_t mqtt_params;

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));
    mqtt_params.handle_event.h_fp = mqtt_event_cb;
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        LOG("MQTT construct Err");
        return -1;
    }
    while (1) {
        IOT_MQTT_Yield(pclient, 200);
        aos_msleep(1000);
    }
    return 0;
}

static void wifi_service_event(input_event_t *event, void *priv_data)
{
   if (event->type != EV_WIFI) {
        return;
    }
    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }
    netmgr_ap_config_t config;
    memset(&config, 0, sizeof(netmgr_ap_config_t));
    netmgr_get_ap_config(&config);
    LOG("wifi_service_event config.ssid %s", config.ssid);
    if (strcmp(config.ssid, "adha") == 0 || strcmp(config.ssid, "aha") == 0) {
        return;
    }
    if (!mqtt_started) {
        aos_task_new("iotx_example", mqtt_main, (void *)NULL, 1024 * 6);
        mqtt_started = 1;
    }
}
#ifdef AOS_COMP_CLI
static void get_identity()
{
    char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = {0};
    char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = {0};
    char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = {0};
    HAL_GetProductKey(_product_key);
    HAL_GetDeviceName(_device_name);
    HAL_GetDeviceSecret(_device_secret);
    aos_cli_printf("ProductKey:%s, DeviceName:%s DeviceSecret:%s \n", _product_key, _device_name, _device_secret);
}

static void cmd_identity(char *pwbuf, int blen, int argc, char **argv)
{
    const char *rtype = argc > 1 ? argv[1] : "";
    if (strcmp(rtype, "get") == 0) {
        get_identity();
    } else if (strcmp(rtype, "set") == 0) {
        if (argc == 4) {
            HAL_SaveDeviceIdentity(NULL, NULL, argv[2], argv[3]);
        } else if (argc == 5) {
            HAL_SaveDeviceIdentity(argv[2], argv[3], argv[4], "");
        } else if (argc == 6) {
            HAL_SaveDeviceIdentity(argv[2], argv[3], argv[4], argv[5]);
        } else {
            aos_cli_printf("arg number err! usage:\n");
            aos_cli_printf("identity set {pk} {ps} {dn} [ds] | identity set {dn} {ds}\n");
        }
    } else if (strcmp(rtype, "clear") == 0) {
        HAL_ClearDeviceIdentity();
    } else {
        aos_cli_printf("usage:\n");
        aos_cli_printf("identity [set pk ps dn ds | set dn ds | get | clear]\n");
    }
}

static void cmd_download(char *pwbuf, int blen, int argc, char **argv)
{
    ota_boot_param_t param = {0};
    if (argc <= 2) {
        LOG("ota_download [url] [size]");
        return;
    }
    LOG("ota download url:%s size:%d \n", argv[1], argv[2]);
    param.len = strtoul(argv[2], NULL, 10);
    ota_int(&param);
    ota_download_init(&ctx);
    ota_download_start(&ctx, argv[1]);
}

static void cmd_diff_ota(char *pwbuf, int blen, int argc, char **argv)
{
    ota_boot_param_t param = {0};
    if (argc <= 1) {
        LOG("ota_diff [size]");
        return;
    }
    LOG("ota diff size:%d \n", argv[1]);
    param.len = strtoul(argv[1], NULL, 10);
    param.upg_flag = 0xB44B;
    ota_hal_boot(&param);
}

static void cmd_mcu_ota(char *pwbuf, int blen, int argc, char **argv)
{
    ota_boot_param_t param = {0};
    unsigned char mcu_type = atoi(argv[1]);
    if (argc <= 3) {
        LOG("ota_mcu [size] [version]");
        return;
    }
    LOG("ota mcu size:%d version:%s \n", argv[1], argv[2]);
    param.len = strtoul(argv[2], NULL, 10);
#if defined OTA_CONFIG_MCU_CAN
    ota_mcu_can_upgrade(param.len, argv[3], NULL);
#elif defined OTA_CONFIG_MCU_YMODEM
    ota_mcu_ymodem_upgrade(param.len, argv[3], NULL);
#endif
}

static struct cli_command ota_cmd[4] = {
    {
        .name = "ota_download",
        .help = "ota_download [url]",
        .function = cmd_download
    },
    {
        .name = "ota_diff",
        .help = "ota diff",
        .function = cmd_diff_ota
    },
    {
        .name = "ota_mcu",
        .help = "ota mcu",
        .function = cmd_mcu_ota
    },
    {
        .name = "identity",
        .help = "identity [set pk ps dn ds | set dn ds | get | clear]",
        .function = cmd_identity
    }
};
#endif

int mqtt_connected_cb(void)
{
    LOG("Connect cloud successfully, OTA init...\n");
    memset(ctx.pk, 0, sizeof(ctx.pk));
    memset(ctx.dn, 0, sizeof(ctx.dn));
    memset(ctx.ds, 0, sizeof(ctx.ds));
    memset(ctx.ps, 0, sizeof(ctx.ps));

    HAL_GetProductKey(ctx.pk);
    HAL_GetDeviceName(ctx.dn);
    HAL_GetDeviceSecret(ctx.ds);
    HAL_GetProductSecret(ctx.ps);
    ota_service_init(&ctx);
    return 0;
}

int application_start(int argc, char *argv[])
{
#ifdef CSP_LINUXHOST
    signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WITH_SAL
    sal_device_config_t data = {0};

    data.uart_dev.port = 1;
    data.uart_dev.config.baud_rate = 115200;
    data.uart_dev.config.data_width = DATA_WIDTH_8BIT;
    data.uart_dev.config.parity = NO_PARITY;
    data.uart_dev.config.stop_bits  = STOP_BITS_1;
    data.uart_dev.config.flow_control = FLOW_CONTROL_DISABLED;
    data.uart_dev.config.mode = MODE_TX_RX;

    sal_add_dev(NULL, &data);
    sal_init();
#endif

    aos_set_log_level(AOS_LL_DEBUG);
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    netmgr_init();
#ifdef AOS_COMP_CLI
    int i = 0;
    for(i = 0; i < sizeof(ota_cmd) / sizeof(struct cli_command); i++) {
        aos_cli_register_command(&ota_cmd[i]);
    }
#endif
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);
    IOT_RegisterCallback(ITE_MQTT_CONNECT_SUCC, mqtt_connected_cb);
    ota_register_cb(&ctx, OTA_CB_ID_UPGRADE, (void*)ota_upgrade_cb);

    netmgr_start(false);
#if defined (CSP_LINUXHOST) && !defined (WITH_SAL)
    aos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP, 0);
#endif
    aos_loop_run();
    return 0;
}
