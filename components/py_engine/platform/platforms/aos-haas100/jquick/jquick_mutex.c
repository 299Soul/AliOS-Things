//
// jquick_mutex.c
//
// Created by liujuncheng@alibaba-inc.com on 2020/11/11.
//
// Copyright (c) 2020 Alibaba All rights reserved.
//

#include "jquick_mutex.h"

#include <pthread.h>
#include <stdlib.h>
//#include <utils/log.h>
#include "HaasLog.h"

JQuick_Mutex jquick_mutex_create()
{
    JQuick_Mutex mutex = (JQuick_Mutex)malloc(sizeof(pthread_mutex_t));
    if (!mutex) {
        LOG_E("JQuick_Mutex: out of memory\n");

        return NULL;
    }

    int v = pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
    if (0 == v) {
        return mutex;
    }

    LOG_E("JQuick_Mutex: Init JQuick_Mutex failed\n");

    free(mutex);

    return NULL;
}

int jquick_mutex_lock(JQuick_Mutex mutex)
{
    if (!mutex) {
        LOG_E("JQuick_Mutex: Mutex is NULL\n");
        return -1;
    }

    return pthread_mutex_lock((pthread_mutex_t*)mutex);
}

int jquick_mutex_unlock(JQuick_Mutex mutex)
{
    if (!mutex) {
        LOG_E("JQuick_Mutex: Mutex is NULL\n");
        return -1;
    }

    return pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

int jquick_mutex_destroy(JQuick_Mutex mutex)
{
    if (!mutex) {
        LOG_E("JQuick_Mutex: Mutex is NULL\n");
        return -1;
    }

    int v = pthread_mutex_destroy((pthread_mutex_t*)mutex);
    if (v == 0) {
        free(mutex);
    }

    return v;
}
