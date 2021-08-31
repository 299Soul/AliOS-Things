/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <reent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <stdarg.h>

#include <k_api.h>
#include "aos/kernel.h"
#include "aos/vfs.h"

#include "aos/hal/uart.h"

#include "vfs_conf.h"

#include "sys/socket.h"
#ifdef CONFIG_AOS_LWIP
#ifdef TELNETD_ENABLED
#include "lwip/apps/telnetserver.h"
#endif
#endif

#define FD_VFS_START VFS_FD_OFFSET
#define FD_VFS_END   (FD_VFS_START + VFS_MAX_FILE_NUM - 1)

#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
#define FD_SOCKET_START FD_AOS_SOCKET_OFFSET
#define FD_SOCKET_END   (FD_AOS_SOCKET_OFFSET + FD_AOS_NUM_SOCKETS - 1)
#define FD_EVENT_START  FD_AOS_EVENT_OFFSET
#define FD_EVENT_END    (FD_AOS_EVENT_OFFSET + FD_AOS_NUM_EVENTS - 1)
#endif
#endif

int _execve_r(struct _reent *ptr, const char *name, char *const *argv,
              char *const *env)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _fcntl_r(struct _reent *ptr, int fd, int cmd, int arg)
{
    if ((fd >= FD_VFS_START) && (fd <= FD_VFS_END)) {
        return aos_fcntl(fd, cmd, arg);
#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
    } else if ((fd >= FD_SOCKET_START) && (fd <= FD_EVENT_END)) {
        return lwip_fcntl(fd, cmd, arg);
#endif
#endif
    } else {
        return -1;
    }
}

int _fork_r(struct _reent *ptr)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _getpid_r(struct _reent *ptr)
{
    ptr->_errno = ENOTSUP;
    return 0;
}

int _isatty_r(struct _reent *ptr, int fd)
{
    if (fd >= 0 && fd < 3) {
        return 1;
    }

    ptr->_errno = ENOTSUP;
    return -1;
}

int _kill_r(struct _reent *ptr, int pid, int sig)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _link_r(struct _reent *ptr, const char *old, const char *new)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence)
{
    int ret = aos_lseek(fd, pos, whence);
    if (ret < 0) {
        ptr->_errno = -ret;
        ret = -1;
    }
    return ret;
}

int _mkdir_r(struct _reent *ptr, const char *name, int mode)
{
    int ret = aos_mkdir(name);
    if (ret < 0) {
        ptr->_errno = -ret;
        ret = -1;
    }
    return ret;
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode)
{
    int ret = aos_open(file, flags);
    if (ret < 0) {
        ptr->_errno = -ret;
        ret = -1;
    }
    return ret;
}

int _close_r(struct _reent *ptr, int fd)
{
    if ((fd >= FD_VFS_START) && (fd <= FD_VFS_END)) {
        return aos_close(fd);
#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
    } else if ((fd >= FD_SOCKET_START) && (fd <= FD_EVENT_END)) {
        return lwip_close(fd);
#endif
#endif
    } else {
        return -1;
    }
}

_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes)
{
    if ((fd >= FD_VFS_START) && (fd <= FD_VFS_END)) {
        return aos_read(fd, buf, nbytes);
#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
    } else if ((fd >= FD_SOCKET_START) && (fd <= FD_EVENT_END)) {
        return lwip_read(fd, buf, nbytes);
#endif
#endif
    } else {
        return -1;
    }
}

/*
 * implement _write_r here
 */
_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
    const char *tmp = buf;
    int         i   = 0;
    uart_dev_t  uart_stdio;

    if (buf == NULL) {
        return 0;
    }

    memset(&uart_stdio, 0, sizeof(uart_stdio));
    uart_stdio.port = HAL_UART_STDIO_PORT;

    if ((fd >= FD_VFS_START) && (fd <= FD_VFS_END)) {
        return aos_write(fd, buf, nbytes);
#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
    } else if ((fd >= FD_SOCKET_START) && (fd <= FD_EVENT_END)) {
        return lwip_write(fd, buf, nbytes);
#endif
#endif
    } else if ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO)) {
        for (i = 0; i < nbytes; i++) {
            if (*tmp == '\n') {
#ifdef TELNETD_ENABLED
                TelnetWrite('\r');
#endif
                hal_uart_send(&uart_stdio, (void *)"\r", 1, AOS_WAIT_FOREVER);
            }

#ifdef TELNETD_ENABLED
            TelnetWrite(*tmp);
#endif
            hal_uart_send(&uart_stdio, (void *)tmp, 1, AOS_WAIT_FOREVER);
            tmp++;
        }

        return nbytes;
    } else {
        return -1;
    }
}

int ioctl(int fildes, int request, ... /* arg */)
{
    long    arg  = 0;
    void   *argp = NULL;
    va_list args;
    int ret;

    va_start(args, request);

    if ((fildes >= VFS_FD_OFFSET) &&
        (fildes <= (VFS_FD_OFFSET + VFS_MAX_FILE_NUM - 1))) {
        arg = va_arg(args, int);
        ret = aos_ioctl(fildes, request, arg);
        va_end(args);
        return ret;
#ifdef POSIX_DEVICE_IO_NEED
#ifdef CONFIG_AOS_LWIP
    } else if ((fildes >= FD_AOS_SOCKET_OFFSET) &&
               (fildes <= (FD_AOS_EVENT_OFFSET + FD_AOS_NUM_EVENTS - 1))) {
        argp = va_arg(args, void *);
        ret = lwip_ioctl(fildes, request, argp);
        va_end(args);
        return ret;
#endif
#endif
    } else {
        va_end(args);
        return -1;
    }
}

int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _rename_r(struct _reent *ptr, const char *old, const char *new)
{
    ptr->_errno = ENOTSUP;
    return 0;
}

void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    ptr->_errno = ENOTSUP;
    return NULL;
}

int _stat_r(struct _reent *ptr, const char *file, struct stat *pstat)
{
    struct aos_stat stat;
    int    ret;

    ret = aos_stat(file, &stat);

    pstat->st_mode  = stat.st_mode;
    pstat->st_size  = stat.st_size;
    pstat->st_atime = stat.st_actime;
    pstat->st_mtime = stat.st_modtime;

    return ret;
}

_CLOCK_T_ _times_r(struct _reent *ptr, struct tms *ptms)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _unlink_r(struct _reent *ptr, const char *file)
{
    int ret = aos_unlink(file);
    if (ret < 0) {
        ptr->_errno = -ret;
        ret = -1;
    }
    return ret;
}

int _wait_r(struct _reent *ptr, int *status)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

int _gettimeofday_r(struct _reent *ptr, struct timeval *tv, void *__tzp)
{
    uint64_t t;
    struct timezone *tz = __tzp;

    if (tv) {
        t = aos_calendar_time_get();
        tv->tv_sec  = t / 1000;
        tv->tv_usec = (t % 1000) * 1000;
    }

    if (tz) {
        /* Not supported. */
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }

    return 0;
}

long timezone = 0; /* global variable */

struct tm* localtime_r(const time_t* t, struct tm* r)
{
    time_t time_tmp;
    time_tmp = *t + timezone;
    return gmtime_r(&time_tmp, r);
}

struct tm* localtime(const time_t* t)
{
    struct tm* timeinfo;
    static struct tm tm_tmp;

    timeinfo = localtime_r(t, &tm_tmp);

    return timeinfo;
}

void *_malloc_r(struct _reent *ptr, size_t size)
{
    void *mem;

#if (RHINO_CONFIG_MM_DEBUG > 0u)
    mem = aos_malloc(size | AOS_UNSIGNED_INT_MSB);
    aos_alloc_trace(mem, (size_t)__builtin_return_address(0));
#else
    mem = aos_malloc(size);
#endif

    return mem;
}

void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    void *mem;

#if (RHINO_CONFIG_MM_DEBUG > 0u)
    mem = aos_realloc(old, newlen | AOS_UNSIGNED_INT_MSB);
    aos_alloc_trace(mem, (size_t)__builtin_return_address(0));
#else
    mem = aos_realloc(old, newlen);
#endif

    return mem;
}

void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
    void *mem;

#if (RHINO_CONFIG_MM_DEBUG > 0u)
    mem = aos_malloc((size * len) | AOS_UNSIGNED_INT_MSB);
    aos_alloc_trace(mem, (size_t)__builtin_return_address(0));
#else
    mem = aos_malloc(size * len);
#endif

    if (mem) {
        bzero(mem, size * len);
    }

    return mem;
}

void _free_r(struct _reent *ptr, void *addr)
{
    aos_free(addr);
}

void _exit(int status)
{
    while (1)
        ;
}

void exit(int status)
{
    aos_task_exit(status);
    /* This function declares the noreturn attribute, and execution should not return */
    while (1)
        ;
}

__attribute__((weak)) void _fini()
{
    return;
}

void _system(const char *s)
{
    return;
}

void abort(void)
{
    k_err_proc(RHINO_SYS_FATAL_ERR);
    /* This function declares the noreturn attribute, and execution should not return */
    while (1)
        ;
}

