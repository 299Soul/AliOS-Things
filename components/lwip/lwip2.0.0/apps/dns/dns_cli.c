/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */
#if AOS_COMP_CLI
#include <string.h>
#include <aos/cli.h>
#include <lwip/netdb.h>

static void dns_command(char *buffer, int32_t buf_len, int32_t argc, char **argv);

struct cli_command dns_cmd[] = {
    { "dns", "dns app", dns_command},
};

static void dns_help_command(void)
{
    LWIP_DEBUGF( DNSCLI_DEBUG, ("Usage: dns destination\n"));
    LWIP_DEBUGF( DNSCLI_DEBUG, ("Eample:\n"));
    LWIP_DEBUGF( DNSCLI_DEBUG, ("dns www.aliyun.com\n"));
}

static void dns_req_command(char *name)
{
    struct hostent *hostent;
    uint8_t index = 0;

    hostent = lwip_gethostbyname(name);

    if (hostent) {
        LWIP_DEBUGF( DNSCLI_DEBUG, ("hostent.h_name %s\n", hostent->h_name));
        if (hostent->h_addr_list != NULL) {
            for (index = 0; hostent->h_addr_list[index]; index++) {
                LWIP_DEBUGF( DNSCLI_DEBUG, ("hostent.h_addr_list[%i]->== %s\n",
                        index, ipaddr_ntoa((ip_addr_t *)hostent->h_addr_list[index])));
            }
        }
    } else {
        LWIP_DEBUGF( DNSCLI_DEBUG, ("lwip_gethostbyname failed, try again\n"));
    }
}

static void dns_command(char *buffer, int32_t buf_len, int32_t argc, char **argv)
{
    if (argc < 2) {
        LWIP_DEBUGF( DNSCLI_DEBUG, ("%s, invalid command\n", __func__));
        dns_help_command();
        return;
    }

    if (strcmp(argv[1], "-h") == 0) {
        dns_help_command();
    } else {
        dns_req_command(argv[1]);
    }
}

int32_t dns_cli_register(void)
{
    if (0 == aos_cli_register_commands(dns_cmd, 1)) {
        return 0;
    }

    return -1;
}
#endif /* AOS_COMP_CLI */
