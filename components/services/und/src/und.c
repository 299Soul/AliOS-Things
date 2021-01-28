/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

#ifdef FEATURE_UND_SUPPORT

#include "und/und.h"
#include "und_log.h"
#include "und_types.h"
#include "und_utils.h"
#include "und_sched.h"
#include "und_config.h"
#include "und_packet.h"
#include "und_report.h"
#include "und_manage.h"
#include "und_platform.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

struct und_ctx_t {
    void *mutex;
};

static struct und_ctx_t g_und_ctx = {0};

int und_init()
{
    int res;
    void *mutex = NULL;
    struct und_ctx_t *ctx = &g_und_ctx;

    if (ctx->mutex) {
        und_debug("und is ready inited\n");
        return UND_SUCCESS;
    }
    ctx->mutex = und_platform_mutex_create();

    UND_PTR_SANITY_CHECK(ctx->mutex, UND_MEM_ERR);

    und_platform_mutex_lock(ctx->mutex);
    mutex = ctx->mutex;

    if (UND_SUCCESS != (res = und_packet_ctx_init())) {
        und_err("und pkt ctx init fail\n");
        goto UND_INIT_FAIL;
    }

    if (UND_SUCCESS != (res = und_cap_manage_init())) {
        und_err("und cap ctx init fail\n");
        goto UND_INIT_FAIL;
    }

    if (UND_SUCCESS != (res = und_sched_init(und_targets_report))) {
        und_err("und sched init fail\n");
        goto UND_INIT_FAIL;
    }

    if (UND_SUCCESS != (res = und_report_init())) {
        und_err("und report init fail\n");
        goto UND_INIT_FAIL;
    }

    und_platform_mutex_unlock(ctx->mutex);
    return UND_SUCCESS;

UND_INIT_FAIL:
    und_sched_deinit();
    und_cap_manage_deinit();
    und_packet_ctx_deinit();
    und_report_deinit();
    und_platform_memset(ctx, 0, sizeof(*ctx));
    und_platform_mutex_unlock(mutex);
    und_platform_mutex_destroy(mutex);

    return res;
}

int und_deinit()
{

    struct und_ctx_t *ctx = &g_und_ctx;
    void *mutex = ctx->mutex;
    int res;

    UND_PTR_SANITY_CHECK(ctx->mutex, UND_ERR);

    und_platform_mutex_lock(mutex);
    res = und_sched_deinit();
    res = und_cap_manage_deinit();
    res = und_packet_ctx_deinit();
    und_platform_memset(ctx, 0, sizeof(*ctx));
    und_platform_mutex_unlock(mutex);
    und_platform_mutex_destroy(mutex);

    return res;
}

int und_update_statis(int cap_idx, int reason_code)
{
    int res;
    struct und_ctx_t *ctx = &g_und_ctx;
    if (ctx->mutex == NULL) {
        res = und_init();
        if (res != UND_SUCCESS) {
            und_err("update, und init fail\n");
            return res;
        }
    }
    und_platform_mutex_lock(ctx->mutex);
    cap_idx = UND_CAPTURE_IDX(cap_idx);
    res = und_update_cap(cap_idx, reason_code, 1, 1);
    if (res == UND_SUCCESS) {
        und_update_report(cap_idx);
        res = und_sched_start(UND_REPORT_SCHED_CYCLE_MS);
    }
    und_platform_mutex_unlock(ctx->mutex);

    return res;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
#endif
