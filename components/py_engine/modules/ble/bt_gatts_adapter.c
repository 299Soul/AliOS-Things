/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <aos/ble.h>
#include <aos/errno.h>
#include <aos/gatt.h>
#include <aos/kernel.h>
#include <atomic.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>
#include <k_api.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <work.h>

#include "aos/init.h"
#include "modble.h"

#define LOG_TAG "BT_GATTS_ADAPTER"

size_t g_attr_num = 0;
gatt_attr_t *g_attrs_list = NULL;

int bt_gatts_adapter_update_user_data(size_t index, uint16_t uuid,
                                      uint8_t *data, size_t len);

uint16_t get_16bits_hex_from_string(const char *str)
{
    uint16_t ret = 0, tmp = 0, ii = 0, jj = 0;
    uint32_t sample_len = strlen("0xAABB");
    uint8_t seed[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    if (strlen(str) == sample_len) {
        if (strncmp(str, "0x", 2) && strncmp(str, "0X", 2)) {
        } else {
            for (ii = sample_len - 1; ii >= sample_len - 4; ii--) {
                // to uppercase
                if (str[ii] >= 'a') {
                    tmp = str[ii] - ('a' - 'A');
                } else {
                    tmp = str[ii];
                }
                // char to number
                for (jj = 0; jj < sizeof(seed); jj++) {
                    if (seed[jj] == tmp) {
                        break;
                    }
                }
                ret += jj << ((sample_len - 1 - ii) * 4);
            }
        }
    }
    // LOGD(LOG_TAG, "[%s] %s -> 0x%X", __func__, str, ret);
    return ret;
}

gatt_perm_en get_attr_perm_from_string(char *str)
{
    gatt_perm_en ret = GATT_PERM_NONE;
    if (strstr(str, "R")) {
        ret |= GATT_PERM_READ;
    }
    if (strstr(str, "W")) {
        ret |= GATT_PERM_WRITE;
    }
    return ret;
}

gatt_prop_en get_char_perm_from_string(char *str)
{
    gatt_prop_en ret = 0;
    if (strstr(str, "R")) {
        ret |= GATT_CHRC_PROP_READ;
    }
    if (strstr(str, "W")) {
        ret |= GATT_CHRC_PROP_WRITE;
    }
    return ret;
}

static gatt_service srvc_instance = { 0 };

uuid_t *bt_gatts_declare_16bits_uuid(uint16_t val)
{
    struct ut_uuid_16 *uuid =
        (struct ut_uuid_16 *)malloc(sizeof(struct ut_uuid_16));
    uuid->uuid.type = UUID_TYPE_16;
    uuid->val = val;
    return (uuid_t *)uuid;
}

void bt_gatts_define_gatt_srvc(gatt_attr_t *out, uint16_t uuid)
{
    LOGD(LOG_TAG, "[%s] declare service uuid 0x%04x", __func__, uuid);

    out->uuid = bt_gatts_declare_16bits_uuid(0x2800);  // UUID_GATT_PRIMARY;
    out->perm = GATT_PERM_READ;
    out->read = out->write = NULL;
    out->user_data = (void *)bt_gatts_declare_16bits_uuid(uuid);
}

void bt_gatts_define_gatt_char(gatt_attr_t *out, uint16_t uuid, uint8_t perimt)
{
    LOGD(LOG_TAG, "[%s] declare char uuid 0x%04x", __func__, uuid);

    struct gatt_char_t *data =
        (struct gatt_char_t *)malloc(sizeof(struct gatt_char_t));
    memset(data, 0, sizeof(struct gatt_char_t));
    data->uuid = bt_gatts_declare_16bits_uuid(uuid);
    data->value_handle = 0;
    data->properties = perimt,

    out->uuid = bt_gatts_declare_16bits_uuid(0x2803);  // UUID_GATT_CHRC;
    out->perm = GATT_PERM_READ;
    out->read = out->write = NULL;
    out->user_data = (void *)data;
}

void bt_gatts_define_gatt_char_val(gatt_attr_t *out, uint16_t uuid,
                                   uint8_t perimt)
{
    LOGD(LOG_TAG, "[%s] declare char val uuid 0x%04x", __func__, uuid);
    out->uuid = bt_gatts_declare_16bits_uuid(uuid);
    out->perm = perimt;
    out->read = out->write = NULL;
    out->user_data = NULL;
}

void bt_gatts_define_ccc(gatt_attr_t *out)
{
    LOGD(LOG_TAG, "[%s] declare ccc ...", __func__);

    struct bt_gatt_ccc_t *ccc =
        (struct bt_gatt_ccc_t *)malloc(sizeof(struct bt_gatt_ccc_t));
    memset(ccc, 0, sizeof(struct bt_gatt_ccc_t));
    out->uuid = bt_gatts_declare_16bits_uuid(0x2902);  // UUID_GATT_CCC;
    out->perm = GATT_PERM_READ | GATT_PERM_WRITE;
    out->read = out->write = NULL;
    out->user_data = (void *)ccc;
}

int bt_gatts_adapter_add_service(amp_bt_host_adapter_gatts_srvc_t *srvc)
{
    int ret = 0;
    amp_bt_host_adapter_gatt_chars_t *char_list = NULL;

    if (srvc) {
        g_attrs_list =
            (gatt_attr_t *)aos_malloc(srvc->attr_cnt * sizeof(gatt_attr_t));
        if (!g_attrs_list) {
            LOGE(LOG_TAG, "[%s] memory not enough for g_attrs_list", __func__);
            return ret;
        }

        memset(g_attrs_list, 0, srvc->attr_cnt * sizeof(gatt_attr_t));

        char_list = srvc->chars;

        // declare the service
        bt_gatts_define_gatt_srvc(&g_attrs_list[g_attr_num++],
                                  get_16bits_hex_from_string(srvc->s_uuid));

        while (g_attr_num < srvc->attr_cnt) {
            if (char_list->descr_type) {
                if (0 == strcmp(char_list->descr_type, "CCC")) {
                    bt_gatts_define_gatt_char(
                        &g_attrs_list[g_attr_num++],
                        get_16bits_hex_from_string(char_list->char_uuid),
                        GATT_CHRC_PROP_NOTIFY |
                            get_char_perm_from_string(char_list->permission));
                    bt_gatts_define_gatt_char_val(
                        &g_attrs_list[g_attr_num++],
                        get_16bits_hex_from_string(char_list->char_uuid),
                        get_attr_perm_from_string(char_list->permission));
                    bt_gatts_define_ccc(&g_attrs_list[g_attr_num++]);
                    char_list++;
                    continue;
                } else {
                    /* CUD, SCC, CPF, CAF*/
                    LOGW(LOG_TAG, "[%s] unsupported descr type: %s ", __func__,
                         char_list->descr_type);
                }
            }
            bt_gatts_define_gatt_char(
                &g_attrs_list[g_attr_num++],
                get_16bits_hex_from_string(char_list->char_uuid),
                GATT_CHRC_PROP_NOTIFY |
                    get_char_perm_from_string(char_list->permission));
            bt_gatts_define_gatt_char_val(
                &g_attrs_list[g_attr_num++],
                get_16bits_hex_from_string(char_list->char_uuid),
                get_attr_perm_from_string(char_list->permission));
            char_list++;
        }
        LOGD(LOG_TAG, "[%s] declare service done, total attr: %d(%d)", __func__,
             g_attr_num, srvc->attr_cnt);
        // >=0: handle, <0: error
        ret = ble_stack_gatt_registe_service(&srvc_instance, g_attrs_list,
                                             srvc->attr_cnt);

        LOGD(LOG_TAG, "[%s] add service done with ret %d", __func__, ret);
    } else {
        LOGE(LOG_TAG, "[%s] srvc is null", __func__);
    }
    return ret;
}

int bt_gatts_adapter_update_user_data(size_t index, uint16_t uuid_val,
                                      uint8_t *data, size_t len)
{
    int ret = -1;
    int ii;
    gatt_attr_t *ptr = NULL;

    if (index >= 0 && index < g_attr_num) {
        ptr = &(g_attrs_list[index]);
    } else {
        for (ii = 0; ii < g_attr_num; ii++) {
            if (((struct ut_uuid_16 *)(g_attrs_list[ii].uuid))->val ==
                uuid_val) {
                ptr = &(g_attrs_list[ii]);
                ret = ii;
                break;
            }
        }
    }

    if (ptr == NULL) {
        LOGE(LOG_TAG, "[%s] give up updating as uuid not matched!!", __func__);
        return -1;
    }

    amp_bt_gatts_adapter_usr_data_t *old =
        (amp_bt_gatts_adapter_usr_data_t *)ptr->user_data;
    amp_bt_gatts_adapter_usr_data_t **new =
        (amp_bt_gatts_adapter_usr_data_t **)&(ptr->user_data);

    if (old) {
        if (old->data) {
            free(old->data);
        } else {
            LOGE(
                LOG_TAG,
                "[%s] old is avaliable, but data is null. should NOT be here!!",
                __func__);
        }
        free(old);
    }

    *new = (amp_bt_gatts_adapter_usr_data_t *)malloc(
        sizeof(amp_bt_gatts_adapter_usr_data_t));
    if (*new) {
        (*new)->len = len;
        (*new)->data = (uint8_t *)malloc(sizeof(uint8_t) * len);
        if ((*new)->data) {
            memcpy((*new)->data, data, len);
            LOGD(LOG_TAG, "[%s]update user data:%p -> %p", __func__, old, *new);
        } else {
            free(*new);
            LOGE(LOG_TAG, "[%s][%d] memory not enough for new data!", __func__,
                 __LINE__);
        }
    } else {
        LOGE(LOG_TAG, "[%s][%d] memory not enough for new data!", __func__,
             __LINE__);
    }
    return ret;
}

static int bt_gatts_adapter_char_get_desc_idx(int start_idx, int16_t uuid)
{
    int idx = -1;
    int i;

    for (i = start_idx; i < g_attr_num; i++) {
        if (UUID16(g_attrs_list[i].uuid) == uuid) {
            idx = i;
            break;
        }
        if (UUID16(g_attrs_list[i].uuid) == UUID16(UUID_GATT_CHRC)) {
            break;
        }
    }

    return idx;
}

int bt_gatts_adapter_update_chars(char *uuid, uint8_t *data, size_t len)
{
    amp_bt_gatts_adapter_usr_data_t *old_data = NULL;

    int32_t index, ccc_index;

    index = bt_gatts_adapter_update_user_data(
        -1, get_16bits_hex_from_string(uuid), data, len);
    LOGD(LOG_TAG, "[%s] index = %d, uuid = %s, len = %d", __func__, index, uuid,
         len);
    if (index < 0) {
        return -1;
    }

    if (index < g_attr_num) {
        uint16_t *conn_handle = NULL;
        struct bt_gatt_ccc_t *ccc;

        ccc_index =
            bt_gatts_adapter_char_get_desc_idx(index, UUID16(UUID_GATT_CCC));
        if (ccc_index < 0) {
            LOGE(LOG_TAG, "[%s]ccc not found", __func__);
            return -1;
        }
        ccc = (struct bt_gatt_ccc_t *)g_attrs_list[ccc_index].user_data;

        for (int jj = 0; jj < ARRAY_SIZES(ccc->cfg); jj++) {
            conn_handle = (uint16_t *)bt_conn_lookup_addr_le(
                ccc->cfg->id, (const bt_addr_le_t *)&ccc->cfg[jj].peer);
            if (conn_handle > 0) {
                if (ccc->cfg[0].value) {
                    ble_stack_gatt_notificate(
                        *conn_handle, g_attrs_list[index].handle, data, len);
                }
            }
        }
    }
    return 0;
}

void bt_gatts_dump_hex(uint8_t data[], size_t len)
{
    printf("\t\tDUMP START: ");
    for (int ii = 0; ii < len; ii++) {
        printf("%02x ", data[ii]);
    }
    printf("\n");
}

int bt_gatts_adapter_event_callback(ble_event_en event, void *event_data)
{
    switch (event) {
    case EVENT_GATT_CHAR_READ:
        {
            evt_data_gatt_char_read_t *read_data =
                (evt_data_gatt_char_read_t *)event_data;
            for (int ii = 0; ii < g_attr_num; ii++) {
                if (g_attrs_list[ii].handle == read_data->char_handle) {
                    LOGD(LOG_TAG,
                         "[%s]read request at handle.%d, uuid:0x%04x, "
                         "offset:%d",
                         __func__, read_data->char_handle,
                         ((struct ut_uuid_16 *)(g_attrs_list[ii].uuid))->val,
                         read_data->offset);
                    amp_bt_gatts_adapter_usr_data_t *data =
                        (amp_bt_gatts_adapter_usr_data_t *)g_attrs_list[ii]
                            .user_data;
                    if (data) {
                        if (data->data && data->len) {
                            bt_gatts_dump_hex(data->data, data->len);
                            read_data->data = data->data;
                            // memcpy(read_data->data, data->data,
                            // data->len);
                            read_data->len = data->len;
                            break;
                        }
                    }
                    LOGE(LOG_TAG, "[%s][%d] no data to be read!", __func__,
                         __LINE__);
                }
            }
        }
        break;
    case EVENT_GATT_CHAR_WRITE:
        {
            evt_data_gatt_char_write_t *write_data =
                (evt_data_gatt_char_write_t *)event_data;
            for (int ii = 0; ii < g_attr_num; ii++) {
                if (g_attrs_list[ii].handle == write_data->char_handle) {
                    LOGD(LOG_TAG,
                         "[%s]read request at handle.%d, uuid:0x%04x, "
                         "offset:%d\n",
                         __func__, write_data->char_handle,
                         ((struct ut_uuid_16 *)(g_attrs_list[ii].uuid))->val,
                         write_data->offset);
                    bt_gatts_adapter_update_user_data(
                        ii, 0, (uint8_t *)write_data->data, write_data->len);
                    amp_bt_gatts_adapter_usr_data_t *data =
                        (amp_bt_gatts_adapter_usr_data_t *)g_attrs_list[ii]
                            .user_data;
                    // bt_gatts_dump_hex(data->data, data->len);
                    native_bt_host_gatts_handle_write(
                        (uint8_t *)write_data->data, write_data->len);
                    break;
                }
            }
        }
        break;
    case EVENT_GATT_CHAR_CCC_CHANGE:
        {
            evt_data_gatt_char_ccc_change_t *data =
                (evt_data_gatt_char_ccc_change_t *)event_data;
            for (int ii = 0; ii < g_attr_num; ii++) {
                if (g_attrs_list[ii].handle == data->char_handle) {
                    struct bt_gatt_ccc_t *ccc =
                        (struct bt_gatt_ccc_t *)g_attrs_list[ii].user_data;
                    LOGD(LOG_TAG,
                         "[%s]ccc-changed at handle.%d 0x%04x, 0x%04x,\n",
                         __func__, data->char_handle, data->ccc_value,
                         ccc->value);
                }
            }
        }
        break;
    default:
        break;
    }
}
