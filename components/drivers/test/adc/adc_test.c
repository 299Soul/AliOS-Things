/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>

#include "aos/cli.h"
#include "aos/vfs.h"

#include <drivers/u_ld.h>
#include <hal2vfs/io_adc.h>
#include <drivers/char/u_device.h>

void vfs_adc_handler (void *arg) {
    ddkc_warn("enter %s arg:%p, fd = %d\r\n", __func__, arg, *((int *)arg));

    return;
}

int vfs_adc_test(int index)
{
    int ret = 0;
    int fd = 0;
    int sampling_cycle = 100;
    char name[16] = {0};
    io_adc_arg_t adc_arg;

    snprintf(name, sizeof(name), "/dev/adc%d", index);
    ddkc_info("opening name:%s\r\n", name);
    fd = open(name, 0);
    ddkc_warn("open %s %s, fd:%d\r\n", name, fd >= 0 ? "success" : "fail", fd);

    if (fd >= 0) {
        // correct parameters
        ret = ioctl(fd, IOC_ADC_START, sampling_cycle);
        ddkc_warn("start adc %s, ret:%d\r\n", ret ? "fail" : "succeed", ret);

        sleep(1);
        adc_arg.value = 0;
        adc_arg.timeout = 500000; // in unit of us

        ret = ioctl(fd, IOC_ADC_GET_VALUE, (unsigned long)&adc_arg);
        ddkc_warn("get value result:%d, value:0x%x\r\n", ret, adc_arg.value);
        sleep(1);

        ret = ioctl(fd, IOC_ADC_STOP, 0);

        close(fd);
    }
    return 0;
}

static void vfs_adc_cli_cmd(char *buf, int len, int argc, char **argv)
{
    int ret = 0;
    int index = argc > 1 ? atoi(argv[1]) : 1;

    ddkc_info("argc:%d, index:%d\r\n", argc, index);
    ret = vfs_adc_test(index);

    ddkc_info("vfs adc test %s, ret:%d\r\n", ret ? "failed" : "success", ret);

    return;
}

struct cli_command vfs_adc_cli_cmds[] = {
    {"adct",         "adc test", vfs_adc_cli_cmd},
};

int vfs_adc_test_cmd_init(void) {
    return aos_cli_register_commands(&vfs_adc_cli_cmds[0],
                                     sizeof(vfs_adc_cli_cmds)/sizeof(vfs_adc_cli_cmds[0]));
}

POST_DRIVER_ENTRY(vfs_adc_test_cmd_init)
