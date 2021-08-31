/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#ifndef AMP_BOOT_H
#define AMP_BOOT_H

#include "stdint.h"
#include "stdbool.h"
#include "amp_platform.h"
#include "aos_system.h"
#include "aos_network.h"
#include "aos_fs.h"
#include "amp_boot_recovery.h"
#include "aos_hal_uart.h"

#include "amp_boot_uart.h"
#include "amp_boot_cmd.h"

#define osWaitForever 0xFFFFFFFF

void pyamp_boot_main();

#endif