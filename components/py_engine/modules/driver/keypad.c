#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/mperrno.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "ulog/ulog.h"

#define LOG_TAG "DRIVER_KEYPAD"

extern const mp_obj_type_t driver_keypad_type;

// this is the actual C-structure for our new object
typedef struct
{
    // base represents some basic information, like type
    mp_obj_base_t Base;
    // a member created by us
    char *ModuleName;
} mp_keypad_obj_t;

void keypad_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    LOGD(LOG_TAG, "entern %s;\n", __func__);
    mp_keypad_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ModuleName(%s)", self->ModuleName);
}

STATIC mp_obj_t keypad_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s;\n", __func__);
    mp_keypad_obj_t* driver_obj = m_new_obj(mp_keypad_obj_t);
    if (!driver_obj) {
        mp_raise_OSError(MP_EINVAL);
    }

    driver_obj->Base.type = &driver_keypad_type;
    driver_obj->ModuleName = "keypad";

    return MP_OBJ_FROM_PTR(driver_obj);
}

STATIC mp_obj_t obj_open(size_t n_args, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s; n_args = %d;\n", __func__, n_args);
    int ret = -1;
    void* instance = NULL;
    if (n_args < 5)
    {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_keypad_obj_t* driver_obj = (mp_keypad_obj_t *)self;
    if (driver_obj == NULL)
    {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }
    LOGD(LOG_TAG, "%s:out\n", __func__);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(keypad_obj_open, 5, obj_open);

STATIC mp_obj_t obj_close(size_t n_args, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s; n_args = %d;\n", __func__, n_args);
    int ret = -1;
    void* instance = NULL;
    if (n_args < 5)
    {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_keypad_obj_t* driver_obj = (mp_keypad_obj_t *)self;
    if (driver_obj == NULL)
    {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }
    LOGD(LOG_TAG, "%s:out\n", __func__);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(keypad_obj_close, 5, obj_close);

STATIC mp_obj_t obj_on(size_t n_args, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s; n_args = %d;\n", __func__, n_args);
    int ret = -1;
    void* instance = NULL;
    if (n_args < 5)
    {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_keypad_obj_t* driver_obj = (mp_keypad_obj_t *)self;
    if (driver_obj == NULL)
    {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }
    LOGD(LOG_TAG, "%s:out\n", __func__);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(keypad_obj_on, 5, obj_on);

STATIC const mp_rom_map_elem_t keypad_locals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_KeyPad)},
    {MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&keypad_obj_open)},
    {MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&keypad_obj_close)},
    {MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&keypad_obj_on)},
};

STATIC MP_DEFINE_CONST_DICT(keypad_locals_dict, keypad_locals_dict_table);

const mp_obj_type_t driver_keypad_type = {
    .base = {&mp_type_type},
    .name = MP_QSTR_KeyPad,
    .print = keypad_obj_print,
    .make_new = keypad_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&keypad_locals_dict,
};

