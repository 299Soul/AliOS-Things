#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/builtin.h"
#include "py/mperrno.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "ulog/ulog.h"

extern const mp_obj_type_t mqtt_client_type;

// this is the actual C-structure for our new object

STATIC const mp_rom_map_elem_t mqtt_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_http) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_client), MP_ROM_PTR(&mqtt_client_type) },

};

STATIC MP_DEFINE_CONST_DICT(mqtt_locals_dict, mqtt_locals_dict_table);

const mp_obj_module_t mqtt_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mqtt_locals_dict,
};
