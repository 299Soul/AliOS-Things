/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include <unistd.h>

#include <k_api.h>
#include <pthread.h>
#include <enviro.h>

extern const char *aos_version_get(void);

pthread_mutex_t g_enviro_lock = PTHREAD_MUTEX_INITIALIZER;
static _pthread_environ_t *g_penviron = NULL;

static _pthread_environ_t *env_new(const char *envname, const char *envval);
static void env_free(_pthread_environ_t *penv);

static _pthread_environ_t *env_new(const char *envname, const char *envval)
{
    _pthread_environ_t *penv = NULL;

    /* malloc _pthread_environ_t */
    penv = krhino_mm_alloc(sizeof(_pthread_environ_t));
    if (penv == NULL) {
        return NULL;
    }

    /* malloc envname and copy the envname */
    penv->envname = krhino_mm_alloc(strlen(envname) + 1);
    if (penv->envname == NULL) {
        return NULL;
    }

    strcpy(penv->envname, envname);

    /* malloc envval and copy the envval */
    penv->envval = krhino_mm_alloc(strlen(envval) + 1);
    if (penv->envval == NULL) {
        return NULL;
    }

    strcpy(penv->envval, envval);

    penv->next = NULL;

    return penv;
}

void env_free(_pthread_environ_t *penv)
{
    if (penv->envname == NULL) {
        krhino_mm_free(penv->envname);
    }

    if (penv->envval == NULL) {
        krhino_mm_free(penv->envval);
    }

    krhino_mm_free(penv);
}

int setenv(const char *envname, const char *envval, int overwrite)
{
    //_pthread_tcb_t     *ptcb     = NULL;
    _pthread_environ_t *penv     = NULL;
    _pthread_environ_t *penv_pre = NULL;

    int ret = -1;

    if ((envname == NULL) || (envval == NULL)) {
        return -1;
    }

    /* get current thread tcb */
    /*
    ptcb = _pthread_get_tcb(krhino_cur_task_get());
    if (ptcb == NULL) {
        return -1;
    }*/

    ret = pthread_mutex_lock(&g_enviro_lock);
    if (ret != 0) {
        return -1;
    }

    /* if no environ in tcb, create the first one */
    if (g_penviron == NULL) {
        penv = env_new(envname, envval);
        if (penv == NULL) {
            pthread_mutex_unlock(&g_enviro_lock);
            return -1;
        }

        g_penviron = penv;

        pthread_mutex_unlock(&g_enviro_lock);
        return 0;
    }

    /* search the environ list to find the match item */
    penv = g_penviron;
    while (penv != NULL) {
        if (strcmp(penv->envname, envname) == 0) {
            /* if the environment variable named by envname already exists and the value of overwrite is non-zero,
               the function shall return success and the environment shall be updated */
            if (overwrite != 0) {
                penv->envval = krhino_mm_realloc(penv->envval, strlen(envval) + 1);
                strcpy(penv->envval, envval);

                pthread_mutex_unlock(&g_enviro_lock);
                return 0;
            } else {
                /* If the environment variable named by envname already exists and the value of overwrite is zero, the
                   function shall return success and the environment shall remain unchanged */
                pthread_mutex_unlock(&g_enviro_lock);
                return 0;
            }
        }

        penv_pre = penv;
        penv = penv->next;
    }

    /* if no match item create one and add to the end of list */
    penv = env_new(envname, envval);
    if (penv == NULL) {
        pthread_mutex_unlock(&g_enviro_lock);
        return -1;
    }

    penv_pre->next = penv;

    pthread_mutex_unlock(&g_enviro_lock);

    return 0;
}

char *getenv(const char *name)
{
    //_pthread_tcb_t     *ptcb = NULL;
    _pthread_environ_t *penv = NULL;

    if (name == NULL) {
        return NULL;
    }

    /* get current thread tcb */
    /*
    ptcb = _pthread_get_tcb(krhino_cur_task_get());
    if (ptcb == NULL) {
        return NULL;
    }*/

    penv = g_penviron;
    if (penv == NULL) {
        return NULL;
    }

    /* search the environ list to find the match item */
    while (penv != NULL) {
        if (strcmp(penv->envname, name) == 0) {
            return penv->envval;
        }

        penv = penv->next;
    }

    return NULL;
}

int unsetenv(const char *name)
{
    //_pthread_tcb_t     *ptcb     = NULL;
    _pthread_environ_t *penv     = NULL;
    _pthread_environ_t *penv_pre = NULL;

    int ret = -1;

    if (name == NULL) {
        return -1;
    }

    /* get current thread tcb */
    /*
    ptcb = _pthread_get_tcb(krhino_cur_task_get());
    if (ptcb == NULL) {
        return -1;
    }*/

    penv = g_penviron;
    if (penv == NULL) {
        return -1;
    }

    ret = pthread_mutex_lock(&g_enviro_lock);
    if (ret != 0) {
        return -1;
    }

    /* search the environ list to find the match item and free it */
    while (penv != NULL) {
        if (strcmp(penv->envname, name) == 0) {
            if (penv_pre == NULL) {
                g_penviron = penv->next;
            } else {
                penv_pre->next = penv->next;
            }

            pthread_mutex_unlock(&g_enviro_lock);

            /* free the _pthread_environ_t data */
            env_free(penv);

            return 0;
        }

        penv_pre = penv;
        penv = penv->next;
    }

    pthread_mutex_unlock(&g_enviro_lock);

    return -1;
}

int putenv(char *string)
{
    int   pos     = 0;
    char *envname = NULL;
    char *envval  = NULL;
    int   ret     = -1;

    for (pos = 0; pos < strlen(string); pos++) {
        if (string[pos] == '=') {
            envval = &string[pos + 1];

            /* malloc a memory to save envname */
            envname = krhino_mm_alloc(pos + 1);
            if (envname == NULL) {
                return -1;
            }

            /* copy envname */
            strncpy(envname, string, pos);
            envname[pos] = '\0';

            ret = setenv(envname, envval, 1);

            /* free envname */
            krhino_mm_free(envname);

            return ret;
        }
    }

    return -1;
}

int uname(struct utsname *name)
{
    const char *version = NULL;
    const char *os      = "AliOS Things";

    if (name == NULL) {
        return -1;
    }

    memset(name, 0, sizeof(struct utsname));

    version = aos_version_get();

    if (strlen(version) <= _UTSNAME_VERSION_LENGTH) {
        strcpy(name->version, version);
    }

    strcpy(name->sysname, os);

    return 0;
}

long sysconf(int name)
{
    long val = 0;

    switch (name) {
        case _SC_JOB_CONTROL :
            val = _POSIX_JOB_CONTROL;
            break;
        case _SC_SAVED_IDS :
            val = _POSIX_SAVED_IDS;
            break;
        case _SC_VERSION :
            val = _POSIX_VERSION;
            break;
        case _SC_ASYNCHRONOUS_IO :
            val = _POSIX_ASYNCHRONOUS_IO;
            break;
        case _SC_FSYNC :
            val = _POSIX_FSYNC;
            break;
        case _SC_MAPPED_FILES :
            val = _POSIX_MAPPED_FILES;
            break;
        case _SC_MEMLOCK :
            val = _POSIX_MEMLOCK;
            break;
        case _SC_MEMLOCK_RANGE :
            val = _POSIX_MEMLOCK_RANGE;
            break;
        case _SC_MEMORY_PROTECTION :
            val = _POSIX_MEMORY_PROTECTION;
            break;
        case _SC_MESSAGE_PASSING :
            val = _POSIX_MESSAGE_PASSING;
            break;
        case _SC_PRIORITIZED_IO :
            val = _POSIX_PRIORITIZED_IO;
            break;
        case _SC_REALTIME_SIGNALS :
            val = _POSIX_REALTIME_SIGNALS;
            break;
        case _SC_SEMAPHORES :
            val = _POSIX_SEMAPHORES;
            break;
        case _SC_SYNCHRONIZED_IO :
            val = _POSIX_SYNCHRONIZED_IO;
            break;
        case _SC_TIMERS :
            val = _POSIX_TIMERS;
            break;
        case _SC_BARRIERS :
            val = _POSIX_BARRIERS;
            break;
        case _SC_READER_WRITER_LOCKS :
            val = _POSIX_READER_WRITER_LOCKS;
            break;
        case _SC_SPIN_LOCKS :
            val = _POSIX_SPIN_LOCKS;
            break;
        case _SC_THREADS :
            val = _POSIX_THREADS;
            break;
        case _SC_THREAD_ATTR_STACKADDR :
            val = _POSIX_THREAD_ATTR_STACKADDR;
            break;
        case _SC_THREAD_ATTR_STACKSIZE :
            val = _POSIX_THREAD_ATTR_STACKSIZE;
            break;
        case _SC_THREAD_PRIORITY_SCHEDULING :
            val = _POSIX_THREAD_PRIORITY_SCHEDULING;
            break;
        case _SC_THREAD_PRIO_INHERIT :
            val = _POSIX_THREAD_PRIO_INHERIT;
            break;
        case _SC_THREAD_PRIO_PROTECT :
            val = _POSIX_THREAD_PRIO_PROTECT;
            break;
        case _SC_THREAD_PROCESS_SHARED :
            val = _POSIX_THREAD_PROCESS_SHARED;
            break;
        case _SC_THREAD_SAFE_FUNCTIONS :
            val = _POSIX_THREAD_SAFE_FUNCTIONS;
            break;
        case _SC_SPAWN :
            val = _POSIX_SPAWN;
            break;
        case _SC_TIMEOUTS :
            val = _POSIX_TIMEOUTS;
            break;
        case _SC_CPUTIME :
            val = _POSIX_CPUTIME;
            break;
        case _SC_THREAD_CPUTIME :
            val = _POSIX_THREAD_CPUTIME;
            break;
        case _SC_ADVISORY_INFO :
            val = _POSIX_ADVISORY_INFO;
            break;
        default:
            val = -1;
            break;
    }

    return val;
}

size_t confstr(int name, char *buf, size_t len)
{
    int len_real = 0;

    if (name == _CS_GNU_LIBC_VERSION) {
        len_real = strlen(_POSIX_GNU_LIBC_VERSION);

        if (len < len_real) {
            return 0;
        }

        strcpy(buf, _POSIX_GNU_LIBC_VERSION);

        return len_real;
    } else if (name == _CS_GNU_LIBPTHREAD_VERSION) {
        len_real = strlen(_POSIX_GNU_LIBPTHREAD_VERSION);

        if (len < len_real) {
            return 0;
        }

        strcpy(buf, _POSIX_GNU_LIBPTHREAD_VERSION);

        return len_real;
    } else {
        return 0;
    }
}

