/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "esp_system.h"
#include "k_api.h"
#include "upgrade.h"
#include "lwip/ip4_addr.h"
#include "esp_wifi.h"

extern void vPortETSIntrLock(void);
extern void aos_msleep(int ms);
extern kstat_t krhino_sched_disable(void);

unsigned int delay_i = 0;
static void delay()
{
    for(delay_i = 0; delay_i < 100000; delay_i++) {

    }
}

void hal_reboot(void)
{
    printf("reboot!\n");
    vPortETSIntrLock();
    krhino_sched_disable();
    delay();
    delay();
    rom_i2c_writeReg(0x67, 4, 1, 8);
    rom_i2c_writeReg(0x67, 4, 2, 0x81);
    while(1) {
        system_restart();
        aos_msleep(100);
    }
}


int hal_reboot_bank(void)
{
    printf("reboot to banker\n");
    wifi_set_sleep_type(NONE_SLEEP_T);
    ota_msleep(300);
    system_upgrade_init();
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    while(1) {
        system_upgrade_process();
        system_restart_in_nmi();
        aos_msleep(1000);
    }
    system_upgrade_reboot();
    return 0;
}

int ota_hal_reboot_bank(void)
{
    return hal_reboot_bank();
}