#include "homepage.h"
#include "../menu.h"
#include "aos/kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include "posix/timer.h"
#include "../../build_version.h"
#include "aos/hal/adc.h"
// #include "netmgr.h"

static int battery_level = 0;
extern int wifi_connected;
extern int bt_connected;
extern int get_ipaddr;

uint8_t ipv4_addr[4] = {192, 168, 0, 10};

MENU_COVER_TYP homepage_cover = {MENU_COVER_NONE};
MENU_TASK_TYP  homepage_tasks = {homepage_init, homepage_uninit};
MENU_TYP homepage = {"homepage", &homepage_cover, &homepage_tasks, NULL, NULL};

static aos_task_t homepage_task_handle;

int homepage_init(void)
{
    OLED_Clear();
    OLED_Refresh_GRAM();
    aos_task_new_ext(&homepage_task_handle, "homepage_task", homepage_task, NULL, 1024, AOS_DEFAULT_APP_PRI);
    printf("aos_task_new homepage_task\n");
    return 0;
}

static int get_battery(int *level, uint32_t *volage)
{
    int32_t  ret       = 0;
    uint32_t output    = 0;
    uint32_t test_sum  = 0;
    uint32_t test_avrg = 0;
    uint32_t test_min  = 3300;
    uint32_t test_max  = 0;

    adc_dev_t adc = {1, 1000, 0x12345678};

    ret = hal_adc_init(&adc);
    if (ret) {
        printf("\r\n=====adc test : adc init failed===\r\n");
        return -1;
    }

    for (int32_t i = 0; i < 10; i++) {
        hal_adc_value_get(&adc, &output, 200);
        test_sum += output;

        /* the min sampling voltage */
        if (test_min >= output) {
            test_min = output;
        }
        /* the max sampling voltage */
        if (test_max <= output) {
            test_max = output;
        }
        osDelay(1);
    }

    hal_adc_finalize(&adc);

    test_avrg = (test_sum - test_min - test_max) >> 3;
    // printf("\r\n=====adc test : the samping volage is:%dmv===\r\n",
    // test_avrg);
    test_avrg *= 3.208;
    *volage = test_avrg;

    if (test_avrg > 4100) {
        *level = 4;
    } else if ((test_avrg > 3980) && (test_avrg < 4100)) {
        *level = 3;
    } else if ((test_avrg > 3850) && (test_avrg < 3980)) {
        *level = 2;
    } else if ((test_avrg > 3700) && (test_avrg < 3850)) {
        *level = 1;
    } else if (test_avrg < 3700) {
        *level = 0;
    }
    return 0;
}

void homepage_task(void)
{
    unsigned char   c = 0;
    struct tm *info;
    struct timespec tv;
    uint8_t         image_version[22];
    uint8_t         tmp[22];
    // netmgr_stats_t stats;
    uint32_t volage;
    while (1) {
        OLED_Clear();
        /* 获取 GMT 时间 */
        clock_gettime(CLOCK_REALTIME, &tv);
        info = gmtime(&tv);

        snprintf(tmp, 21, "%2d:%02d", (info->tm_hour + 8) % 24, info->tm_min);
        OLED_Show_String(0, 12 * 0, tmp, 12, 1);
        // if (wifi_connected)
        // {
        //     OLED_Icon_Draw(86, 0, &icon_wifi_on_12_12, 0);
        // }

        // if (get_ipaddr)
        // {
        //     netmgr_stats(0, &stats);
        //     if (stats.ip_available)
        //     {
        //         snprintf(image_version, 20, "IP: %s", stats.ip);
        //         OLED_Show_String(20, (12 + 4) * 3, image_version, 12, 1);
        //     }
        // }else
        // {
        //     //snprintf(image_version, 20, "IP: ?.?.?.?", stats.ip);
        //     //OLED_Show_String(20, (12 + 4) * 3, image_version, 12, 1);
        // }

        // if (bt_connected)
        // {
        //     OLED_Icon_Draw(97, 0, &icon_bt_on_12_12, 0);
        // }

        OLED_Show_String(40, (12 + 4) * 1, "HaaS EDU", 12, 1);
        snprintf(image_version, 21, "VER: %s", BUILD_VERSION);
        OLED_Show_String(33, (12 + 4) * 2, image_version, 12, 1);

        OLED_Icon_Draw(2, 24, &icon_skip_left, 0);
        OLED_Icon_Draw(122, 24, &icon_skip_right, 0);

        if (0 == get_battery(&battery_level, &volage)) {
            // printf("get_battery success %d:%d\n", volage, battery_level);
            OLED_Icon_Draw(110, 0, &icon_battery_20_12[battery_level], 0);
        } else {
            printf("get_battery fail\n");
        }

        OLED_Refresh_GRAM();
        aos_msleep(1000);
    }
}

int homepage_uninit(void)
{
    aos_task_delete(&homepage_task_handle);
    printf("aos_task_delete homepage_task\n");
    return 0;
}
