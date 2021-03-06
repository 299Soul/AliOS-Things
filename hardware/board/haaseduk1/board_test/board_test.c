/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include "aos/cli.h"
#include "aos/hal/flash.h"
#include "do.h"
#include "led.h"
#include "key.h"
#include "hal_gpio.h"
#include "hal_iomux_haas1000.h"
#include "netmgr.h"
#include <uservice/uservice.h>
#include <uservice/eventid.h>
#include <netmgr_wifi.h>

#define UNKNOWN_RESULT 99
#define USE_QUICK_WIFI_TEST

typedef enum {
    TEST_NULL,
    TEST_LED,
    TEST_KEY,
    TEST_UART_TTL,
    TEST_SPI,
    TEST_I2C,
    TEST_PWM,
    TEST_ADC,
    TEST_SD,
    TEST_WIFI_CONN,
    TEST_BT,
    TEST_WATCHDOG,
    TEST_NOISE,
    TEST_USB,
    TEST_AUDIO,
    TEST_EXTRAIO,
    TEST_GADC2,
    TEST_OLED,
    // i2c devices
    TEST_HUMI,
    TEST_GYRO,
    TEST_MAG,
    TEST_BARO,
    TEST_ALS,
    TEST_MAX,
};

typedef struct {
    char *name;
    int result;
} test_record_t;

extern void led_test(void);
extern int ble_test(void);
extern int key_test(void);
extern int pwm_test();
extern int adc_test(uint32_t port);
extern int sdcard_test(void);
extern void watchdog_test();
extern void led_blink(void);
extern void oled_blink(void);
extern int audio_test(int item);
extern int usb_test(void);
extern int sensors_test(int index);
extern void edu_oled_show(char *buf, int len);
extern void audio_test_stop(int item);

aos_task_t g_extraio_test_task;
extern aos_task_t g_led_test_task;

test_record_t test_rec_group[TEST_MAX] = {
    [TEST_LED] = {
        .name            = "LED",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_KEY] = {
        .name            = "KEY",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_UART_TTL] = {
        .name            = "UART",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_SPI] = {
        .name            = "SPI",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_I2C] = {
        .name            = "I2C",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_PWM] = {
        .name            = "PWM",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_ADC] = {
        .name            = "ADC",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_SD] = {
        .name            = "SD",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_WIFI_CONN] = {
        .name            = "WIFI CONNECT",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_BT] = {
        .name            = "BT",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_WATCHDOG] = {
        .name            = "WATCHDOG",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_NOISE] = {
        .name            = "NOISE",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_USB] = {
        .name            = "USB",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_AUDIO] = {
        .name            = "AUDIO",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_EXTRAIO] = {
        .name            = "EXTRAIO",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_GADC2] = {
        .name            = "GADC2",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_OLED] = {
        .name            = "OLED",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_HUMI] = {
        .name            = "HUMITURE",
        .result          = UNKNOWN_RESULT,
    },
    [TEST_GYRO] = {
        .name            = "GYRO",
        .result          = UNKNOWN_RESULT
    },
    [TEST_MAG] = {
        .name            = "MAG",
        .result          = UNKNOWN_RESULT
    },
    [TEST_BARO] = {
        .name            = "BARO",
        .result          = UNKNOWN_RESULT
    },
    [TEST_ALS] = {
        .name            = "ALS",
        .result          = UNKNOWN_RESULT
        },
    // extraboard_test end
};

static int _ip_got_finished = 0;
uint32_t led_test_flag = 0;
uint32_t oled_test_flag = 0;
uint32_t extraio_test_flag = 0;
uint32_t audio_test_flag = 0;
uint32_t auto_test_flag = 0;
uint32_t extra_test_enable = 1;
uint32_t keytest_successed = 0;

int show_last_autotest_results()
{
    int len = 4;
    char buff[5] = {0};
    char buf[2] = {0};
    char buff_lan[21] = {0};
    char buff_rel[21] = "PASS !";
    int j = 0;
    int k = 0;

    OLED_Clear();
    if (0 == aos_kv_get(test_rec_group[1].name, buff, &len)) {
        printf("\r\n\r\n");
        printf("**************************************************************\r\n");
        printf("**************************AUTO TEST RESULT********************\r\n");
        printf("--------------------------------------------------------------\r\n");
        printf("Test Items                  Result \r\n");
        printf("--------------------------------------------------------------\r\n");
        for (int i = 1; i < TEST_MAX; i++) {
            int ret = 0;
            memset(buff, 0, sizeof(buff));
            ret = aos_kv_get(test_rec_group[i].name, buff, &len);
            if (0 == strncmp(buff, "WAIT", 4)) {
                memcpy(buff, "PASS", 4);
            }
            if (ret == 0) {
                printf("%12s                 %s\n", test_rec_group[i].name, buff);
                // memcpy(buf,test_rec_group[i].name, 2);
                if (0 == strncmp(buff, "PASS", 4)) {
                    buff_lan[j] = 'Y';
                } else {
                    buff_lan[j] = 'N';
                    memcpy(buff_rel, "FAIL !", 7);
                }
                j++;
                k++;
                if (k % 10 == 0) {
                    buff_lan[j++] = ' ';
                }
            }
            aos_kv_del(test_rec_group[i].name);
        }
        OLED_Show_String(1, 4, "Auto Test Result:", 12, 1);
        OLED_Show_String(20, 16, buff_rel, 12, 1);
        OLED_Show_String(2, 28, "1234567890 123456789", 12, 1);
        OLED_Show_String(2, 40, "|||||||||| |||||||||", 12, 1);
        OLED_Show_String(2, 52, buff_lan, 12, 1);
        OLED_Refresh_GRAM();
        printf("**************************************************************\r\n");
        printf("**************************************************************\r\n");
        printf("**************************************************************\r\n");
        return 0;
    }
    return 1;
}

static void wifi_event_cb(uint32_t event_id, const void *param, void *context)
{
    aos_task_t task;
    aos_status_t ret;
    if (event_id != EVENT_NETMGR_DHCP_SUCCESS)
        return;

    if (_ip_got_finished != 0)
        return;

    _ip_got_finished = 1;
}

#ifndef USE_QUICK_WIFI_TEST
static int mfg_wifi_connect(const char *ssid, const char *passwd)
{
    netmgr_hdl_t hdl;
    netmgr_connect_params_t params;

    hdl = netmgr_get_dev("/dev/wifi0");

    if (hdl == -1) {
        printf("netmgr_get_dev failed! \r\n");
        return -1;
    }

    memset(&params, 0, sizeof(netmgr_connect_params_t));

    strncpy(params.params.wifi_params.ssid, ssid, sizeof(params.params.wifi_params.ssid) - 1);
    strncpy(params.params.wifi_params.pwd, passwd, sizeof(params.params.wifi_params.pwd) - 1);
    params.params.wifi_params.timeout = 15000;
    netmgr_connect(hdl, &params);

    return 0;
}
#endif

void extraio_test(void)
{
    unsigned int oled_id = 0;

    while (extraio_test_flag) {
        if ((oled_id % 8) == 0) {
            aos_msleep(500);
            for (int i = 0; i < 8; i++) {
                expansion_board_do_low(i);
            }
        }
        aos_msleep(200);
        expansion_board_do_high(oled_id % 8);
        oled_id++;
    }
}

static void beep()
{
    gpio_dev_t beeper;

    /* set as output mode */
    beeper.port = HAL_IOMUX_PIN_P2_6;
    beeper.config = OUTPUT_PUSH_PULL;

    hal_gpio_init(&beeper);
    hal_gpio_output_high(&beeper);
    aos_msleep(100);
    hal_gpio_output_low(&beeper);
}

void test_process_entry()
{
    uint8_t value = 0;
    int len = 1;
    int ret = 0;
    printf("Haas edu test entry here! version build time %s:%s \r\n", __DATE__, __TIME__);
    sh1106_init();

    ret = show_last_autotest_results();
    if (ret == 0) {
        return;
    }
    if (0 == aos_kv_get("factory_test_extra", &value, &len)) {
        extra_test_enable = 1;
    }
    key_test();
    led_test_flag = 1;
    oled_test_flag = 1;
    extraio_test_flag = 1;
    audio_test_flag = 1;
    OLED_Clear();
    /*first, init the value*/
    {
        test_rec_group[TEST_LED].result = -1;
        test_rec_group[TEST_OLED].result = -1;
        test_rec_group[TEST_EXTRAIO].result = -1;
        test_rec_group[TEST_AUDIO].result = -1;
    }
#if 1
    OLED_Show_String(4, 4,  "Led   pass -- prs k1.", 12, 1);
    OLED_Show_String(4, 16, "Oled  pass -- prs k2.", 12, 1);
    OLED_Show_String(4, 28, "Extra pass -- prs k3.", 12, 1);
    OLED_Show_String(4, 40, "Voice pass -- prs k4.", 12, 1);
    OLED_Show_String(2, 52, "Pls K1+K2 to autotest", 12, 1);
#endif
    OLED_Refresh_GRAM();
#if 1
    led_test();
    aos_task_create(&g_extraio_test_task, "extraio_test", extraio_test, NULL, NULL, 4096, 30, 0x01u);
    audio_test(0);
    while (auto_test_flag != 1 || keytest_successed != 0xF) {
        // printf("keytest_successed %x\n",keytest_successed);
        if (led_test_flag == 0) {
            printf("\r\n\r\n******************************************\r\n\r\n");
            printf("\r\n\r\n====Result: LED test PASS !!!=== \r\n\r\n");
            printf("\r\n\r\n******************************************\r\n\r\n");
            led_test_flag = 1;
            aos_task_delete(&g_led_test_task);
            OLED_Clear();
            OLED_Show_String(8, 8, "Key 1 pressed!", 12, 1);
            OLED_Show_String(6, 30, "Led-test passed!", 12, 1);
            OLED_Refresh_GRAM();
            audio_test_stop(0);
            beep();
            test_rec_group[TEST_LED].result = 0;
        }
        if (oled_test_flag == 0) {
            printf("\r\n\r\n******************************************\r\n\r\n");
            printf("\r\n\r\n====Result: OELD test PASS !!!=== \r\n\r\n");
            printf("\r\n\r\n******************************************\r\n\r\n");
            oled_test_flag = 1;
            beep();
            OLED_Clear();
            OLED_Show_String(8, 8, "Key 2 pressed!", 12, 1);
            OLED_Show_String(6, 30, "Oled-test passed!", 12, 1);
            OLED_Refresh_GRAM();
            audio_test_stop(0);
            test_rec_group[TEST_OLED].result = 0;
        }
        if (extraio_test_flag == 0) {
            printf("\r\n\r\n******************************************\r\n\r\n");
            printf("\r\n\r\n====Result: EXTRAIO test PASS !!!=== \r\n\r\n");
            printf("\r\n\r\n******************************************\r\n\r\n");
            extraio_test_flag = 1;
            beep();
            OLED_Clear();
            OLED_Show_String(8, 8,  "Key 3 pressed!", 12, 1);
            OLED_Show_String(6, 30, "Extraio-test passed!", 12, 1);
            OLED_Refresh_GRAM();
            aos_task_delete(&g_extraio_test_task);
            test_rec_group[TEST_EXTRAIO].result = 0;
            audio_test(1);
        }
        if (audio_test_flag == 0) {
            printf("\r\n\r\n******************************************\r\n\r\n");
            printf("\r\n\r\n====Result: AUDIO test PASS !!!=== \r\n\r\n");
            printf("\r\n\r\n******************************************\r\n\r\n");
            audio_test_flag = 1;
            OLED_Clear();
            OLED_Show_String(8, 8,  "Key 4 pressed!", 12, 1);
            OLED_Show_String(6, 30, "Audio-test passed", 12, 1);
            OLED_Show_String(2, 52, "Pls K1+K2 to autotest", 12, 1);
            OLED_Refresh_GRAM();
            audio_test_stop(1);
            beep();
            test_rec_group[TEST_AUDIO].result = 0;
        }
        aos_msleep(100);
    }
    printf("\r\n\r\n");
    printf("***************************************************************\r\n");
    printf("************************* KEY Test ****************************\r\n");
    printf("***************************************************************\r\n");
    printf("***************************************************************\r\n");
    printf("===== KEY test : Start=====\r\n");

    printf("\r\n\r\n******************************************\r\n\r\n");
    printf("\r\n\r\n====Result: KEY test PASS !!!=== \r\n\r\n");
    printf("\r\n\r\n******************************************\r\n\r\n");
    test_rec_group[TEST_KEY].result = 0;
#endif
    printf("\r\n\r\n\r\n\r\n");
    printf("!!!!!!!!!!!!!!!!!!!!AUTO TEST BEGIN!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
    edu_oled_show("  Auto Testing!", 10);
    test_rec_group[TEST_ADC].result = adc_test(0);
    test_rec_group[TEST_NOISE].result = adc_test(1);
    test_rec_group[TEST_GADC2].result = adc_test(2);
    // test_rec_group[TEST_UART_TTL].result = uart_ttl_test();
    // test_rec_group[TEST_SPI].result = spi_test();
    test_rec_group[TEST_SD].result = sdcard_test();
    test_rec_group[TEST_PWM].result = pwm_test();
    if (extra_test_enable)
        test_rec_group[TEST_USB].result = usb_test();
    test_rec_group[TEST_HUMI].result = sensors_test(0);
    test_rec_group[TEST_GYRO].result = sensors_test(1);
    test_rec_group[TEST_MAG].result = sensors_test(2);
    test_rec_group[TEST_BARO].result = sensors_test(3);
    test_rec_group[TEST_ALS].result = sensors_test(4);
    test_rec_group[TEST_BT].result = ble_test();
    printf("\r\n\r\n");
#ifdef USE_QUICK_WIFI_TEST
    printf("\r\n\r\n");
    printf("***************************************************************\r\n");
    printf("************************* WIFI Test *******************\r\n");
    printf("*How to test: find any Wi-Fi devices as successed. *****\r\n");
    printf("***************************************************************\r\n");
    printf("=====WIFI Connect test : Start=====\r\n");
    netmgr_wifi_ap_list_t ap_info[32];
    int ap_num = 0;

    memset(ap_info, 0, sizeof(ap_info));
    ap_num = netmgr_wifi_scan_result(ap_info, 32, NETMGR_WIFI_SCAN_TYPE_FULL);

    if (ap_num != 0)
#else
    printf("\r\n\r\n");
    printf("***************************************************************\r\n");
    printf("************************* WIFI Connect Test *******************\r\n");
    printf("*How to test: connect wifi ssid:haas200 paasword:12345678 *****\r\n");
    printf("***************************************************************\r\n");
    printf("=====WIFI Connect test : Start=====\r\n");
    mfg_wifi_connect("haas200", "12345678");
 if (_ip_got_finished == 1)
#endif
    {
        test_rec_group[TEST_WIFI_CONN].result = 0;
        printf("\r\n\r\n====Result: wifi connect test PASS !!!=== \r\n\r\n");
    } else {
        test_rec_group[TEST_WIFI_CONN].result = -1;
        printf("\r\n\r\n====Result: wifi connect test Failed !!!=== \r\n\r\n");
    }
    printf("\r\n\r\n******************************************\r\n\r\n");
    printf("--------------------------------------------------------------\r\n");

    for (int i = 1; i < TEST_MAX; i++) {
        if (test_rec_group[i].result != UNKNOWN_RESULT) {
            aos_kv_set(test_rec_group[i].name, (test_rec_group[i].result == 0) ? "PASS" : "FAIL", 4, 1);
        }
    }

    aos_kv_set(test_rec_group[TEST_WATCHDOG].name, "WAIT", 4, 1);
    watchdog_test();
    aos_reboot();
}

static void handle_haas_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    if ((0 == strcmp(argv[1], "key")) && (0 == strcmp(argv[2], "short"))) {
        printf("key show pressed\n");
        led_test_flag = 0;
    } else if ((0 == strcmp(argv[1], "key")) && (0 == strcmp(argv[2], "long"))) {
        printf("key long pressed\n");
        auto_test_flag = 1;
    } else if ((0 == strcmp(argv[1], "kv")) && (0 == strcmp(argv[2], "erase"))) {
        hal_flash_erase(HAL_PARTITION_PARAMETER_2, 0, 0xd000);
    }
}

static struct cli_command haas_cmd = {
    .name = "haas",
    .help = "haas [set]",
    .function = handle_haas_cmd
};

extern uint32_t g_wifi_factory_test;
int board_test()
{
    int count = 0;
    int ret = 0;

    expansion_board_do_init();

    // Wi-Fi initialization
    ret = event_service_init(NULL);
    if (ret != 0) {
        printf("event_service_init failed! %d\n", ret);
    }
    ret = netmgr_service_init(NULL);
    if (ret != 0) {
        printf("netmgr_service_init failed! %d\n", ret);
    }
    event_subscribe(EVENT_NETMGR_DHCP_SUCCESS, wifi_event_cb, NULL);
    // BLE initialization

    aos_task_new("test_process", test_process_entry, NULL, 8192);

    while (1) {
        aos_msleep(1000);
    }
}

