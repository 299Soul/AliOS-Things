/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include <termio.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "aos_hal_gpio.h"

#ifdef CONFIG_GPIO_NUM
#define PLATFORM_GPIO_NUM CONFIG_GPIO_NUM
#else
#define PLATFORM_GPIO_NUM 40
#endif

static uint8_t gpio_value[PLATFORM_GPIO_NUM * 2] = { 0 };

int32_t aos_hal_gpio_init(gpio_dev_t *gpio)
{
    memset(gpio_value, 0, PLATFORM_GPIO_NUM * 2);
    return 0;
}

int32_t aos_hal_gpio_output_high(gpio_dev_t *gpio)
{
    gpio_value[gpio->port] = 1;
    return 0;
}

int32_t aos_hal_gpio_output_low(gpio_dev_t *gpio)
{
    gpio_value[gpio->port] = 0;
    return 0;
}

int32_t aos_hal_gpio_output_toggle(gpio_dev_t *gpio)
{
    return 0;
}

int32_t aos_hal_gpio_input_get(gpio_dev_t *gpio, uint32_t *value)
{
    *value = gpio_value[gpio->port];
    return 0;
}

static gpio_irq_handler_t irq_handler = NULL;
static void *irq_arg = NULL;

static void linux_sig_handler(void *arg)
{
    printf("recv signal SIGUSR1\n");
    irq_handler(irq_arg);
}

int32_t aos_hal_gpio_enable_irq(gpio_dev_t *gpio, gpio_irq_trigger_t trigger, gpio_irq_handler_t handler, void *arg)
{
    irq_handler = handler;
    irq_arg = arg;

    struct sigaction sa_usr;
    sa_usr.sa_flags = 0;
    sa_usr.sa_handler = linux_sig_handler;

    sigaction(SIGUSR1, &sa_usr, NULL);

    return 0;
}

int32_t aos_hal_gpio_disable_irq(gpio_dev_t *gpio)
{
    return 0;
}

int32_t aos_hal_gpio_clear_irq(gpio_dev_t *gpio)
{
    return aos_hal_gpio_disable_irq(gpio);
}

int32_t aos_hal_gpio_finalize(gpio_dev_t *gpio)
{
    return 0;
}
