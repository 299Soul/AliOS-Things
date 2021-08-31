/*
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
 */

#include <aos/gpioc_csi.h>

static void gpioc_csi_unregister(aos_gpioc_t *gpioc)
{
    aos_gpioc_csi_t *gpioc_csi;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);
    (void)csi_gpio_detach_callback(&gpioc_csi->csi_gpio);
    (void)csi_gpio_uninit(&gpioc_csi->csi_gpio);
}

static aos_status_t set_dir(aos_gpioc_csi_t *gpioc_csi, uint32_t pin, uint32_t dir)
{
    uint32_t mask = (uint32_t)1 << pin;
    csi_error_t r;

    if (dir == AOS_GPIO_DIR_NONE)
        r = CSI_OK;
    else if (dir == AOS_GPIO_DIR_INPUT)
        r = csi_gpio_dir(&gpioc_csi->csi_gpio, mask, GPIO_DIRECTION_INPUT);
    else if (dir == AOS_GPIO_DIR_OUTPUT)
        r = csi_gpio_dir(&gpioc_csi->csi_gpio, mask, GPIO_DIRECTION_OUTPUT);
    else
        r = CSI_ERROR;

    return (r == CSI_OK) ? 0 : -EIO;
}

static aos_status_t set_input_cfg(aos_gpioc_csi_t *gpioc_csi, uint32_t pin, uint32_t cfg)
{
    uint32_t mask = (uint32_t)1 << pin;
    csi_error_t r;

    if (cfg == AOS_GPIO_INPUT_CFG_HI)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_PULLNONE);
    else if (cfg == AOS_GPIO_INPUT_CFG_PU)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_PULLUP);
    else if (cfg == AOS_GPIO_INPUT_CFG_PD)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_PULLDOWN);
    else
        r = CSI_ERROR;

    return (r == CSI_OK) ? 0 : -EIO;
}

static aos_status_t set_irq_trig(aos_gpioc_csi_t *gpioc_csi, uint32_t pin, uint32_t trig)
{
    csi_gpio_t *csi_gpio = &gpioc_csi->csi_gpio;
    uint32_t mask = (uint32_t)1 << pin;
    csi_error_t r;

    if (trig == AOS_GPIO_IRQ_TRIG_NONE)
        r = CSI_OK;
    else if (trig == AOS_GPIO_IRQ_TRIG_EDGE_RISING)
        r = csi_gpio_irq_mode(csi_gpio, mask, GPIO_IRQ_MODE_RISING_EDGE);
    else if (trig == AOS_GPIO_IRQ_TRIG_EDGE_FALLING)
        r = csi_gpio_irq_mode(csi_gpio, mask, GPIO_IRQ_MODE_FALLING_EDGE);
    else if (trig == AOS_GPIO_IRQ_TRIG_EDGE_BOTH)
        r = csi_gpio_irq_mode(csi_gpio, mask, GPIO_IRQ_MODE_BOTH_EDGE);
    else if (trig == AOS_GPIO_IRQ_TRIG_LEVEL_HIGH)
        r = csi_gpio_irq_mode(csi_gpio, mask, GPIO_IRQ_MODE_HIGH_LEVEL);
    else if (trig == AOS_GPIO_IRQ_TRIG_LEVEL_LOW)
        r = csi_gpio_irq_mode(csi_gpio, mask, GPIO_IRQ_MODE_LOW_LEVEL);
    else
        r = CSI_ERROR;

    return (r == CSI_OK) ? 0 : -EIO;
}

static aos_status_t set_output_cfg(aos_gpioc_csi_t *gpioc_csi, uint32_t pin, uint32_t cfg)
{
    uint32_t mask = (uint32_t)1 << pin;
    csi_error_t r;

    if (cfg == AOS_GPIO_OUTPUT_CFG_PP)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_PUSH_PULL);
    else if (cfg == AOS_GPIO_OUTPUT_CFG_ODNP)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_OPEN_DRAIN);
    else if (cfg == AOS_GPIO_OUTPUT_CFG_ODPU)
        r = csi_gpio_mode(&gpioc_csi->csi_gpio, mask, GPIO_MODE_OPEN_DRAIN);
    else
        r = CSI_ERROR;

    return (r == CSI_OK) ? 0 : -EIO;
}

static void restore_mode(aos_gpioc_csi_t *gpioc_csi, uint32_t pin)
{
    uint32_t mode = gpioc_csi->modes[pin];
    uint32_t dir = mode & AOS_GPIO_DIR_MASK;

    if (dir == AOS_GPIO_DIR_INPUT) {
        uint32_t cfg = mode & AOS_GPIO_INPUT_CFG_MASK;
        uint32_t trig = mode & AOS_GPIO_IRQ_TRIG_MASK;
        (void)set_dir(gpioc_csi, pin, dir);
        (void)set_input_cfg(gpioc_csi, pin, cfg);
        (void)set_irq_trig(gpioc_csi, pin, trig);
    } else if (dir == AOS_GPIO_DIR_OUTPUT) {
        uint32_t cfg = mode & AOS_GPIO_OUTPUT_CFG_MASK;
        (void)set_dir(gpioc_csi, pin, dir);
        (void)set_output_cfg(gpioc_csi, pin, cfg);
    }
}

static aos_status_t gpioc_csi_set_mode(aos_gpioc_t *gpioc, uint32_t pin)
{
    aos_gpioc_csi_t *gpioc_csi;
    uint32_t mode;
    uint32_t dir;
    aos_status_t ret;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);
    mode = gpioc->pins[pin].mode;
    dir = mode & AOS_GPIO_DIR_MASK;

    if (dir == AOS_GPIO_DIR_INPUT) {
        uint32_t cfg = mode & AOS_GPIO_INPUT_CFG_MASK;
        uint32_t trig = mode & AOS_GPIO_IRQ_TRIG_MASK;

        if (cfg == AOS_GPIO_INPUT_CFG_DEFAULT) {
            cfg = gpioc_csi->default_input_cfg;
            mode &= ~AOS_GPIO_INPUT_CFG_MASK;
            mode |= cfg;
        }

        ret = set_dir(gpioc_csi, pin, dir);
        if (ret) {
            restore_mode(gpioc_csi, pin);
            return ret;
        }

        ret = set_input_cfg(gpioc_csi, pin, cfg);
        if (ret) {
            restore_mode(gpioc_csi, pin);
            return ret;
        }

        ret = set_irq_trig(gpioc_csi, pin, trig);
        if (ret) {
            restore_mode(gpioc_csi, pin);
            return ret;
        }
    } else if (dir == AOS_GPIO_DIR_OUTPUT) {
        uint32_t cfg = mode & AOS_GPIO_OUTPUT_CFG_MASK;
        uint32_t mask = (uint32_t)1 << pin;
        csi_gpio_pin_state_t val;

        if (cfg == AOS_GPIO_OUTPUT_CFG_DEFAULT) {
            cfg = gpioc_csi->default_output_cfg;
            mode &= ~AOS_GPIO_OUTPUT_CFG_MASK;
            mode |= cfg;
        }

        ret = set_dir(gpioc_csi, pin, dir);
        if (ret) {
            restore_mode(gpioc_csi, pin);
            return ret;
        }

        ret = set_output_cfg(gpioc_csi, pin, cfg);
        if (ret) {
            restore_mode(gpioc_csi, pin);
            return ret;
        }

        val = gpioc->pins[pin].value ? GPIO_PIN_HIGH : GPIO_PIN_LOW;
        csi_gpio_write(&gpioc_csi->csi_gpio, mask, val);
    }

    gpioc_csi->modes[pin] = mode;

    return 0;
}

static void gpioc_csi_enable_irq(aos_gpioc_t *gpioc, uint32_t pin)
{
    aos_gpioc_csi_t *gpioc_csi;
    uint32_t mask = (uint32_t)1 << pin;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);
    (void)csi_gpio_irq_enable(&gpioc_csi->csi_gpio, mask, true);
}

static void gpioc_csi_disable_irq(aos_gpioc_t *gpioc, uint32_t pin)
{
    aos_gpioc_csi_t *gpioc_csi;
    uint32_t mask = (uint32_t)1 << pin;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);
    (void)csi_gpio_irq_enable(&gpioc_csi->csi_gpio, mask, false);
}

static int gpioc_csi_get_value(aos_gpioc_t *gpioc, uint32_t pin)
{
    aos_gpioc_csi_t *gpioc_csi;
    uint32_t mask = (uint32_t)1 << pin;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);

    return !!(csi_gpio_read(&gpioc_csi->csi_gpio, mask) & mask);
}

static void gpioc_csi_set_value(aos_gpioc_t *gpioc, uint32_t pin)
{
    aos_gpioc_csi_t *gpioc_csi;
    uint32_t mask = (uint32_t)1 << pin;
    csi_gpio_pin_state_t val;

    gpioc_csi = aos_container_of(gpioc, aos_gpioc_csi_t, gpioc);
    val = gpioc->pins[pin].value ? GPIO_PIN_HIGH : GPIO_PIN_LOW;
    csi_gpio_write(&gpioc_csi->csi_gpio, mask, val);
}

static const aos_gpioc_ops_t gpioc_csi_ops = {
    .unregister         = gpioc_csi_unregister,
    .set_mode           = gpioc_csi_set_mode,
    .enable_irq         = gpioc_csi_enable_irq,
    .disable_irq        = gpioc_csi_disable_irq,
    .get_value          = gpioc_csi_get_value,
    .set_value          = gpioc_csi_set_value,
};

#define get_polarity(gpio, pin) (!!(csi_gpio_read(gpio, (uint32_t)1 << (pin)) & ((uint32_t)1 << (pin))))

static void irq_handler(csi_gpio_t *csi_gpio, uint32_t pin_mask, void *arg)
{
    aos_gpioc_csi_t *gpioc_csi;
    aos_gpioc_t *gpioc;

    gpioc_csi = aos_container_of(csi_gpio, aos_gpioc_csi_t, csi_gpio);
    gpioc = &gpioc_csi->gpioc;

    for (uint32_t i = 0; i < gpioc->num_pins; i++) {
        if (pin_mask & ((uint32_t)1 << i))
            aos_gpioc_hard_irq_handler(gpioc, i, get_polarity(csi_gpio, i));
    }
}

aos_status_t aos_gpioc_csi_register(aos_gpioc_csi_t *gpioc_csi)
{
    aos_gpioc_t *gpioc;
    csi_gpio_t *csi_gpio;
    aos_status_t ret;

    if (!gpioc_csi)
        return -EINVAL;

    gpioc = &gpioc_csi->gpioc;

    if (gpioc->num_pins > AOS_GPIOC_CSI_MAX_NUM_PINS)
        return -EINVAL;

    gpioc->ops = &gpioc_csi_ops;

    for (uint32_t i = 0; i < gpioc->num_pins; i++)
        gpioc_csi->modes[i] = AOS_GPIO_DIR_NONE;

    csi_gpio = &gpioc_csi->csi_gpio;

    if (csi_gpio_init(csi_gpio, gpioc->dev.id) != CSI_OK)
        return -EIO;

    if (csi_gpio_attach_callback(csi_gpio, irq_handler, NULL) != CSI_OK) {
        (void)csi_gpio_uninit(csi_gpio);
        return -EIO;
    }

    ret = aos_gpioc_register(gpioc);
    if (ret) {
        (void)csi_gpio_detach_callback(csi_gpio);
        (void)csi_gpio_uninit(csi_gpio);
        return ret;
    }

    return 0;
}

aos_status_t aos_gpioc_csi_unregister(uint32_t id)
{
    return aos_gpioc_unregister(id);
}
