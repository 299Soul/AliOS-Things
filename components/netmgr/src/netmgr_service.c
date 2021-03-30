/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <stdlib.h>
#include <netmgr.h>
#include <netmgr_wifi.h>
#include <aos/list.h>
#include <aos/kv.h>

#include "activation.h"

#define KV_VAL_LENGTH       32

struct hdl_info;
typedef struct hdl_info {
    slist_t next;
    char* file_name;
    netmgr_hdl_t hdl;
    netmgr_type_t type;
} hdl_info_t;

#define TAG "netmgr_service"

static slist_t g_hdl_list_head;

int netmgr_wifi_set_ip_mode(netmgr_wifi_ip_mode_t mode);
netmgr_wifi_ip_mode_t netmgr_wifi_get_ip_mode(void);
int netmgr_wifi_get_ip_stat(char* ip_addr, char* mask, char* gw, char* dns_server);
void netmgr_wifi_auto_reconnect(int enable);
int  netmgr_wifi_get_wifi_state();
int netmgr_wifi_set_static_ip_stat(const char* ip_addr, const char* mask, const char* gw, const char* dns_server);

#ifdef CONFIG_KV_SMART
char *netmgr_kv_get(const char *key)
{
    static char str[KV_VAL_LENGTH];
    int len = KV_VAL_LENGTH;

    memset(str, 0, KV_VAL_LENGTH);
    aos_kv_get(key, str, &len);

    return str;
}

int netmgr_kv_getint(const char *key)
{
    int val, ret;

    ret = aos_kv_getint(key, &val);

    if (ret == 0) {
        return val;
    }

    return 0;
}
#endif

static int add_hdl_info(int fd, char* file_name, netmgr_type_t type)
{
    hdl_info_t* cur;

    printf("%s:%d\n", __func__, __LINE__);
    cur = malloc(sizeof(hdl_info_t));
    if(cur == NULL) {
        return -1;
    }
    memset(cur, 0, sizeof(hdl_info_t));
    printf("%s:%d\n", __func__, __LINE__);

    cur->hdl = fd;
    cur->file_name = strdup(file_name);
    cur->type = type;

    slist_add_tail(&cur->next, &g_hdl_list_head);

    return 0;
}

static int del_hdl_by_name(char* file_name)
{
    hdl_info_t* cur;
    int found = 0;

    slist_for_each_entry(&g_hdl_list_head, cur, hdl_info_t, next) {
        if(strcmp(cur->file_name, file_name) == 0) {
            found = 1;
            break;
        }
    }

    if(1 == found) {
        free(cur->file_name);
        slist_del(&cur->next, &g_hdl_list_head);
        return 0;
    } else {
        return -1;
    }
}

static netmgr_hdl_t  get_hdl_by_name(char* file_name)
{
    hdl_info_t* cur;
    int found = 0;

    slist_for_each_entry(&g_hdl_list_head, cur, hdl_info_t, next) {
        if(strcmp(cur->file_name, file_name) == 0) {
            found = 1;
            break;
        }
    }

    if(1 == found) {
        return cur->hdl;
    } else {
        return -1;
    }
}

/**
 * @brief  create & init the netmgr uservice
 * @param  [in] task
 * @return
 */
void netmgr_wifi_cli_register(void);
int netmgr_service_init(utask_t *task)
{
    return netmgr_add_dev("/dev/wifi0");
}

/**
 * @brief  destroy & uninit the netmgr uservice
 * @return
 */
void netmgr_service_deinit()
{
    hdl_info_t * cur;
    slist_t *tmp;

    slist_for_each_entry_safe(&g_hdl_list_head, tmp, cur, hdl_info_t , next) {
        del_hdl_by_name(cur->file_name);
        close(cur->hdl);
    }
}

int netmgr_add_dev(const char* name)
{
    int fd;
    fd = open(name, O_RDWR);
    if(fd >= 0) {
        netmgr_wifi_cli_register();
        slist_init(&g_hdl_list_head);
        add_hdl_info(fd, (char *)name, NETMGR_TYPE_WIFI);
        netmgr_wifi_init(fd);
        return 0;
    } else {
        return -1;
    }
}

netmgr_hdl_t netmgr_get_dev(const char* name)
{
    return get_hdl_by_name((char *)name);
}

int netmgr_set_ifconfig(netmgr_hdl_t hdl, netmgr_ifconfig_info_t* info)
{
    if(info != NULL) {
        if(info->dhcp_en == true) {
            netmgr_wifi_set_ip_mode(NETMGR_WIFI_IP_MODE_AUTO);
        } else {
            netmgr_wifi_set_ip_mode(NETMGR_WIFI_IP_MODE_STATIC);
        }
        return netmgr_wifi_set_static_ip_stat(info->ip_addr, info->mask, info->gw, info->dns_server);
    }
    return -1;
}

int netmgr_get_ifconfig(netmgr_hdl_t hdl, netmgr_ifconfig_info_t* info)
{
    if(info != NULL) {
        netmgr_wifi_ip_mode_t mode = netmgr_wifi_get_ip_mode();

        if(mode == NETMGR_WIFI_IP_MODE_AUTO) {
            info->dhcp_en = true;
        } else {
            info->dhcp_en = false;
        }
        return netmgr_wifi_get_ip_stat(info->ip_addr, info->mask, info->gw, info->dns_server);
    } else {
        return -1;
    }
}

void netmgr_set_auto_reconnect(netmgr_hdl_t hdl, bool enable)
{
    netmgr_wifi_auto_reconnect(enable);
}


int netmgr_get_config(netmgr_hdl_t hdl, netmgr_config_t* config)
{
    if(config != NULL) {
        return netmgr_wifi_get_config(hdl, &(config->config.wifi_config));
    }
    return -1;
}

int netmgr_del_config(netmgr_hdl_t hdl, netmgr_del_config_t* config)
{
    if(config != NULL) {
        return netmgr_wifi_del_config(hdl, config->config.ssid);
    }
    return -1;
}


int netmgr_get_state(netmgr_hdl_t hdl)
{
    return netmgr_wifi_get_wifi_state(hdl);
}

int netmgr_connect(netmgr_hdl_t hdl, netmgr_connect_params_t* params)
{
    if(params != NULL) {
        return netmgr_wifi_connect(hdl, &(params->params.wifi_params));
    } else {
        return -1;
    }
}

int netmgr_disconnect(netmgr_hdl_t hdl)
{
    return netmgr_wifi_disconnect(hdl);
}

int netmgr_save_config(netmgr_hdl_t hdl)
{
    return netmgr_wifi_save_config(hdl);
}

int netmgr_set_connect_params(netmgr_hdl_t hdl, netmgr_connect_params_t* params)
{
    //TODO
    return 0;
}

int netmgr_set_msg_cb(netmgr_hdl_t hdl, netmgr_msg_cb_t cb)
{
    netmgr_wifi_set_msg_cb(hdl, cb);
    return 0;
}

int netmgr_del_msg_cb(netmgr_hdl_t hdl, netmgr_msg_cb_t cb)
{
    netmgr_wifi_del_msg_cb(hdl, cb);
    return 0;
}
