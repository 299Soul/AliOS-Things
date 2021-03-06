/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifdef __ICCARM__
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "aos/kernel.h"

#include "sys/types.h"
#include "sys/time.h"
#include "aos/hal/uart.h"
#include "umm.h"

int errno;

extern long long  aos_now_ms(void);

__ATTRIBUTES void *malloc(unsigned int size)
{
    return umm_alloc(size);
}

__ATTRIBUTES void *realloc(void *old, unsigned int newlen)
{
    return umm_realloc(old, newlen);
}

__ATTRIBUTES void *calloc(size_t len, size_t elsize)
{
    void *mem;

    mem = umm_alloc(elsize * len);

    if (mem) {
        memset(mem, 0, elsize * len);
    }

    return mem;
}

__ATTRIBUTES void free(void *mem)
{
    umm_free(mem);
}

__ATTRIBUTES time_t time(time_t *tod)
{
    uint64_t t = aos_now_ms();
    return (time_t)(t / 1000);
}

int *__errno _PARAMS ((void))
{
    return 0;
}

void __assert_func(const char *a, int b, const char *c, const char *d)
{
    while (1);
}

/*TO DO*/
#pragma weak __write
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    uart_dev_t uart_stdio;
    int i;
    memset(&uart_stdio, 0, sizeof(uart_stdio));
    uart_stdio.port = HAL_UART_STDIO_PORT;

    if (buffer == 0) {
        /*
         * This means that we should flush internal buffers.  Since we don't we just return.
         * (Remember, "handle" == -1 means that all handles should be flushed.)
         */
        return 0;
    }

    /* This function only writes to "standard out" and "standard err" for all other file handles it returns failure. */
    if ((handle != 1) && (handle != 2)) {
        return ((size_t) - 1);
    }

    /* Send data. */
    for (i = 0; i < size; i++) {
        if (buffer[i] == '\n') {
            hal_uart_send(&uart_stdio, (void *)"\r", 1, AOS_WAIT_FOREVER);
        }

        hal_uart_send(&uart_stdio, &buffer[i], 1, AOS_WAIT_FOREVER);
    }

    return size;
}

void bzero()
{

}

void __lseek()
{

}

void __close()
{

}

int remove(char const *p)
{
    return 0;
}

int gettimeofday(struct timeval *tp, void *ignore)
{
    return 0;
}

void getopt()
{

}

void optarg()
{

}

#endif

