/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#define CONFIG_LOGMACRO_DETAILS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "amp_config.h"
#include "amp_system.h"
#include "amp_fs.h"
#include "amp_defines.h"
#include "board_mgr.h"
#include "board_marker.h"
#include "wrappers_defs.h"
#include "infra_list.h"

#include "cJSON.h"

#define MOD_STR "BOARD_MGR"
#define DRIVER_DIR JSE_FS_ROOT_DIR "/drivers/"

#define DRIVER_NAME "driver.json"

extern int g_repl_config;

typedef struct parse_json
{
    char *marker_name;
    addon_module_m module;
    int8_t (*fn)(cJSON *, char *);
} parse_json_t;

typedef struct board_item
{
    addon_module_m module;
    item_handle_t handle;
    char *name_id;
    void *node;
    uint8_t status;
} board_item_t;

typedef struct board_mgr
{
    uint32_t item_size;
    board_item_t **item;
} board_mgr_t;

typedef struct page_entry
{
    char *page;
    dlist_t node;
}page_entry_t;

static board_mgr_t g_board_mgr = {0, NULL};

static dlist_t g_pages_list;

static int8_t board_add_new_item(addon_module_m module, char *name_id,
                                 void *node);

static board_mgr_t *board_get_handle(void)
{
    return &g_board_mgr;
}

#ifdef JSE_HW_ADDON_GPIO
#include "amp_hal_gpio.h"
static void board_set_gpio_default(gpio_dev_t *gpio_device)
{
    gpio_params_t *priv = (gpio_params_t *)gpio_device->priv;
    if (NULL == gpio_device || NULL == priv)
    {
        return;
    }

    gpio_device->port = -1;
    gpio_device->config = OUTPUT_PUSH_PULL;
    priv->irq_mode = 0;
    priv->js_cb_ref = 0;
    priv->reserved = NULL;
}

static int8_t board_parse_gpio(cJSON *gpio, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *dir = NULL;
    cJSON *pull = NULL;
    cJSON *intMode = NULL;
    gpio_dev_t device;
    gpio_params_t *priv = NULL;
    gpio_config_t *config = (gpio_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        if ((priv = (gpio_params_t *)amp_calloc(1, sizeof(gpio_params_t))) == NULL) {
            amp_error(MOD_STR, "malloc failed");
            return (-1);
        }
        item = gpio;
        index += 1;
        if (NULL == item || NULL == id)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }

        device.priv = priv;
        board_set_gpio_default(&device);
        dir = cJSON_GetObjectItem(item, GPIO_DIR);
        pull = cJSON_GetObjectItem(item, GPIO_PULL);
        intMode = cJSON_GetObjectItem(item, GPIO_INTMODE);
        if (NULL != dir && cJSON_String == dir->type && NULL != pull &&
            cJSON_String == pull->type)
        {
            if  (strcmp(dir->valuestring, GPIO_DIR_INPUT) == 0) {
                if (strcmp(GPIO_PULL_DOWN, pull->valuestring) == 0)
                    *config = INPUT_PULL_DOWN;
                else if (strcmp(GPIO_PULL_UP, pull->valuestring) == 0)
                    *config = INPUT_PULL_UP;
                else if (strcmp(GPIO_PULL_OPEN, pull->valuestring) == 0)
                    *config = INPUT_HIGH_IMPEDANCE;
            }

            else if  (strcmp(dir->valuestring, GPIO_DIR_OUTPUT) == 0) {
                if (strcmp(GPIO_PULL_DOWN, pull->valuestring) == 0)
                    *config = OUTPUT_PUSH_PULL;
                else if (strcmp(GPIO_PULL_UP, pull->valuestring) == 0)
                    *config = OUTPUT_OPEN_DRAIN_PULL_UP;
                else if (strcmp(GPIO_PULL_OPEN, pull->valuestring) == 0)
                    *config = OUTPUT_OPEN_DRAIN_NO_PULL;
            }

            else if  (strcmp(dir->valuestring, GPIO_DIR_IRQ) == 0) {
                *config = IRQ_MODE;
                if (strcmp(GPIO_INT_RISING, intMode->valuestring) == 0)
                    priv->irq_mode = IRQ_TRIGGER_RISING_EDGE;
                else if (strcmp(GPIO_INT_FALLING, intMode->valuestring) == 0)
                    priv->irq_mode = IRQ_TRIGGER_FALLING_EDGE;
                else if (strcmp(GPIO_INT_BOTH, intMode->valuestring) == 0)
                    priv->irq_mode = IRQ_TRIGGER_BOTH_EDGES;
            }

            else if  (strcmp(dir->valuestring, GPIO_DIR_ANALOG) == 0) {
                *config = ANALOG_MODE;
            }
        }
        gpio_dev_t *new_gpio = amp_calloc(1, sizeof(*new_gpio));
        if (NULL == new_gpio)
        {
            continue;
        }
        device.port = port->valueint;
        char *gpio_id = strdup(id);
        memcpy(new_gpio, &device, sizeof(gpio_dev_t));
        ret = board_add_new_item(MODULE_GPIO, gpio_id, new_gpio);
        if (0 == ret)
        {
            continue;
        }
        if (NULL != gpio_id)
        {
            amp_free(gpio_id);
            gpio_id = NULL;
        }
        if (NULL != new_gpio)
        {
            amp_free(new_gpio);
            new_gpio = NULL;
            amp_free(priv);
            priv = NULL;
        }
    }
    return (0);
}
#endif

#ifdef JSE_HW_ADDON_UART
#include "amp_hal_uart.h"
static void board_set_uart_default(uart_dev_t *uart_device)
{
    if (NULL == uart_device)
    {
        return;
    }
    uart_device->port = 0;
    uart_device->priv = NULL;
    uart_device->config.baud_rate = 115200;
    uart_device->config.data_width = DATA_WIDTH_8BIT;
    uart_device->config.flow_control = FLOW_CONTROL_DISABLED;
    uart_device->config.parity = NO_PARITY;
    uart_device->config.stop_bits = STOP_BITS_1;
    uart_device->config.mode = MODE_TX_RX;
}

static int8_t board_parse_uart(cJSON *uart, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    uart_dev_t device;
    uart_config_t *config = (uart_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = uart;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_uart_default(&device);
        temp = cJSON_GetObjectItem(item, UART_DATA_WIDTH);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            int32_t width = temp->valueint;
            switch (width)
            {
            case 5:
                config->data_width = DATA_WIDTH_5BIT;
                break;
            case 6:
                config->data_width = DATA_WIDTH_6BIT;
                break;
            case 7:
                config->data_width = DATA_WIDTH_7BIT;
                break;
            case 8:
                config->data_width = DATA_WIDTH_8BIT;
                break;

            default:
                break;
            }
        }
        temp = cJSON_GetObjectItem(item, UART_BAUD_RATE);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            config->baud_rate = temp->valueint;
        }
        temp = cJSON_GetObjectItem(item, UART_STOP_BITS);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            int32_t stopbits = temp->valueint;
            switch (stopbits)
            {
            case 1:
                config->stop_bits = STOP_BITS_1;
                break;
            case 2:
                config->stop_bits = STOP_BITS_2;

            default:
                break;
            }
        }
        temp = cJSON_GetObjectItem(item, UART_FLOW_CONTROL);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, UART_FC_DISABLE) == 0) {
                config->flow_control = FLOW_CONTROL_DISABLED;
            }

            else if (strcmp(temp->valuestring, UART_FC_CTS) == 0) {
                config->flow_control = FLOW_CONTROL_CTS;
            }

            else if (strcmp(temp->valuestring, UART_FC_RTS) == 0) {
                config->flow_control = FLOW_CONTROL_RTS;
            }

            else if (strcmp(temp->valuestring, UART_FC_RTSCTS) == 0) {
                config->flow_control = FLOW_CONTROL_CTS_RTS;
            }
        }
        temp = cJSON_GetObjectItem(item, UART_PARITY_CONFIG);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, UART_PARITY_NONE) == 0) {
                config->parity = NO_PARITY;
            }

            else if (strcmp(temp->valuestring, UART_PARITY_ODD) == 0) {
                config->parity = ODD_PARITY;
            }

            else if (strcmp(temp->valuestring, UART_PARITY_EVEN) == 0) {
                config->parity = EVEN_PARITY;
            }
        }
        uart_dev_t *new_uart = amp_calloc(1, sizeof(*new_uart));
        if (NULL == new_uart)
        {
            continue;
        }
        device.port = port->valueint;
        char *uart_id = strdup(id);
        *new_uart = device;
        ret = board_add_new_item(MODULE_UART, uart_id, new_uart);
        amp_debug(MOD_STR, "*** add item: %s", uart_id);
        if (0 == ret)
        {
            continue;
        }
        if (NULL != uart_id)
        {
            amp_free(uart_id);
            uart_id = NULL;
        }
        if (NULL != new_uart)
        {
            amp_free(new_uart);
            new_uart = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_I2C
#include "amp_hal_i2c.h"
static void board_set_i2c_default(i2c_dev_t *i2c_device)
{
    if (NULL == i2c_device)
    {
        return;
    }
    i2c_device->port = 0;
    i2c_device->priv = NULL;
    i2c_device->config.address_width = 7;
    i2c_device->config.freq = 100000;
    i2c_device->config.mode = I2C_MODE_MASTER;
    i2c_device->config.dev_addr = 0xFF;
}

static int8_t board_parse_i2c(cJSON *i2c, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    i2c_dev_t device;
    i2c_config_t *config = (i2c_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = i2c;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_i2c_default(&device);

        temp = cJSON_GetObjectItem(item, I2C_ADDR_WIDTH);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            switch (temp->valueint)
            {
            case 7:
                config->address_width = I2C_HAL_ADDRESS_WIDTH_7BIT;
                break;
            case 10:
                config->address_width = I2C_HAL_ADDRESS_WIDTH_10BIT;
                break;

            default:
                break;
            }
        }
        temp = cJSON_GetObjectItem(item, I2C_FREQ);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            config->freq = temp->valueint;
        }
        temp = cJSON_GetObjectItem(item, I2C_MODE);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, I2C_MASTER) == 0) {
                config->mode = I2C_MODE_MASTER;
            }

            else if (strcmp(temp->valuestring, I2C_SLAVE) == 0) {
                config->mode = I2C_MODE_SLAVE;
            }
        }
        temp = cJSON_GetObjectItem(item, I2C_ADDR_DEV);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            config->dev_addr = temp->valueint;
        }

        i2c_dev_t *new_i2c = amp_calloc(1, sizeof(*new_i2c));
        if (NULL == new_i2c)
        {
            continue;
        }
        device.port = port->valueint;
        char *i2c_id = strdup(id);
        *new_i2c = device;
        ret = board_add_new_item(MODULE_I2C, i2c_id, new_i2c);
        if (0 == ret)
        {
            continue;
        }
        if (NULL != i2c_id)
        {
            amp_free(i2c_id);
            i2c_id = NULL;
        }
        if (NULL != new_i2c)
        {
            amp_free(new_i2c);
            new_i2c = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_SPI
#include "amp_hal_spi.h"
static void board_set_spi_default(spi_dev_t *spi_device)
{
    if (NULL == spi_device)
    {
        return;
    }
    spi_device->port = 0;
    spi_device->priv = NULL;
    spi_device->config.mode = HAL_SPI_MODE_MASTER;
    spi_device->config.freq = 325 * 10 * 1000; // 3.25M
}

static int8_t board_parse_spi(cJSON *spi, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    spi_dev_t device;
    spi_config_t *config = (spi_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = spi;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_spi_default(&device);

        temp = cJSON_GetObjectItem(item, SPI_MODE);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, SPI_MODE_MASTER) == 0) {
                config->mode = HAL_SPI_MODE_MASTER;
            }

            else if (strcmp(temp->valuestring, SPI_MODE_SLAVE) == 0) {
                config->mode = HAL_SPI_MODE_SLAVE;
            }
        }
        temp = cJSON_GetObjectItem(item, SPI_FREQ);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            config->freq = temp->valueint;
        }

        spi_dev_t *new_spi = amp_calloc(1, sizeof(*new_spi));
        if (NULL == new_spi)
        {
            continue;
        }
        device.port = port->valueint;
        char *spi_id = strdup(id);
        *new_spi = device;
        ret = board_add_new_item(MODULE_SPI, spi_id, new_spi);
        if (0 == ret)
        {
            continue;
        }
        if (NULL != spi_id)
        {
            amp_free(spi_id);
            spi_id = NULL;
        }
        if (NULL != new_spi)
        {
            amp_free(new_spi);
            new_spi = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_CAN
#include "amp_hal_can.h"
static void board_set_can_default(can_dev_t *can_device)
{
    if (NULL == can_device)
    {
        return;
    }
    can_device->port = 0;
    can_device->priv = NULL;
    can_device->config.baud_rate = CAN_BAUD_500K;
    can_device->config.ide = CAN_IDE_NORMAL;
    can_device->config.auto_bus_off = CAN_AUTO_BUS_OFF_ENABLE;
    can_device->config.auto_retry_transmit = CAN_AUTO_RETRY_TRANSMIT_ENABLE;
}

static int8_t board_parse_can(cJSON *can, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    can_dev_t device;
    can_config_t *config = (can_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = can;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_can_default(&device);

        temp = cJSON_GetObjectItem(item, CAN_BAUD_RATE);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            switch (temp->valueint)
            {
            case 1000000:
                config->baud_rate = CAN_BAUD_1M;
                break;
            case 500000:
                config->baud_rate = CAN_BAUD_500K;
                break;
            case 250000:
                config->baud_rate = CAN_BAUD_250K;
                break;
            case 125000:
                config->baud_rate = CAN_BAUD_125K;
                break;
            default:
                break;
            }
        }
        temp = cJSON_GetObjectItem(item, CAN_IDE);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, IDE_NORMAL_CAN) == 0) {
                config->ide = CAN_IDE_NORMAL;
            }

            else if (strcmp(temp->valuestring, IDE_EXTEND_CAN) == 0) {
                config->ide = CAN_IDE_EXTEND;
            }
        }

        temp = cJSON_GetObjectItem(item, CAN_AUTO_BUS_OFF);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, CAN_DISABLE) == 0) {
                config->auto_bus_off = CAN_AUTO_BUS_OFF_DISABLE;
            }

            else if (strcmp(temp->valuestring, CAN_ENABLE) == 0) {
                config->auto_bus_off = CAN_AUTO_BUS_OFF_ENABLE;
            }
        }

        temp = cJSON_GetObjectItem(item, CAN_RETRY_TRANSMIT);
        if (NULL != temp && cJSON_String == temp->type)
        {
            if (strcmp(temp->valuestring, CAN_DISABLE) == 0) {
                config->auto_retry_transmit = CAN_AUTO_RETRY_TRANSMIT_DISABLE;
            }

            else if (strcmp(temp->valuestring, CAN_ENABLE) == 0) {
                config->auto_retry_transmit = CAN_AUTO_RETRY_TRANSMIT_ENABLE;
            }
        }

        can_dev_t *new_can = amp_calloc(1, sizeof(*new_can));
        if (NULL == new_can)
        {
            continue;
        }
        device.port = port->valueint;
        char *can_id = strdup(id);
        *new_can = device;
        ret = board_add_new_item(MODULE_CAN, can_id, new_can);
        if (0 == ret)
        {
            continue;
        }
        if (NULL != can_id)
        {
            amp_free(can_id);
            can_id = NULL;
        }
        if (NULL != new_can)
        {
            amp_free(new_can);
            new_can = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_PWM
#include "amp_hal_pwm.h"
static void board_set_pwm_default(pwm_dev_t *pwm_device)
{
    if (NULL == pwm_device)
    {
        return;
    }
    pwm_device->port = 0;
    pwm_device->priv = NULL;
    pwm_device->config.freq = 0;
    pwm_device->config.duty_cycle = 100;
}

static int8_t board_parse_pwm(cJSON *pwm, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    pwm_dev_t device;
    pwm_config_t *config = (pwm_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = pwm;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_pwm_default(&device);
        pwm_dev_t *new_pwm = amp_calloc(1, sizeof(*new_pwm));
        if (NULL == new_pwm)
        {
            continue;
        }
        device.port = port->valueint;
        char *pwm_id = strdup(id);
        *new_pwm = device;
        ret = board_add_new_item(MODULE_PWM, pwm_id, new_pwm);
        if (0 == ret)
        {
            continue;
        }

        if (NULL != pwm_id)
        {
            amp_free(pwm_id);
            pwm_id = NULL;
        }
        if (NULL != new_pwm)
        {
            amp_free(new_pwm);
            new_pwm = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_ADC
#include "amp_hal_adc.h"
static void board_set_adc_default(adc_dev_t *adc_device)
{
    if (NULL == adc_device)
    {
        return;
    }
    adc_device->port = 0;
    adc_device->priv = NULL;
    adc_device->config.sampling_cycle = 12000000;
}

static int8_t board_parse_adc(cJSON *adc, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    adc_dev_t device;
    adc_config_t *config = (adc_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = adc;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_adc_default(&device);

        temp = cJSON_GetObjectItem(item, ADC_SAMPLING);
        if (NULL != temp && cJSON_Number == temp->type)
        {
            config->sampling_cycle = temp->valueint;
        }

        adc_dev_t *new_adc = amp_calloc(1, sizeof(*new_adc));
        if (NULL == new_adc)
        {
            continue;
        }
        device.port = port->valueint;
        char *adc_id = strdup(id);
        *new_adc = device;
        ret = board_add_new_item(MODULE_ADC, adc_id, new_adc);
        if (0 == ret)
        {
            continue;
        }

        if (NULL != adc_id)
        {
            amp_free(adc_id);
            adc_id = NULL;
        }
        if (NULL != new_adc)
        {
            amp_free(new_adc);
            new_adc = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_DAC
#include "amp_hal_dac.h"
static void board_set_dac_default(dac_dev_t *dac_device)
{
    if (NULL == dac_device)
    {
        return;
    }

    dac_device->port = 0;
    dac_device->priv = NULL;
}

static int8_t board_parse_dac(cJSON *dac, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    dac_dev_t device;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = dac;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_dac_default(&device);
        dac_dev_t *new_dac = amp_calloc(1, sizeof(*new_dac));
        if (NULL == new_dac)
        {
            continue;
        }
        device.port = port->valueint;
        char *dac_id = strdup(id);
        *new_dac = device;
        ret = board_add_new_item(MODULE_DAC, dac_id, new_dac);
        if (0 == ret)
        {
            continue;
        }

        if (NULL != dac_id)
        {
            amp_free(dac_id);
            dac_id = NULL;
        }
        if (NULL != new_dac)
        {
            amp_free(new_dac);
            new_dac = NULL;
        }
    }

    return (0);
}
#endif

#ifdef JSE_HW_ADDON_TIMER
#include "amp_hal_timer.h"
static void board_set_timer_default(timer_dev_t *timer_device)
{
    if (NULL == timer_device)
    {
        return;
    }

    timer_device->port = 0;
    timer_device->priv = NULL;
    timer_device->config.reload_mode = TIMER_RELOAD_MANU;
    timer_device->config.period = 1000;

}

static int8_t board_parse_timer(cJSON *timer, char *id)
{
    int index = 0;
    int8_t ret = -1;
    cJSON *port = NULL;
    cJSON *item = NULL;
    cJSON *temp = NULL;
    timer_dev_t device;
    timer_config_t *config = (timer_config_t *)&device.config;
    int8_t size = 1;

    if (size <= 0)
    {
        return (-1);
    }
    while (index < size)
    {
        item = timer;
        index += 1;
        if (NULL == item)
        {
            continue;
        }
        port = cJSON_GetObjectItem(item, MARKER_PORT);
        if (NULL == port || cJSON_Number != port->type)
        {
            continue;
        }
        board_set_timer_default(&device);
        timer_dev_t *new_timer = amp_calloc(1, sizeof(*new_timer));
        if (NULL == new_timer)
        {
            continue;
        }
        device.port = port->valueint;
        char *timer_id = strdup(id);
        *new_timer = device;
        ret = board_add_new_item(MODULE_TIMER, timer_id, new_timer);
        if (0 == ret)
        {
            continue;
        }

        if (NULL != timer_id)
        {
            amp_free(timer_id);
            timer_id = NULL;
        }
        if (NULL != new_timer)
        {
            amp_free(new_timer);
            new_timer = NULL;
        }
    }

    return (0);
}
#endif

static char *board_get_json_buff(const char *json_path)
{
    void *json_data = NULL;
    uint32_t len = 0;
    int32_t curpos = -1;
    void *json_fd = NULL;

    if (NULL == json_path)
    {
        return (NULL);
    }

    json_fd = HAL_Fopen(json_path, "r");
    if (json_fd == NULL) {
        amp_debug(MOD_STR, "fopen fail");
        return (NULL);
    }
    // amp_debug(MOD_STR, "jse_lseek");
    HAL_Fseek(json_fd, 0, HAL_SEEK_END, &curpos);
    if (curpos < 0) {
        len = HAL_Ftell(json_fd);
    } else {
        len = curpos;
    }
    // amp_debug(MOD_STR, "jse_lseek, len: %d", len);
    json_data = amp_calloc(1, sizeof(char) * (len + 1));
    if (NULL == json_data)
    {
        HAL_Fclose(json_fd);
        amp_debug(MOD_STR, "jse_close");
        return (NULL);
    }
    HAL_Fseek(json_fd, 0, SEEK_SET, &curpos);
    // amp_debug(MOD_STR, "jse_read");
    HAL_Fread(json_data, 1, len, json_fd);
    // amp_debug(MOD_STR, "jse_read, data: %s", json_data);
    HAL_Fclose(json_fd);
    return json_data;
}

static parse_json_t g_parse_json[] = {
#ifdef JSE_HW_ADDON_UART
    {MARKER_UART, MODULE_UART, board_parse_uart},
#endif

#ifdef JSE_HW_ADDON_GPIO
    {MARKER_GPIO, MODULE_GPIO, board_parse_gpio},
#endif

#ifdef JSE_HW_ADDON_PWM
    {MARKER_PWM, MODULE_PWM, board_parse_pwm},
#endif

#ifdef JSE_HW_ADDON_I2C
    {MARKER_I2C, MODULE_I2C, board_parse_i2c},
#endif

#ifdef JSE_HW_ADDON_SPI
    {MARKER_SPI, MODULE_SPI, board_parse_spi},
#endif

#ifdef JSE_HW_ADDON_CAN
    {MARKER_CAN, MODULE_CAN, board_parse_can},
#endif

#ifdef JSE_HW_ADDON_ADC
    {MARKER_ADC, MODULE_ADC, board_parse_adc},
#endif

#ifdef JSE_HW_ADDON_DAC
    {MARKER_DAC, MODULE_DAC, board_parse_dac},
#endif

#ifdef JSE_HW_ADDON_TIMER
    {MARKER_TIMER, MODULE_TIMER, board_parse_timer},
#endif
    {NULL, MODULE_NUMS, NULL},
};

static int32_t board_parse_json_buff(const char *json_buff)
{
    cJSON *root = NULL;
    cJSON *page = NULL, *pages = NULL;
    cJSON *io = NULL;
    cJSON *debug = NULL;
    cJSON *repl = NULL;
    cJSON *item = NULL;
    cJSON *child = NULL;
    parse_json_t *parser_handle = NULL;

    if (NULL == json_buff)
    {
        return -1;
    }
    root = cJSON_Parse(json_buff);
    if (NULL == root)
    {
        return -1;
    }

    /* debugLevel configuration */
    if((debug = cJSON_GetObjectItem(root, APP_CONFIG_DEBUG)) != NULL) {
        /* parsing debugLevel configuration */
        if (!cJSON_IsString(debug)) {
            amp_error(MOD_STR, "debugLevel not string");
            goto parse_end;
        }
        
        amp_debug(MOD_STR, "get debugLevel:%s", debug->valuestring);
        if(strcmp(debug->valuestring, "DEBUG") == 0) {
            aos_set_log_level(LOG_DEBUG);
        }
        else if(strcmp(debug->valuestring, "INFO") == 0) {
            aos_set_log_level(LOG_INFO);
        }
        else if(strcmp(debug->valuestring, "WARN") == 0) {
            aos_set_log_level(LOG_WARNING);

        }
        else if(strcmp(debug->valuestring, "ERROR") == 0) {
            aos_set_log_level(LOG_ERR);

        }
        else if(strcmp(debug->valuestring, "FATAL") == 0) {
            aos_set_log_level(LOG_CRIT);
        }
        else {
            amp_debug(MOD_STR, "debugLevel error, set to default: 'ERROR'");
            aos_set_log_level(LOG_ERR);
        }
    }
    else {
        amp_debug(MOD_STR, "No debugLevel configuration in app.json, set to default: 'ERROR'");
    }

    /* page configuration */
    if((pages = cJSON_GetObjectItem(root, APP_CONFIG_PAGES)) != NULL) {
        /* parsing io configuration */
        if(!cJSON_IsArray(pages)) {
            amp_error(MOD_STR, "Pages entries need array");
            goto parse_end;
        }

        dlist_init(&g_pages_list);
        cJSON_ArrayForEach(page, pages) {
            if (!cJSON_IsString(page)) {
                amp_error(MOD_STR, "page not string");
                goto parse_end;
            }

            amp_debug(MOD_STR, "get page:%s", page->valuestring);

            /* add page to dlink */
            page_entry_t *page_entry = amp_malloc(sizeof(page_entry_t));
            page_entry->page = strdup(page->valuestring); /* don't forget to free */
            dlist_add_tail(&page_entry->node, &g_pages_list);
        }
    }
    else {
        amp_debug(MOD_STR, "No pages configuration in app.json");
    }

    /* repl configuration */
    if((repl = cJSON_GetObjectItem(root, APP_CONFIG_REPL)) != NULL) {
        /* parsing debugLevel configuration */
        if (!cJSON_IsString(repl)) {
            amp_error(MOD_STR, "repl not string");
            goto parse_end;
        }
        
        amp_debug(MOD_STR, "get app repl config is:%s", repl->valuestring);
        if (strcmp(repl->valuestring, "disable") == 0) {
            g_repl_config = 0;
        } else if (strcmp(repl->valuestring, "enable") == 0) {
            g_repl_config = 1;
        } else {
            amp_debug(MOD_STR, "repl configuration is wrong, set to default: 'enable'");
            g_repl_config = 1;
        }
    }
    else {
        amp_debug(MOD_STR, "No repl configuration in app.json, set to default: 'enable'");
    }

    /* net configuration */
    /* TODO */

    /* io configuration */
    if((io = cJSON_GetObjectItem(root, APP_CONFIG_IO)) != NULL) {
        /* parsing io configuration */
        child = io->child;
        while (NULL != child) {
            item = cJSON_GetObjectItem(child, MARKER_ID);
            if (NULL == item || cJSON_String != item->type) {
                child = child->next;
                continue;
            }
            parser_handle = &g_parse_json[0];
            while (NULL != parser_handle->marker_name) {
                if (0 == strcmp(item->valuestring, parser_handle->marker_name)) {
                    parser_handle->fn(child, child->string);
                }
                parser_handle += 1;
            }
            child = child->next;
        }
    }
    else {
        amp_warn(MOD_STR, "No io configuration in app.json");
    }

    cJSON_Delete(root);
    return 0;

parse_end:
    cJSON_Delete(root);
    return -1;
}

static void *board_get_items(addon_module_m module, item_handle_t *handle,
                             const char *name_id)
{
    board_mgr_t *mgr_handle = board_get_handle();
    board_item_t *item = NULL;
    if (NULL == handle && NULL == name_id)
    {
        return (NULL);
    }
    uint32_t i = 0;
    for (i = 0; i < mgr_handle->item_size; ++i)
    {
        item = mgr_handle->item[i];
        if (module != item->module)
        {
            continue;
        }
        if (NULL != handle && item->handle.handle != handle->handle)
        {
            continue;
        }
        if (NULL != name_id && 0 != strcmp(item->name_id, name_id))
        {
            continue;
        }
        return (item);
    }

    return (NULL);
}

static int8_t board_add_new_item(addon_module_m module, char *name_id,
                                 void *node)
{
    board_item_t *item = NULL;
    board_mgr_t *mgr_handle = board_get_handle();
    if (NULL == name_id || NULL == node)
        return (-1);
    if (NULL != board_get_items(module, NULL, name_id))
    {
        return (-1);
    }
    board_item_t *new_item = amp_calloc(1, sizeof(*new_item));
    if (NULL == new_item)
    {
        return (-1);
    }
    void *addr = amp_realloc(
        mgr_handle->item, sizeof(board_item_t *) * (mgr_handle->item_size + 1));
    if (NULL == addr)
    {
        goto out;
    }

    new_item->module = module;
    new_item->name_id = name_id;
    new_item->handle.handle = (void*)new_item;
    new_item->node = node;
    new_item->status = 0;
    mgr_handle->item = addr;
    mgr_handle->item[mgr_handle->item_size] = new_item;
    mgr_handle->item_size += 1;

    return (0);
out:
    if (NULL != new_item)
    {
        amp_free(new_item);
        new_item = NULL;
    }
    return (-1);
}

int8_t board_attach_item(addon_module_m module, const char *name_id,
                         item_handle_t *out)
{
    board_item_t *item = NULL;
    if (NULL == name_id)
    {
        return (-1);
    }
    item = board_get_items(module, NULL, name_id);
    if (NULL == item || 1 == item->status)
    {
        return (-1);
    }
    item->status = 1;
    *out = item->handle;

    return (0);
}

int8_t board_disattach_item(addon_module_m module, item_handle_t *handle)
{
    board_item_t *item = NULL;
    if (NULL == handle)
    {
        return (-1);
    }
    item = board_get_items(module, handle, NULL);
    if (NULL == item)
    {
        return (-1);
    }
    item->status = 0;

    return (0);
}

int8_t board_check_attach_status(addon_module_m module, item_handle_t *handle)
{
    board_item_t *item = NULL;
    if (NULL == handle)
    {
        return (0);
    }
    item = board_get_items(module, handle, NULL);
    if (NULL == item)
    {
        return (0);
    }

    return (item->status);
}

void *board_get_node_by_name(addon_module_m module, const char *name_id)
{
    board_item_t *item = NULL;
    if (NULL == name_id)
    {
        return (NULL);
    }
    item = board_get_items(module, NULL, name_id);
    if (NULL == item || 0 == item->status)
    {
        return (NULL);
    }

    return (item->node);
}

void *board_get_node_by_handle(addon_module_m module, item_handle_t *handle)
{
    board_item_t *item = NULL;
    if (NULL == handle)
        return (NULL);
    item = board_get_items(module, handle, NULL);
    if (NULL == item || 0 == item->status)
    {
        return (NULL);
    }

    return (item->node);
}

int32_t board_mgr_init(const char *json_path)
{
    int32_t ret = -1;
    char *json = NULL;

    memset(&g_board_mgr, 0x00, sizeof(g_board_mgr));
    json = board_get_json_buff(json_path);
    if (NULL == json)
    {
        amp_debug(MOD_STR, "default board config is null");
        return ret;
    }
    // return 0;
    ret = board_parse_json_buff(json);
    amp_free(json);
    json = NULL;

    return ret;
}

int8_t board_load_drivers(const char *driver)
{
    char *p = (char *)driver;
    char *index = NULL;
    char *json = NULL;
    char *new_driver = NULL;
    int32_t len = -1;
    int8_t ret = -1;
    if (NULL == driver)
    {
        return (-1);
    }
    len = strlen(driver);
    if (len < 8)
    {
        return (-1);
    }
    if (0 != strncmp(driver + len - 3, ".js", 3))
    {
        return (-1);
    }
    p = p + strlen("/spiffs/");
    index = (char *)driver + len - 1;
    while (index > p)
    {
        if (*index == '/')
        {
            break;
        }
        index -= 1;
    }
    if (index <= p)
    {
        return (-1);
    }
    new_driver = amp_calloc(1, sizeof(char) * (index - driver + 16));
    if (NULL == new_driver)
    {
        return (-1);
    }
    memmove(new_driver, driver, index - driver + 1);
    memmove(new_driver + (index - driver + 1), DRIVER_NAME,
            strlen(DRIVER_NAME));

    amp_debug(MOD_STR, "%s%d, new_driver = %s ", __FUNCTION__, __LINE__,
              new_driver);

    json = board_get_json_buff(new_driver);
    if (NULL == json)
    {
        goto out;
    }
    ret = board_parse_json_buff(json);
out:
    if (NULL != new_driver)
    {
        amp_free(new_driver);
        new_driver = NULL;
    }
    if (NULL != json)
    {
        amp_free(json);
        json = NULL;
    }
    return (ret);
}
