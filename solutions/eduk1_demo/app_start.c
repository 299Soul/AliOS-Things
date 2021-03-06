/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/init.h"
#include "board.h"
#include "k1_apps/menu.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <uservice/eventid.h>
// #include "drivers/lcd/st7789v/st7789v_vfs.h"
// #include "drivers/lcd/st7789v/st7789v.h"
#include "netmgr.h"

extern int board_test(void);
extern void vendor_cli_register_init(void);
int g_haasboard_is_k1c = 0;

int bt_connected = 0;
int wifi_connected = 0;
int ip_got_finished = 0;
char eduk1_ip_addr[IPADDR_STR_LEN] = {0};

int demo_main(int argc, char *argv[])
{
    int ret = 0;

    while (1)
    {
        if (wifi_connected && !ip_got_finished)
        {
           ret = netmgr_wifi_get_ip_addr(&eduk1_ip_addr);

           if (ret ==0)
            ip_got_finished = 1;
        }
        aos_msleep(1000);
    }
}

static void entry_func(void *data)
{
    demo_main(0 , NULL);
}

static void wifi_event_cb(uint32_t event_id, const void *param, void *context)
{
    switch (event_id)
    {
        case EVENT_NETMGR_DHCP_SUCCESS:
            wifi_connected = 1;
            aos_task_new("link_dmeo", entry_func, NULL, 6*1024);
            break;

        case EVENT_NETMGR_WIFI_DISCONNECTED:
            wifi_connected = 0;
            break;

        default:
            break;
    }
    return;
}
extern int haasedu_is_k1c();

int application_start(int argc, char **argv)
{
    int ret;

    aos_set_log_level(AOS_LL_INFO);
    event_service_init(NULL);

    netmgr_service_init(NULL);

    /*enable network auto reconnect*/
    netmgr_set_auto_reconnect(NULL, true);

    /*enable auto save wifi config*/
    netmgr_wifi_set_auto_save_ap(true);

    event_subscribe(EVENT_NETMGR_DHCP_SUCCESS, wifi_event_cb, NULL);

    g_haasboard_is_k1c = haasedu_is_k1c();
    sh1106_init();
    menu_init();

    ret = BleCfg_run();
    if (ret) {
        return ret;
    }

    aos_msleep(100);

    (void)BleCfg_recovery_wifi();
    (void)BleCfg_recovery_devinfo();

    return 0;
}
