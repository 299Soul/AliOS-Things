// Automatically generated by makemoduledefs.py.

#if (MICROPY_PY_ARRAY)
extern const struct _mp_obj_module_t mp_module_uarray;
#define MODULE_DEF_MP_QSTR_ARRAY \
    { MP_ROM_QSTR(MP_QSTR_uarray), MP_ROM_PTR(&mp_module_uarray) },
#else
#define MODULE_DEF_MP_QSTR_ARRAY
#endif

#if (MICROPY_PY_MACHINE)
extern const struct _mp_obj_module_t mp_module_machine;
#define MODULE_DEF_MP_QSTR_MACHINE \
    { MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&mp_module_machine) },
#else
#define MODULE_DEF_MP_QSTR_MACHINE
#endif

#if PY_BUILD_BLE
extern const struct _mp_obj_module_t mp_module_ble;
#define MODULE_DEF_MP_QSTR_BLE \
    { MP_ROM_QSTR(MP_QSTR_ble), MP_ROM_PTR(&mp_module_ble) },

extern const struct _mp_obj_module_t mp_module_blecfg;
#define MODULE_DEF_MP_QSTR_BLECFG \
    { MP_ROM_QSTR(MP_QSTR_blecfg), MP_ROM_PTR(&mp_module_blecfg) },
#else
#define MODULE_DEF_MP_QSTR_BLE
#define MODULE_DEF_MP_QSTR_BLECFG
#endif

#define MICROPY_REGISTERED_MODULES \
    MODULE_DEF_MP_QSTR_ARRAY       \
    MODULE_DEF_MP_QSTR_MACHINE     \
    MODULE_DEF_MP_QSTR_BLE         \
    MODULE_DEF_MP_QSTR_BLECFG
