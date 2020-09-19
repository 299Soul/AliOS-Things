/*  Bluetooth Mesh */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_BT_MESH_BEACON
#include <zephyr.h>
#include <errno.h>
#include <misc/util.h>

#include <net/buf.h>
//#include <conn.h>
#include <api/mesh.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_MESH_DEBUG_BEACON)
#include "common/log.h"

#include "adv.h"
#include "mesh.h"
#include "../inc/net.h"
#include "prov.h"
#include "mesh_crypto.h"
#include "beacon.h"
#include "foundation.h"
//#include "bt_mesh_custom_log.h"

#define UNPROVISIONED_INTERVAL     K_MSEC(500)//K_SECONDS(5)
#define PROVISIONED_INTERVAL       K_SECONDS(10)

#define BEACON_TYPE_UNPROVISIONED  0x00
#define BEACON_TYPE_SECURE         0x01

/* 4 transmissions, 20ms interval */
#define UNPROV_XMIT_COUNT          3
#define UNPROV_XMIT_INT            100

/* 4 transmission, 20ms interval */
#define PROV_XMIT_COUNT            3
#define PROV_XMIT_INT              20

static struct k_delayed_work beacon_timer;

const struct bt_mesh_prov *bt_mesh_prov_get(void);
void unprovisioned_device_found(struct net_buf_simple *buf);

static struct bt_mesh_subnet *cache_check(u8_t data[21])
{
    int i;

    for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
        struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

        if (sub->net_idx == BT_MESH_KEY_UNUSED) {
            continue;
        }

        if (!memcmp(sub->beacon_cache, data, 21)) {
            return sub;
        }
    }

    return NULL;
}

static void cache_add(u8_t data[21], struct bt_mesh_subnet *sub)
{
    memcpy(sub->beacon_cache, data, 21);
}

static void beacon_complete(int err, void *user_data)
{
    struct bt_mesh_subnet *sub = user_data;

    BT_DBG("err %d", err);

    sub->beacon_sent = k_uptime_get_32();
}

void bt_mesh_beacon_create(struct bt_mesh_subnet *sub,
                           struct net_buf_simple *buf)
{
    u8_t flags = bt_mesh_net_flags(sub);
    struct bt_mesh_subnet_keys *keys;

    net_buf_simple_add_u8(buf, BEACON_TYPE_SECURE);

    if (sub->kr_flag) {
        /**
         *
         * Config Client Model initiates key refresh, so when kr_flags = 1,
         * we should use old key for the SNB
         * TODO: need to change to find Config Client Model in Primary Element
         *
        */
        #ifdef CONFIG_BT_MESH_CFG_CLI
        keys = &sub->keys[0];
        #else
        keys = &sub->keys[1];
        #endif
    } else {
        keys = &sub->keys[0];
    }

    net_buf_simple_add_u8(buf, flags);

    /* Network ID */
    net_buf_simple_add_mem(buf, keys->net_id, 8);

    /* IV Index */
    net_buf_simple_add_be32(buf, bt_mesh.iv_index);

    net_buf_simple_add_mem(buf, sub->auth, 8);

    BT_DBG("net_idx 0x%04x flags 0x%02x NetID %s", sub->net_idx,
           flags, bt_hex(keys->net_id, 8));
    BT_DBG("IV Index 0x%08x Auth %s", bt_mesh.iv_index,
           bt_hex(sub->auth, 8));
}

void bt_mesh_beacon_create_with_flag(struct bt_mesh_subnet *sub,
                           struct net_buf_simple *buf, uint8_t kr_flag, uint8_t iv_flag)
{
    u8_t flags = 0;
    struct bt_mesh_subnet_keys *keys;

    net_buf_simple_add_u8(buf, BEACON_TYPE_SECURE);

    if (sub->kr_flag) {
        keys = &sub->keys[1];
    } else {
        keys = &sub->keys[0];
    }

    if (kr_flag) flags |= BT_MESH_NET_FLAG_KR;
    if (iv_flag) flags |= BT_MESH_NET_FLAG_IVU;

    net_buf_simple_add_u8(buf, flags);

    /* Network ID */
    net_buf_simple_add_mem(buf, keys->net_id, 8);

    /* IV Index */
    net_buf_simple_add_be32(buf, bt_mesh.iv_index);

    net_buf_simple_add_mem(buf, sub->auth, 8);

    BT_DBG("net_idx 0x%04x flags 0x%02x NetID %s", sub->net_idx,
           flags, bt_hex(keys->net_id, 8));
    BT_DBG("IV Index 0x%08x Auth %s", bt_mesh.iv_index,
           bt_hex(sub->auth, 8));
}

/* If the interval has passed or is within 5 seconds from now send a beacon */
#define BEACON_THRESHOLD(sub) (K_SECONDS(10 * ((sub)->beacons_last + 1)) - \
                   K_SECONDS(5))

static int secure_beacon_send(void)
{
    static const struct bt_mesh_send_cb send_cb = {
        .end = beacon_complete,
    };
    u32_t now = k_uptime_get_32();
    int i;

    for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
        struct bt_mesh_subnet *sub = &bt_mesh.sub[i];
        struct net_buf *buf;
        u32_t time_diff;

        if (sub->net_idx == BT_MESH_KEY_UNUSED) {
            continue;
        }

        time_diff = now - sub->beacon_sent;
        if (time_diff < K_SECONDS(600) &&
            time_diff < BEACON_THRESHOLD(sub)) {
            continue;
        }

        buf = bt_mesh_adv_create(BT_MESH_ADV_BEACON, PROV_XMIT_COUNT,
                                 PROV_XMIT_INT, K_NO_WAIT);
        if (!buf) {
            BT_ERR("Unable to allocate beacon buffer");
            return -ENOBUFS;
        }

        bt_mesh_beacon_create(sub, &buf->b);

        bt_mesh_adv_send(buf, &send_cb, sub);
        BT_DBG("%s: secure beacon sent (subnet: %d).", __func__, i);
        net_buf_unref(buf);
    }

    return 0;
}

static int sbeacon_send_with_flag(uint8_t kr_flag, uint8_t iv_flag)
{
    static const struct bt_mesh_send_cb send_cb = {
        .end = beacon_complete,
    };
    u32_t now = k_uptime_get_32();
    int i;

    for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
        struct bt_mesh_subnet *sub = &bt_mesh.sub[i];
        struct net_buf *buf;
        u32_t time_diff;

        if (sub->net_idx == BT_MESH_KEY_UNUSED) {
            continue;
        }

        time_diff = now - sub->beacon_sent;
        if (time_diff < K_SECONDS(600) &&
            time_diff < BEACON_THRESHOLD(sub)) {
            continue;
        }

        buf = bt_mesh_adv_create(BT_MESH_ADV_BEACON, PROV_XMIT_COUNT,
                                 PROV_XMIT_INT, K_NO_WAIT);
        if (!buf) {
            BT_ERR("Unable to allocate beacon buffer");
            return -ENOBUFS;
        }

        bt_mesh_beacon_create_with_flag(sub, &buf->b, kr_flag, iv_flag);

        bt_mesh_adv_send(buf, &send_cb, sub);
        BT_DBG("%s: secure beacon sent (subnet: %d).", __func__, i);
        net_buf_unref(buf);
    }

    return 0;
}

int secure_beacon_send2()
{
    return secure_beacon_send();
}

int secure_beacon_send_with_flag(uint8_t kr_flag, uint8_t iv_flag)
{
    return sbeacon_send_with_flag(kr_flag, iv_flag);
}

static int unprovisioned_beacon_send(void)
{
#if defined(CONFIG_BT_MESH_PB_ADV)
    const struct bt_mesh_prov *prov;
    struct net_buf *buf;
    u8_t uri_hash[16] = { 0 };
    u16_t oob_info;

    BT_DBG("");

    buf = bt_mesh_adv_create(BT_MESH_ADV_BEACON, UNPROV_XMIT_COUNT,
                             UNPROV_XMIT_INT, K_NO_WAIT);
    if (!buf) {
        BT_ERR("Unable to allocate beacon buffer");
        return -ENOBUFS;
    }

    prov = bt_mesh_prov_get();

    net_buf_add_u8(buf, BEACON_TYPE_UNPROVISIONED);
    net_buf_add_mem(buf, prov->uuid, 16);

    /* for tmall ble profile, OOB info default is 0x0000
       URI hash is optional */
    if (prov->uri && bt_mesh_s1(prov->uri, uri_hash) == 0) {
        oob_info = prov->oob_info | BT_MESH_PROV_OOB_URI;
    } else {
        oob_info = prov->oob_info;
    }

    net_buf_add_be16(buf, oob_info);
    net_buf_add_mem(buf, uri_hash, 4);

    bt_mesh_adv_send(buf, NULL, NULL);
    net_buf_unref(buf);

    if (prov->uri) {
        size_t len;

        buf = bt_mesh_adv_create(BT_MESH_ADV_URI, UNPROV_XMIT_COUNT,
                                 UNPROV_XMIT_INT, K_NO_WAIT);
        if (!buf) {
            BT_ERR("Unable to allocate URI buffer");
            return -ENOBUFS;
        }

        len = strlen(prov->uri);
        if (net_buf_tailroom(buf) < len) {
            BT_WARN("Too long URI to fit advertising data");
        } else {
            net_buf_add_mem(buf, prov->uri, len);
            bt_mesh_adv_send(buf, NULL, NULL);
        }

        net_buf_unref(buf);
    }

#endif /* CONFIG_BT_MESH_PB_ADV */
    return 0;
}

static void update_beacon_observation(void)
{
    static bool first_half;
    int i;

    /* Observation period is 20 seconds, whereas the beacon timer
     * runs every 10 seconds. We process what's happened during the
     * window only after the seconnd half.
     */
    first_half = !first_half;
    if (first_half) {
        return;
    }

    for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
        struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

        if (sub->net_idx == BT_MESH_KEY_UNUSED) {
            continue;
        }

        sub->beacons_last = sub->beacons_cur;
        sub->beacons_cur = 0;
    }
}

static void beacon_send(struct k_work *work)
{
    /* Don't send anything if we have an active provisioning link */
    if (IS_ENABLED(CONFIG_BT_MESH_PROV) && bt_prov_active()) {
        k_delayed_work_submit(&beacon_timer, UNPROVISIONED_INTERVAL);
        return;
    }

    //BT_DBG("");

    if (bt_mesh_is_provisioned()) {
        update_beacon_observation();
        secure_beacon_send();

        /* Only resubmit if beaconing is still enabled */
        if (bt_mesh_beacon_get() == BT_MESH_BEACON_ENABLED ||
            bt_mesh.ivu_initiator) {
            k_delayed_work_submit(&beacon_timer,
                                  PROVISIONED_INTERVAL);
        }
    } else {
        unprovisioned_beacon_send();
        k_delayed_work_submit(&beacon_timer, UNPROVISIONED_INTERVAL);
#if 0   //Lancelot
        if(pb_adv_active() == 0) {
        }
#endif
    }

}

static void secure_beacon_recv(struct net_buf_simple *buf)
{
    u8_t *data, *net_id, *auth;
    struct bt_mesh_subnet *sub;
    u32_t iv_index;
    bool new_key, kr_change, iv_change;
    u8_t flags;

    if (buf->len < 21) {
        BT_ERR("Too short secure beacon (len %u)", buf->len);
        return;
    }

    sub = cache_check(buf->data);
    if (sub) {
        /* We've seen this beacon before - just update the stats */
        goto update_stats;
    }

    /* So we can add to the cache if auth matches */
    data = buf->data;

    flags = net_buf_simple_pull_u8(buf);
    net_id = buf->data;
    net_buf_simple_pull(buf, 8);
    iv_index = net_buf_simple_pull_be32(buf);
    auth = buf->data;

    BT_DBG("flags 0x%02x id %s iv_index 0x%08x",
           flags, bt_hex(net_id, 8), iv_index);

    sub = bt_mesh_subnet_find(net_id, flags, iv_index, auth, &new_key);
    if (!sub) {
        BT_DBG("No subnet that matched beacon");
        return;
    }

    if (sub->kr_phase == BT_MESH_KR_PHASE_2 && !new_key) {
        BT_WARN("Ignoring Phase 2 KR Update secured using old key");
        return;
    }

    cache_add(data, sub);

    /* If we have NetKey0 accept initiation only from it */
    if (bt_mesh_subnet_get(BT_MESH_KEY_PRIMARY) &&
        sub->net_idx != BT_MESH_KEY_PRIMARY) {
        BT_WARN("Ignoring secure beacon on non-primary subnet");
        goto update_stats;
    }

    BT_DBG("net_idx 0x%04x iv_index 0x%08x, current iv_index 0x%08x",
           sub->net_idx, iv_index, bt_mesh.iv_index);

    if (bt_mesh.ivu_initiator &&
        bt_mesh.iv_update == BT_MESH_IV_UPDATE(flags)) {
        bt_mesh_beacon_ivu_initiator(false);
    }

    iv_change = bt_mesh_net_iv_update(iv_index, BT_MESH_IV_UPDATE(flags));

    kr_change = bt_mesh_kr_update(sub, BT_MESH_KEY_REFRESH(flags), new_key);
    if (kr_change) {
        bt_mesh_net_beacon_update(sub);
    }

    if (iv_change) {
        /* Update all subnets */
        bt_mesh_net_sec_update(NULL);
    } else if (kr_change) {
        /* Key Refresh without IV Update only impacts one subnet */
        bt_mesh_net_sec_update(sub);
    }

update_stats:
    if (bt_mesh_beacon_get() == BT_MESH_BEACON_ENABLED &&
        sub->beacons_cur < 0xff) {
        sub->beacons_cur++;
    }
}

#ifdef CONFIG_BT_MESH_PROVISIONER
#define PROVISIONER_IDLE        0
#define PROVISIONER_SCANNING    1
#define PROVISIOINER_PROV       2

static u8_t provisioner_state = PROVISIONER_IDLE;

void set_provisioner_state(u8_t state)
{
    provisioner_state = state;
}

u8_t get_provisioner_state(void)
{
    return provisioner_state;
}
#endif

void bt_mesh_beacon_recv(struct net_buf_simple *buf)
{
    u8_t type;

    //BT_DBG("%u bytes: %s", buf->len, bt_hex(buf->data, buf->len));

    if (buf->len < 1) {
        BT_ERR("Too short beacon");
        return;
    }

    type = net_buf_simple_pull_u8(buf);
    switch (type) {
        case BEACON_TYPE_UNPROVISIONED:
            #ifdef CONFIG_BT_MESH_PROVISIONER
            if (get_provisioner_state() == PROVISIONER_SCANNING){
                BT_DBG("unprov device beacon Found");
                //TODO
                unprovisioned_device_found(buf);
            }
            #else
                BT_DBG("Ignoring unprovisioned device beacon");
            #endif
            break;
        case BEACON_TYPE_SECURE:
            secure_beacon_recv(buf);
            break;
        default:
            BT_WARN("Unknown beacon type 0x%02x", type);
            break;
    }
}

void bt_mesh_beacon_init(void)
{
    k_delayed_work_init(&beacon_timer, beacon_send);
}

void bt_mesh_beacon_ivu_initiator(bool enable)
{
    bt_mesh.ivu_initiator = enable;

    if (enable) {
        k_work_submit(&beacon_timer.work);
    } else if (bt_mesh_beacon_get() == BT_MESH_BEACON_DISABLED) {
        k_delayed_work_cancel(&beacon_timer);
    }
}

void bt_mesh_beacon_enable(void)
{
    int i;

    if (!bt_mesh_is_provisioned()) {
        k_work_submit(&beacon_timer.work);
        return;
    }

    for (i = 0; i < ARRAY_SIZE(bt_mesh.sub); i++) {
        struct bt_mesh_subnet *sub = &bt_mesh.sub[i];

        if (sub->net_idx == BT_MESH_KEY_UNUSED) {
            continue;
        }

        sub->beacons_last = 0;
        sub->beacons_cur = 0;

        bt_mesh_net_beacon_update(sub);
    }

    k_work_submit(&beacon_timer.work);
}

void bt_mesh_beacon_disable(void)
{
    if (!bt_mesh.ivu_initiator) {
        k_delayed_work_cancel(&beacon_timer);
    }
}

#endif