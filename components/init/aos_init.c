/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if(CONFIG_AOS_LWIP > 0)
#include <sys/socket.h>
#endif
#include "aos/init.h"
#if(AOS_COMP_CLI > 0)
#include "aos/cli.h"
#endif
#include "ulog/ulog.h"
// #include "uagent.h"
#include "aos/kernel.h"
#if(AOS_COMP_WIFI > 0)
#include "aos/hal/wifi.h"
#endif

#ifdef AOS_COMP_PWRMGMT
#include <pwrmgmt_api.h>
#endif

#ifdef AOS_COMP_LITTLEFS
#include "littlefs.h"
#endif

#ifdef AOS_COMP_FATFS
#include "fatfs.h"
#endif

#if AOS_COMP_DEBUG
#include "aos/debug.h"
#endif

#ifdef AOS_COMP_UND
#include "und/und.h"
#endif

#ifdef IPERF_ENABLED
extern int iperf_cli_register(void);
#endif

#ifdef PING_ENABLED
extern int ping_cli_register(void);
#endif

extern int  vfs_init(void);
#ifdef AOS_LOOP
#include "aos/yloop.h"
extern aos_loop_t aos_loop_init(void);
#endif
extern int32_t kv_init(void);
extern void ota_service_init(void);
extern void dumpsys_cli_init(void);

#ifdef WITH_SAL
extern int sal_device_init(void);
#endif

#ifdef WITH_MAL
extern int mal_device_init(void);
#endif

#if defined(CONFIG_DRV_VFS) && (CONFIG_DRV_VFS == 1)
int u_driver_entry(char* string);
int u_post_driver_entry(char* string);
#endif

#ifdef AOS_BINS

extern void *kmbins_tbl[];
extern char  app_info_addr;
extern k_mm_head  *g_kmm_head;
struct m_app_info_t *app_info = (struct m_app_info_t *) &app_info_addr;

static void app_pre_init(void)
{
    memcpy((void *)(app_info->data_ram_start), (void *)(app_info->data_flash_begin),
           app_info->data_ram_end - app_info->data_ram_start);

    memset((void *)(app_info->bss_start), 0, app_info->bss_end - app_info->bss_start);

    krhino_add_mm_region(g_kmm_head, (void *)(app_info->heap_start),
                         app_info->heap_end - app_info->heap_start);
}
#endif

#if (AOS_COMP_CLI > 0)
#ifndef CONFIG_NO_LWIP
#if (CONFIG_AOS_LWIP > 0)
static void udp_cmd(char *buf, int len, int argc, char **argv)
{
    struct sockaddr_in saddr;

    if (argc < 4) {
        return;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2]));
    saddr.sin_addr.s_addr = inet_addr(argv[1]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        aos_cli_printf("error creating socket!\n");
        return;
    }

    int ret = sendto(sockfd, argv[3], strlen(argv[3]), 0,
                     (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret < 0) {
        aos_cli_printf("error send data %d!\n", ret);
    }

    close(sockfd);
}

struct cli_command  tcpip_cli_cmd[] = {
    /* net */
    {"udp",         "[ip] [port] [string data] send udp data", udp_cmd},
};

static void tcpip_cli_init(void)
{
    aos_cli_register_commands(&tcpip_cli_cmd[0],sizeof(tcpip_cli_cmd) / sizeof(struct cli_command));
}

#ifdef AOS_NET_WITH_WIFI
static uint8_t hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return 0;
}

static void hexstr2bin(const char *macstr, uint8_t *mac, int len)
{
    int i;
    for (i=0;i < len && macstr[2 * i];i++) {
        mac[i] = hex(macstr[2 * i]) << 4;
        mac[i] |= hex(macstr[2 * i + 1]);
    }
}

static void wifi_debug_cmd(char *buf, int len, int argc, char **argv)
{
    hal_wifi_start_debug_mode(NULL);
}

static void mac_cmd(char *buf, int len, int argc, char **argv)
{
    uint8_t mac[6];

    if (argc == 1)
    {
        hal_wifi_get_mac_addr(NULL, mac);
        aos_cli_printf("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else if(argc == 2)
    {
        hexstr2bin(argv[1], mac, 6);
        hal_wifi_set_mac_addr(NULL, mac);
        aos_cli_printf("Set MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        aos_cli_printf("invalid cmd\r\n");
    }
}


static struct cli_command  wifi_cli_cmd[] = {
    { "wifi_debug", "wifi debug mode", wifi_debug_cmd },
    { "mac", "get/set mac", mac_cmd },
};

static void hal_wifi_cli_init(void)
{
    aos_cli_register_commands(&wifi_cli_cmd[0],sizeof(wifi_cli_cmd) / sizeof(struct cli_command));
}
#endif /* AOS_NET_WITH_WIFI */
#endif /* CONFIG_AOS_LWIP */
#endif /*!defined CONFIG_NO_LWIP */

void cli_service_init(kinit_t *kinit)
{
    if (kinit->cli_enable)
    {
        aos_cli_init();
        /*kernel basic cmds reg*/
#ifndef CONFIG_NO_LWIP
#if(CONFIG_AOS_LWIP > 0)
        tcpip_cli_init();
#ifdef AOS_NET_WITH_WIFI
        hal_wifi_cli_init();
#endif
#ifdef IPERF_ENABLED
        iperf_cli_register();
#endif

#ifdef PING_ENABLED
        ping_cli_register();
#endif

#endif
#endif

    }
    return;
}

#endif

void aos_show_welcome(void)
{
    puts("             Welcome to AliOS Things           ");
#ifdef CONFIG_AOS_INIT_WELCOME
    puts("       �����������[ �����[     �����[ �������������[ ���������������[     ");
    puts("      �����X�T�T�����[�����U     �����U�����X�T�T�T�����[�����X�T�T�T�T�a     ");
    puts("      ���������������U�����U     �����U�����U   �����U���������������[     ");
    puts("      �����X�T�T�����U�����U     �����U�����U   �����U�^�T�T�T�T�����U     ");
    puts("      �����U  �����U���������������[�����U�^�������������X�a���������������U     ");
    puts("      �^�T�a  �^�T�a�^�T�T�T�T�T�T�a�^�T�a �^�T�T�T�T�T�a �^�T�T�T�T�T�T�a     ");
    puts("�����������������[�����[  �����[�����[�������[   �����[ �������������[ ���������������[");
    puts("�^�T�T�����X�T�T�a�����U  �����U�����U���������[  �����U�����X�T�T�T�T�a �����X�T�T�T�T�a");
    puts("   �����U   ���������������U�����U�����X�����[ �����U�����U  �������[���������������[");
    puts("   �����U   �����X�T�T�����U�����U�����U�^�����[�����U�����U   �����U�^�T�T�T�T�����U");
    puts("   �����U   �����U  �����U�����U�����U �^���������U�^�������������X�a���������������U");
    puts("   �^�T�a   �^�T�a  �^�T�a�^�T�a�^�T�a  �^�T�T�T�a �^�T�T�T�T�T�a �^�T�T�T�T�T�T�a");
#endif
}

int aos_components_init(kinit_t *kinit)
{
#ifdef AOS_COMP_VFS
    vfs_init();
#endif

#if defined(CONFIG_DRV_VFS) && (CONFIG_DRV_VFS == 1)
    u_driver_entry("aos_components_init");
#endif

#ifdef AOS_COMP_UAGENT
    uagent_init();
#endif

#if AOS_COMP_CLI
    cli_service_init(kinit);
#endif

#if defined(CONFIG_DRV_VFS) && (CONFIG_DRV_VFS == 1)
    u_post_driver_entry("aos_components_init");
#endif

#ifdef AOS_COMP_TRACE
    TRACE_INIT();
#endif

#ifdef AOS_COMP_NFTL
    nftl_init();
#endif

#ifdef AOS_COMP_FATFS
    fatfs_register();
#endif

#ifdef AOS_COMP_LITTLEFS
    littlefs_register();
#endif

#ifdef AOS_COMP_ULOG
    ulog_init();
#endif

#ifdef AOS_COMP_KV
    kv_init();
#endif

#ifdef WITH_SAL
    sal_device_init();
#endif

#ifdef WITH_MAL
    mal_device_init();
#endif

#ifdef AOS_LOOP
    aos_loop_init();
#endif

#ifdef AOS_UOTA
    ota_service_init();
#endif

#ifdef AOS_COMP_SENSOR
    sensor_init();
#endif

#ifdef AOS_COMP_PWRMGMT
    pwrmgmt_init();
#endif

#if AOS_COMP_DEBUG
    aos_debug_init();
#endif

#ifdef AOS_COMP_UND
    und_init();
#endif

    aos_show_welcome();

#ifdef AOS_BINS
    app_pre_init();

    if (app_info->app_entry) {
        app_info->app_entry((void *)kmbins_tbl, 0, NULL);
    }
#endif

    return 0;
}

