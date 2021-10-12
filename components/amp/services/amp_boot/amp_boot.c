/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/kernel.h"
#include "aos/kv.h"
#include "amp_boot.h"

#define MOD_STR "amp_boot"

#define AMP_BOOT_WAIT_TIME 1000
#define AMP_BOOT_MAX_KV_LEN 256

#define AMP_BOOT_KV_VALUE_END_STR "[amp_value_end]\n"

static char g_amp_boot_sync[] = "amp_boot";

void amp_boot_init()
{
#if ((!defined(BOARD_HAAS100)) && (!defined(BOARD_HAASEDUK1)))
    amp_boot_uart_init();
#endif
}

void amp_boot_deinit()
{
#if ((!defined(BOARD_HAAS100)) && (!defined(BOARD_HAASEDUK1)))
    amp_boot_uart_deinit();
#endif
}

void print_usage()
{

}

void aos_boot_delay(uint32_t ms)
{
    uint64_t begin_ms = aos_now_ms();

    while (1) {
        uint64_t now_ms = aos_now_ms();
        if (now_ms - begin_ms >= ms) {
            break;
        }
    }
}

void amp_boot_loop(int line)
{
    while (1) {
        aos_boot_delay(500);
    }
}

void amp_boot_flash_js()
{
    amp_recovery_appbin();
}

void amp_boot_flash_kv()
{
    char key[64]  = {0};
    char kv_value_end_str[] = AMP_BOOT_KV_VALUE_END_STR;
    uint8_t *data = NULL;
    int       num = 0;
    int       ret = 0;

    aos_boot_delay(5);
    num = amp_boot_uart_recv_line(key, 64, 50000);
    if ((num == 0) || (num >= 64)) {
        amp_error(MOD_STR, "recv key error %d\n", num);
        return;
    }
    aos_boot_delay(5);
    key[num] = '\0';
    amp_debug(MOD_STR, "[key]=%s\n", key);

    for (int i = 0; i < 1; i++) {
        amp_boot_uart_send_str("[value]=");
    }
    aos_boot_delay(5);
    amp_debug(MOD_STR, "[value]=");

    data = (uint8_t *)amp_malloc(AMP_BOOT_MAX_KV_LEN);
    if (data == NULL) {
        return;
    }
    memset(data, 0, AMP_BOOT_MAX_KV_LEN);
    num = 0;
    while (num < AMP_BOOT_MAX_KV_LEN) {
        unsigned char c = 0;
        if (1 == amp_boot_uart_recv_byte(&c)) {
            data[num] = (uint8_t)c;
            num ++;
            if ((c == '\n') && (num > strlen(kv_value_end_str))) {
                if (strncmp(kv_value_end_str, (char *)&data[num - strlen(kv_value_end_str)], strlen(kv_value_end_str)) == 0) {
                    break;
                }
            }
        }

        aos_boot_delay(5);
    }
    amp_debug(MOD_STR, "\n");
    num -= strlen(kv_value_end_str);

    data[num] = 0;

    amp_debug(MOD_STR, "begin write kv %s, size %d value 0x%x, 0x%x 0x%x\n", key, num, (uint32_t)(*(uint16_t *)(data)), (uint32_t)data[0], (uint32_t)data[1]);

    ret = aos_kv_set(key, data, num, 1);
    if (ret == 0) {
        amp_boot_uart_send_str("[kvok]");
        amp_debug(MOD_STR, "write kv [%s] success\n", key);
    }
    amp_free(data);
}

void amp_boot_cli_menu()
{
    print_usage();
    amp_debug(MOD_STR, "\r\namp boot# ");

    while (1) {
        int boot_cmd = 0;
        boot_cmd = amp_boot_get_cmd(3000);

        switch (boot_cmd) {
        case AMP_BOOT_CMD_NULL:
            amp_debug(MOD_STR, "no command 3 seconds \r\n");
            return;
            break;

        case AMP_BOOT_CMD_ERROR:
            amp_debug(MOD_STR, "command not supported  \r\n");
            break;

        case AMP_BOOT_CMD_EXIT:
            amp_debug(MOD_STR, "aos boot finished \r\n");
            return;

        case AMP_BOOT_CMD_FLASH_JS:
            amp_boot_cmd_begin(AMP_BOOT_CMD_FLASH_JS);
            amp_boot_flash_js();
            amp_boot_cmd_end(AMP_BOOT_CMD_FLASH_JS);
            break;

        case AMP_BOOT_CMD_FLASH_KV:
            amp_boot_cmd_begin(AMP_BOOT_CMD_FLASH_KV);
            amp_boot_flash_kv();
            amp_boot_cmd_end(AMP_BOOT_CMD_FLASH_KV);
            break;

        default:
            print_usage();
            return;
        }
        amp_debug(MOD_STR, "\r\namp boot# ");
    }
}

bool amp_boot_cli_in()
{
    int   begin_num = 0;
    int           i = 0;
    unsigned char c = 0;
    uint64_t begin_time = aos_now_ms();

    amp_boot_uart_send_str("amp shakehand begin...\n");

    while (1) {
        c = 0;
        if ((amp_boot_uart_recv_byte(&c) != 1) || (c == 0)) {
            aos_boot_delay(5);
            if ((aos_now_ms() - begin_time) > AMP_BOOT_WAIT_TIME) {
                return false;
            }
            continue;
        }

        if (c == 0xA5) {
            begin_num ++;
            amp_boot_uart_send_byte(0x5A);
        } else {
            begin_num = 0;
        }

        if (begin_num == 4) {
            break;
        }
    }

    while (1) {
        c = 0;
        aos_boot_delay(5);

        if (amp_boot_uart_recv_byte(&c) == 1) {
            if (c != g_amp_boot_sync[i]) {
                i = 0;
            } else {
                i ++;
            }

            if (i >= strlen(g_amp_boot_sync)) {
                break;
            }
        }

        if ((aos_now_ms() - begin_time) > AMP_BOOT_WAIT_TIME) {
            return false;
        }
    }
    amp_debug(MOD_STR, "amp shakehand success\n");

    return true;
}

void amp_boot_main(void)
{
    int ret = 0;
    unsigned char c = 0;
    unsigned int  i = 0;

#ifdef SUPPORT_SET_DRIVER_TRACE_FLAG
    aos_set_driver_trace_flag(0);
#endif

    amp_boot_init();

    if (amp_boot_cli_in() == false) {
#ifdef SUPPORT_SET_DRIVER_TRACE_FLAG
        aos_set_driver_trace_flag(1);
#endif
        amp_boot_deinit();
        return;
    }

    amp_boot_cli_menu();
#ifdef SUPPORT_SET_DRIVER_TRACE_FLAG
    aos_set_driver_trace_flag(1);
#endif
    amp_boot_deinit();
    return;
}
