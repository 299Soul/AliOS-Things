#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aos_hal_uart.h"
#include "board_mgr.h"
#include "py/builtin.h"
#include "py/mperrno.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "ulog/ulog.h"

#define LOG_TAG "DRIVER_UART"

extern const mp_obj_type_t driver_uart_type;

#define UART_TIMEOUT (0xFFFFFF)
// this is the actual C-structure for our new object
typedef struct {
    // base represents some basic information, like type
    mp_obj_base_t Base;
    // a member created by us
    char *ModuleName;
    item_handle_t uart_handle;
} mp_uart_obj_t;

void uart_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    LOGD(LOG_TAG, "entern %s;\n", __func__);
    mp_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ModuleName(%s)", self->ModuleName);
}

STATIC mp_obj_t uart_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s;\n", __func__);
    mp_uart_obj_t *driver_obj = m_new_obj(mp_uart_obj_t);
    if (!driver_obj) {
        mp_raise_OSError(MP_EINVAL);
    }

    driver_obj->Base.type = &driver_uart_type;
    driver_obj->ModuleName = "uart";
    driver_obj->uart_handle.handle = NULL;

    return MP_OBJ_FROM_PTR(driver_obj);
}

STATIC mp_obj_t obj_open(size_t n_args, const mp_obj_t *args)
{
    mp_int_t ret = -1;
    if (n_args < 2) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }

    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    const char *id = mp_obj_str_get_str(args[1]);
    if (id == NULL) {
        LOGE(LOG_TAG, "%s:illegal par id =%s;\n", __func__, id);
        return mp_const_none;
    }

    ret = py_board_mgr_init();
    if (ret != 0) {
        LOGE(LOG_TAG, "%s:py_board_mgr_init failed\n", __func__);
        return mp_const_none;
    }

    ret = py_board_attach_item(MODULE_UART, id, &(driver_obj->uart_handle));
    if (ret != 0) {
        LOGE(LOG_TAG, "%s: py_board_attach_item failed ret = %d;\n", __func__, ret);
        goto out;
    }

    uart_dev_t *uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        goto out;
    }

    LOGD(LOG_TAG, "%s: port = %d;\n", __func__, uart_device->port);
    LOGD(LOG_TAG, "%s: baud_rate = %d;\n", __func__, uart_device->config.baud_rate);
    LOGD(LOG_TAG, "%s: data_width = %d;\n", __func__, uart_device->config.data_width);
    LOGD(LOG_TAG, "%s: parity = %d;\n", __func__, uart_device->config.parity);
    LOGD(LOG_TAG, "%s: stop_bits = %d;\n", __func__, uart_device->config.stop_bits);
    LOGD(LOG_TAG, "%s: flow_control = %d;\n", __func__, uart_device->config.flow_control);
    LOGD(LOG_TAG, "%s: mode = %d;\n", __func__, uart_device->config.mode);

    ret = aos_hal_uart_init(uart_device);

out:
    if (0 != ret) {
        LOGE(LOG_TAG, "%s: aos_hal_uart_init failed ret = %d;\n", __func__, ret);
        py_board_disattach_item(MODULE_UART, &(driver_obj->uart_handle));
    }

    return MP_ROM_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_open, 2, obj_open);

STATIC mp_obj_t obj_close(size_t n_args, const mp_obj_t *args)
{
    mp_int_t ret = -1;
    if (n_args < 1) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }

    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    uart_dev_t *uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        return mp_const_none;
    }

    ret = aos_hal_uart_finalize(uart_device);
    if (ret != 0) {
        LOGE(LOG_TAG, "aos_hal_uart_finalize failed\n");
    }

    py_board_disattach_item(MODULE_UART, &(driver_obj->uart_handle));

    return MP_ROM_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_close, 1, obj_close);

STATIC mp_obj_t obj_read(size_t n_args, const mp_obj_t *args)
{
    uint32_t recvsize = 0;
    if (n_args != 2) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }

    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    uart_dev_t *uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        return mp_const_none;
    }

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);

    mp_int_t ret = aos_hal_uart_recv_II(uart_device, bufinfo.buf, bufinfo.len, &recvsize, 0);
    if (ret != 0) {
        LOGE(LOG_TAG, "%s:aos_hal_uart_recv_II failed\n", __func__);
        return MP_ROM_INT(ret);
    } else {
        return MP_ROM_INT(recvsize);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_read, 2, obj_read);

STATIC mp_obj_t obj_write(size_t n_args, const mp_obj_t *args)
{
    mp_int_t ret = -1;
    if (n_args != 2) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }

    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    uart_dev_t *uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        return mp_const_none;
    }

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);

    ret = aos_hal_uart_send(uart_device, bufinfo.buf, bufinfo.len, UART_TIMEOUT);
    if (ret != 0) {
        LOGE(LOG_TAG, "aos_hal_uart_send failed\n");
        return MP_ROM_INT(ret);
    } else {
        return MP_ROM_INT(bufinfo.len);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_write, 2, obj_write);

STATIC mp_obj_t obj_setBaudRate(size_t n_args, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s; n_args = %d;\n", __func__, n_args);
    int ret = -1;
    uart_dev_t *uart_device = NULL;
    if (n_args < 2) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }
    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        return mp_const_none;
    }

    uint32_t baud_rate = (uint32_t)mp_obj_get_int(args[1]);
    LOGD(LOG_TAG, "%s: set baud_rate = %d;\n", __func__, baud_rate);
    ret = aos_hal_uart_finalize(uart_device);
    if (ret != 0) {
        LOGE(LOG_TAG, "setBaudRate hal_uart_finalize failed\n");
    }
    uart_device->config.baud_rate = baud_rate;
    LOGD(LOG_TAG, "%s: port = %d;\n", __func__, uart_device->port);
    LOGD(LOG_TAG, "%s: baud_rate = %d;\n", __func__, uart_device->config.baud_rate);
    LOGD(LOG_TAG, "%s: data_width = %d;\n", __func__, uart_device->config.data_width);
    LOGD(LOG_TAG, "%s: parity = %d;\n", __func__, uart_device->config.parity);
    LOGD(LOG_TAG, "%s: stop_bits = %d;\n", __func__, uart_device->config.stop_bits);
    LOGD(LOG_TAG, "%s: flow_control = %d;\n", __func__, uart_device->config.flow_control);
    LOGD(LOG_TAG, "%s: mode = %d;\n", __func__, uart_device->config.mode);
    ret = aos_hal_uart_init(uart_device);
    if (ret == -1) {
        LOGE(LOG_TAG, "setBaudRate hal_uart_init failed\n");
    }
    LOGD(LOG_TAG, "%s:out\n", __func__);

    return MP_ROM_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_setBaudRate, 2, obj_setBaudRate);

STATIC mp_obj_t obj_on(size_t n_args, const mp_obj_t *args)
{
    LOGD(LOG_TAG, "entern  %s; n_args = %d;\n", __func__, n_args);
    int ret = -1;
    uart_dev_t *uart_device = NULL;
    if (n_args < 1) {
        LOGE(LOG_TAG, "%s: args num is illegal :n_args = %d;\n", __func__, n_args);
        return mp_const_none;
    }
    mp_obj_base_t *self = (mp_obj_base_t *)MP_OBJ_TO_PTR(args[0]);
    mp_uart_obj_t *driver_obj = (mp_uart_obj_t *)self;
    if (driver_obj == NULL) {
        LOGE(LOG_TAG, "driver_obj is NULL\n");
        return mp_const_none;
    }

    uart_device = py_board_get_node_by_handle(MODULE_UART, &(driver_obj->uart_handle));
    if (NULL == uart_device) {
        LOGE(LOG_TAG, "%s: py_board_get_node_by_handle failed;\n", __func__);
        return mp_const_none;
    }

    // ret = uart_add_recv(uart_device, driver_obj->uart_handle, NULL); //TODO
    if (ret < 0) {
        LOGE(LOG_TAG, "uart_add_recv failed\n");
    }
    LOGD(LOG_TAG, "%s:out\n", __func__);

    return MP_ROM_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(uart_obj_on, 1, obj_on);

STATIC const mp_rom_map_elem_t uart_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_UART) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&uart_obj_open) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&uart_obj_close) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&uart_obj_read) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&uart_obj_write) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&uart_obj_on) },
};

STATIC MP_DEFINE_CONST_DICT(uart_locals_dict, uart_locals_dict_table);

const mp_obj_type_t driver_uart_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = uart_obj_print,
    .make_new = uart_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&uart_locals_dict,
};
