/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include "aos/kernel.h"
#include "linkkit/wrappers/wrappers_defs.h"
#include "linkkit/infra/infra_aes.h"
#include "ulog/ulog.h"
#include "aos/kv.h"

#define DEVINFO_DN "hal_devinfo_dn"
#define DEVINFO_DS "hal_devinfo_ds"
#define DEVINFO_PK "hal_devinfo_pk"
#define DEVINFO_PS "hal_devinfo_ps"
#define LOG_TAG    "HAL"
static const char *demo_iv = "f165u329c054k637";

#define PLATFORM_WAIT_INFINITE (~0)

char _product_key[IOTX_PRODUCT_KEY_LEN + 1] = { 0 };
char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = { 0 };
char _device_name[IOTX_DEVICE_NAME_LEN + 1] = { 0 };
char _device_secret[IOTX_DEVICE_SECRET_LEN + 1] = { 0 };

int HAL_SetProductKey(char *product_key)
{
    int len;
    if (product_key == NULL) {
        return -1;
    }

    memset(_product_key, 0x0, IOTX_PRODUCT_KEY_LEN + 1);
    len = strlen(product_key);
    if (len > IOTX_PRODUCT_KEY_LEN) {
        return -1;
    }

    strncpy(_product_key, product_key, len);
    return len;
}

int HAL_SetDeviceName(char *device_name)
{
    int len;
    if (device_name == NULL) {
        return -1;
    }

    memset(_device_name, 0x0, IOTX_DEVICE_NAME_LEN + 1);
    len = strlen(device_name);
    if (len > IOTX_DEVICE_NAME_LEN) {
        return -1;
    }

    strncpy(_device_name, device_name, len);
    return len;
}

int HAL_SetProductSecret(char *product_secret)
{
    int res;
    int len;

    if (product_secret == NULL) {
        return -1;
    }

    memset(_product_secret, 0x0, IOTX_PRODUCT_SECRET_LEN + 1);
    len = strlen(product_secret);
    if (len > IOTX_PRODUCT_SECRET_LEN) {
        return -1;
    }

    strncpy(_product_secret, product_secret, len);
    return len;
}

int HAL_SetDeviceSecret(char *device_secret)
{
    int len;
    if (device_secret == NULL) {
        return -1;
    }

    memset(_device_secret, 0x0, IOTX_DEVICE_SECRET_LEN + 1);
    len = strlen(device_secret);
    if (len > IOTX_DEVICE_SECRET_LEN) {
        return -1;
    }

    strncpy(_device_secret, device_secret, len);
    return len;
}

int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN + 1])
{
    int len = strlen(_product_key);
    memset(product_key, 0x0, IOTX_PRODUCT_KEY_LEN + 1);
    if (len == 0) {
        len = IOTX_PRODUCT_KEY_LEN;
        aos_kv_get(DEVINFO_PK, _product_key, &len);
    }

    strncpy(product_key, _product_key, len);

    return len;
}

int HAL_GetProductSecret(char product_secret[IOTX_PRODUCT_SECRET_LEN + 1])
{
    int len = strlen(_product_secret);
    if (len == 0) {
#if 1
        len = IOTX_PRODUCT_SECRET_LEN;
        aos_kv_get(DEVINFO_PS, _product_secret, &len);
#else
        len = strlen("kvFuluNMSGaLXIT6");
        memcpy(_product_secret, "kvFuluNMSGaLXIT6", len + 1);
#endif
    }

    memset(product_secret, 0x0, IOTX_PRODUCT_SECRET_LEN + 1);
    strncpy(product_secret, _product_secret, len);

    return len;

    return len;
}

int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN + 1])
{
    int len = strlen(_device_name);
    memset(device_name, 0x0, IOTX_DEVICE_NAME_LEN + 1);
    if (len == 0) {
        len = IOTX_DEVICE_NAME_LEN;
        aos_kv_get(DEVINFO_DN, _device_name, &len);
    }
    strncpy(device_name, _device_name, len);

    return strlen(device_name);
}

int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN + 1])
{
    int len = strlen(_device_secret);
    if (len == 0) {
#if 1
        len = IOTX_DEVICE_SECRET_LEN;
        aos_kv_get(DEVINFO_DS, _device_secret, &len);
#else
        len = strlen("1e71e85af103c118eca9409cb4132705");
        memcpy(_device_secret, "1e71e85af103c118eca9409cb4132705", len + 1);
#endif
    }

    memset(device_secret, 0x0, IOTX_DEVICE_SECRET_LEN + 1);
    strncpy(device_secret, _device_secret, len);

    return len;
}

int HAL_SaveDeviceIdentity(char *pk, char *ps, char *dn, char *ds)
{
    int res, len;

    if (HAL_SetProductKey(pk) > 0) {
        len = strlen(pk);
        aos_kv_set(DEVINFO_PK, pk, len, 1);
    }

    if (HAL_SetProductSecret(ps) > 0) {
        do {
            char dec_secret[IOTX_PRODUCT_SECRET_LEN + 1] = { 0 };
            if (strlen(_product_key) == 0) {
                HAL_GetProductKey(_product_key);
            }
            p_Aes128_t aes_e_h =
                infra_aes128_init((unsigned char *)_product_key,
                                  (unsigned char *)demo_iv, AES_ENCRYPTION);
            if (aes_e_h == NULL) {
                LOGE(LOG_TAG, "aes init failed");
                break;
            }
            len = strlen(ps);
            res = infra_aes128_cfb_encrypt(aes_e_h, _product_secret, len,
                                           dec_secret);
            infra_aes128_destroy(aes_e_h);
            if (res < 0) {
                LOGE(LOG_TAG, "encrypt ps failed");
                break;
            }
            aos_kv_set(DEVINFO_PS, dec_secret, len, 1);
        } while (0);
    }

    if (HAL_SetDeviceName(dn) > 0) {
        len = strlen(dn);
        aos_kv_set(DEVINFO_DN, dn, len, 1);
    }

    if (HAL_SetDeviceSecret(ds) > 0) {
        do {
            char dec_secret[IOTX_DEVICE_SECRET_LEN + 1] = { 0 };
            if (strlen(_product_key) == 0) {
                HAL_GetProductKey(_product_key);
            }
            p_Aes128_t aes_e_h =
                infra_aes128_init((unsigned char *)_product_key,
                                  (unsigned char *)demo_iv, AES_ENCRYPTION);
            if (aes_e_h == NULL) {
                LOGE(LOG_TAG, "aes init failed");
                break;
            }
            len = strlen(ds);
            res = infra_aes128_cfb_encrypt(aes_e_h, ds, len, dec_secret);
            infra_aes128_destroy(aes_e_h);
            if (res < 0) {
                LOGE(LOG_TAG, "encrypt ds failed");
                break;
            }
            aos_kv_set(DEVINFO_DS, dec_secret, len, 1);
        } while (0);
    }

    return 0;
}

int HAL_ClearDeviceIdentity()
{
    aos_kv_del(DEVINFO_PK);
    aos_kv_del(DEVINFO_PS);
    aos_kv_del(DEVINFO_DN);
    aos_kv_del(DEVINFO_DS);
    return 0;
}

int HAL_GetFirmwareVersion(_OU_ char version[IOTX_FIRMWARE_VERSION_LEN])
{
    memset(version, 0x0, IOTX_FIRMWARE_VERSION_LEN);

    strncpy(version, (const char *)SYSINFO_APP_VERSION,
            IOTX_FIRMWARE_VERSION_LEN - 1);
    version[IOTX_FIRMWARE_VERSION_LEN - 1] = '\0';

    return strlen(version);
}
