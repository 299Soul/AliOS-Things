/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "aos_hal_rtc.h"
#include "shared/timeutils/timeutils.h"
#include "modmachine.h"
#include "py/mphal.h"
#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "ulog/ulog.h"

#define LOG_TAG "machine_rtc"

const mp_obj_type_t machine_rtc_type;

typedef struct _machine_rtc_obj_t {
    mp_obj_base_t base;
    rtc_dev_t dev;
} machine_rtc_obj_t;

// singleton RTC object
STATIC const machine_rtc_obj_t machine_rtc_obj = { { &machine_rtc_type } };

STATIC mp_obj_t machine_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    rtc_dev_t *dev = &(machine_rtc_obj.dev);
    dev->port = 0;
    dev->config.format = HAL_RTC_FORMAT_DEC;

    // return constant object
    return (mp_obj_t)&machine_rtc_obj;
}

STATIC mp_obj_t machine_rtc_datetime_helper(mp_uint_t n_args, const mp_obj_t *args)
{
    machine_rtc_obj_t *self = (machine_rtc_obj_t *)MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        // Get time
        rtc_time_t tm = { 0 };

        aos_hal_rtc_get_time(&self->dev, &tm);

        mp_obj_t tuple[7] = {
            mp_obj_new_int(tm.year), mp_obj_new_int(tm.month), mp_obj_new_int(tm.date), mp_obj_new_int(tm.weekday),
            mp_obj_new_int(tm.hr),   mp_obj_new_int(tm.min),   mp_obj_new_int(tm.sec),
        };

        return mp_obj_new_tuple(7, tuple);
    } else {
        // Set time

        mp_obj_t *items = NULL;
        mp_obj_get_array_fixed_n(args[1], 8, &items);

        rtc_time_t tm = {
            .year = mp_obj_get_int(items[0]),
            .month = mp_obj_get_int(items[1]),
            .date = mp_obj_get_int(items[2]),
            .weekday = mp_obj_get_int(items[3]),
            .hr = mp_obj_get_int(items[4]),
            .min = mp_obj_get_int(items[5]),
            .sec = mp_obj_get_int(items[6]),
        };

        aos_hal_rtc_set_time(&self->dev, &tm);

        return mp_const_none;
    }
}
STATIC mp_obj_t machine_rtc_datetime(mp_uint_t n_args, const mp_obj_t *args)
{
    return machine_rtc_datetime_helper(n_args, args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_datetime_obj, 1, 2, machine_rtc_datetime);

STATIC mp_obj_t machine_rtc_init(mp_obj_t self_in, mp_obj_t date)
{
    mp_obj_t args[2] = { self_in, date };
    machine_rtc_datetime_helper(2, args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_init_obj, machine_rtc_init);

STATIC const mp_rom_map_elem_t machine_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_rtc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_datetime), MP_ROM_PTR(&machine_rtc_datetime_obj) },
};
STATIC MP_DEFINE_CONST_DICT(machine_rtc_locals_dict, machine_rtc_locals_dict_table);

const mp_obj_type_t machine_rtc_type = {
    { &mp_type_type },
    .name = MP_QSTR_RTC,
    .make_new = machine_rtc_make_new,
    .locals_dict = (mp_obj_t)&machine_rtc_locals_dict,
};