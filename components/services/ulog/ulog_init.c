/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "ulog/ulog.h"
#include "ulog_api.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "k_api.h"
#include "aos/cli.h"
#include "uagent/uagent.h"
#include "aos/kernel.h"

bool log_init = false;

#ifdef AOS_COMP_CLI

__attribute__((weak)) void update_net_cli(const char cmd, const char *param)
{
    /* Will be overwrite in session_udp, as this implement take effect only the UDP feature support,
       I don't like so many compile switcher in the code which result reader is not so comfortable,
       so weak attribute used here.
    */
}

__attribute__((weak)) void fs_control_cli(const char cmd, const char *param)
{
    /* Will be overwrite in session_fs */
}

__attribute__((weak)) void on_show_ulog_file()
{
    /* Will be overwrite in session_fs */
}

#ifdef ULOG_CONFIG_ASYNC
static void cmd_cli_ulog(char *pwbuf, int blen, int argc, char *argv[])
{
    bool exit_loop = false;
    uint8_t session = 0xFF;
    uint8_t level = 0xFF;
    uint8_t i;

    if (argc == 2) {
        const char* option = argv[1];
        switch (option[0])
        {
        case 'f':
            on_show_ulog_file();
            break;
        case 'x': {
#if ULOG_CONFIG_POP_FS
            char *buf = (char*)aos_malloc(512);
            if (NULL != buf) {
                printf("get ulog list %d\n", aos_get_ulog_list(buf, 512));
                puts(buf);
                aos_free(buf);
            }
#else
            printf("pop log to fs is off");
#endif /* ULOG_CONFIG_POP_FS */
        }
                  break;
        default:
            break;
        }
    }
    for (i = 1; i < argc && !exit_loop; i += 2)
    {
        const char* option = argv[i];
        const char* param = argv[i + 1];
        if (option != NULL && param != NULL && strlen(option) == 1) {
            switch (option[0])
            {
            case 's':/* session */
                session = strtoul(param, NULL, 10);
                break;

            case 'l':/* level */
                level = strtoul(param, NULL, 10);
                break;

            case 'a':/* syslog watcher address */
                update_net_cli(option[0], param);
                break;

            case 't':/* terminate record on fs */
                fs_control_cli(option[0], param);
                break;

            default: /* unknown option */
                exit_loop = true;
                break;
            }
        } else {/* unknown format */
            break;
        }
    }

    if ((session < ulog_session_size) && (level <= LOG_NONE)) {
        ULOG_TRACE_ERR("session %d level %d", session, level==LOG_NONE?LOG_NONE:(level+1));
        on_filter_level_changes((ulog_session_type_t)session, level==LOG_NONE?LOG_NONE:(level+1));
    }
}
#endif

static void sync_log_cmd(char *buf, int len, int argc, char **argv)
{
    if (argc < 2) {
        printf("log level : %s\r\n", get_sync_stop_level());
        return;
    } else if (argc == 2) {
        const char* option = argv[1];
        switch (option[0])
        {
        case 'A':
        case 'F':
        case 'E':
        case 'W':
        case 'I':
        case 'D':
        case 'N':
            on_sync_filter_level_change(ulog_session_std, option[0]);
            break;

        default:
            printf("unknown option %c, only support[AFEWID]\r\n", option[0]);
            break;
        }
    }

}

static struct cli_command ulog_cmd[] = {
    { "loglevel","set sync log level", sync_log_cmd },
#if ULOG_CONFIG_ASYNC
    { "ulog", "ulog [option param]",   cmd_cli_ulog },
#endif
};

#endif /* AOS_COMP_CLI */

#if defined(AOS_COMP_UAGENT) && (ULOG_POP_CLOUD_ENABLE||ULOG_UPLOAD_LOG_FILE)
#include "cJSON.h"

static int on_ulog_handler(void *p, const unsigned short len, void *cmd)
{
    int rc = -1;
    cJSON *root = NULL;
    root = cJSON_Parse(cmd);
    if (NULL != root) {
        cJSON *ulog_cmd = NULL;
#if ULOG_POP_CLOUD_ENABLE
        ulog_cmd = cJSON_GetObjectItem(root, "ulog level");
        if (ulog_cmd != NULL && cJSON_IsNumber(ulog_cmd)) {
            if (ulog_cmd->valueint <= LOG_NONE) {
                on_filter_change(ulog_session_cloud, ulog_cmd->valueint);
                rc = 0;
            }
        }
#endif

#if ULOG_UPLOAD_LOG_FILE
        /* log list requset */
        ulog_cmd = cJSON_GetObjectItem(root, "ulog list");

        if (ulog_cmd != NULL && cJSON_IsString(ulog_cmd)) {
            if (0 == strlen(ulog_cmd->valuestring)) {
                char *buf = (char*)aos_malloc(1024);
                if (NULL != buf) {
                    memset(buf, 0, 1024);
                    if (0 == aos_get_ulog_list(buf, 1024)) {
                        rc = uagent_send(UAGENT_MOD_ULOG, ULOG_LOG_LIST,
                             strlen(buf), buf, send_policy_object);
                    }
                    aos_free(buf);
                }
            }
        }
        /*  request upload ulog file */
        ulog_cmd = cJSON_GetObjectItem(root, "url");
        if (NULL != ulog_cmd && cJSON_IsString(ulog_cmd)) {
            cJSON *ulog_idx = cJSON_GetObjectItem(root, "idx");
            if (NULL != ulog_idx && cJSON_IsNumber(ulog_idx)) {
                rc = http_start(ulog_cmd->valuestring, ulog_idx->valueint);
            }
        }
#endif
        cJSON_Delete(root);
    }
    return rc;
}

#ifndef MOD_VER
#define MOD_VER "1.0.0"
#endif
static uagent_func_node_t ulog_uagent_funclist[] =
{
    {ULOG_LEVEL_CHANGE, "uloglevel",  on_ulog_handler,  NULL,  &ulog_uagent_funclist[1]},
#if ULOG_POP_CLOUD_ENABLE
#if ULOG_UPLOAD_LOG_FILE
    {ULOG_SHOW,         "ulogshow",   NULL,             NULL,  &ulog_uagent_funclist[2]},
#else /* !ULOG_UPLOAD_LOG_FILE */
    {ULOG_SHOW,         "ulogshow",   NULL,             NULL,  NULL},
#endif
#endif /* ULOG_POP_CLOUD_ENABLE */

#if ULOG_UPLOAD_LOG_FILE
    {ULOG_LOG_LIST,     "loglist",    NULL,             NULL,  NULL}
#endif
};

static mod_func_t ulog_uagent_func =
{
    { UAGENT_MOD_ULOG, sizeof(ulog_uagent_funclist) / sizeof(ulog_uagent_funclist[0]), "uLOG", MOD_VER },
    ulog_uagent_funclist
};

#endif /* AOS_COMP_UAGENT */

void ulog_init(void)
{
    if (!log_init) {
        log_init_mutex();
#ifdef ULOG_CONFIG_ASYNC
        ulog_async_init();
#endif
        on_filter_level_changes(ulog_session_size, LOG_NONE);
#ifdef AOS_COMP_CLI
        aos_cli_register_commands(&ulog_cmd[0], sizeof(ulog_cmd) / sizeof(struct cli_command));
#endif
        log_init = true;
#if defined(AOS_COMP_UAGENT) && (ULOG_POP_CLOUD_ENABLE||ULOG_UPLOAD_LOG_FILE)

        uagent_func_node_t * p = ulog_uagent_func.header;
        if (NULL != p) {
            while (NULL != p) {
                if (0 != uagent_register(UAGENT_MOD_ULOG, "uLog", MOD_VER, p->func, p->func_name, p->service, p->argu)) {
                    LOGE("ULOG", "register into uagent fail");
                }
                p = p->next;
            }
        } else {
            LOGE("ULOG", "register into uagent fail, table is null");
        }
#endif
    }
}

