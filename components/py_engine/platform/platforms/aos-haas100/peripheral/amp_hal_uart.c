/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "aos/kernel.h"
#include "amp_hal_uart.h"

int32_t amp_hal_uart_init(uart_dev_t *uart)
{
    return hal_uart_init(uart);
}

int32_t amp_hal_uart_send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    return hal_uart_send(uart, data, size, timeout);
}

int32_t amp_hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout)
{
    return hal_uart_recv(uart, data, expect_size, timeout);
}

int32_t amp_hal_uart_recv_II(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout)
{
    return hal_uart_recv_II(uart, data, expect_size, recv_size, timeout);
}

typedef struct {
    void (*callback)(int, void *, uint16_t, void *);
    void *userdata;
    uart_dev_t *uart;
    int task_running;
    int stop;
    aos_mutex_t lock;
    aos_sem_t sem;
    aos_task_t task;
} uart_recv_notify_t;

static uart_recv_notify_t *uart_recv_notifiers[8];

static void uart_recv_handler(void *args)
{
    uart_recv_notify_t *notify = (uart_recv_notify_t *)args;
    uart_dev_t *uart;
    char recv_buffer[256];
    int recv_size;
    int ret;

    if (!notify)
        return;

    uart = notify->uart;
    while (1) {
        aos_mutex_lock(&notify->lock, AOS_WAIT_FOREVER);
        if (notify->stop) {
            aos_mutex_unlock(&notify->lock);
            break;
        }
        aos_mutex_unlock(&notify->lock);

        ret = hal_uart_recv_II(uart, recv_buffer, sizeof(recv_buffer),
            &recv_size, 100);
        if (ret || recv_size <= 0)
            continue;

        notify->callback(uart->port, recv_buffer, recv_size, notify->userdata);
    }

    aos_sem_signal(&notify->sem);
}

extern int32_t hal_uart_receive_register(int port,  void(*cb)(int, void *), void *args);
static int uart_port_registered[7];
int32_t amp_hal_uart_callback(uart_dev_t *uart, void (*cb)(int, void *, uint16_t, void *), void *args)
{
#if 0
    if (uart_port_registered[uart->port])
        return 0;
    hal_uart_receive_register(uart, cb, args);
    uart_port_registered[uart->port] = 1;
#else
    uart_recv_notify_t *notify = uart_recv_notifiers[uart->port];

    if (!notify) {
        notify = aos_malloc(sizeof(uart_recv_notify_t));
        if (!notify)
            return -1;
        memset(notify, 0, sizeof(uart_recv_notify_t));
        aos_mutex_new(&notify->lock);
        aos_sem_new(&notify->sem, 0);
        uart_recv_notifiers[uart->port] = notify;
    }

    notify->callback = cb;
    notify->userdata = args;
    notify->uart = uart;

    if (!notify->task_running) {
        if (aos_task_new_ext(&notify->task, "amp_uart_recv",
            uart_recv_handler, notify, 2048, 32))
            return -1;
        notify->task_running = 1;
    }
#endif
    return 0;
}

int32_t amp_hal_uart_finalize(uart_dev_t *uart)
{
    uart_recv_notify_t *notify = uart_recv_notifiers[uart->port];
    if (notify) {
        if (notify->task_running) {
            aos_mutex_lock(&notify->lock, AOS_WAIT_FOREVER);
            notify->stop = 1;
            aos_mutex_unlock(&notify->lock);
            aos_sem_wait(&notify->sem, 3000);
        }
        aos_mutex_free(&notify->lock);
        aos_sem_free(&notify->sem);
        aos_free(notify);
        uart_recv_notifiers[uart->port] = NULL;
    }
    return hal_uart_finalize(uart);
}
