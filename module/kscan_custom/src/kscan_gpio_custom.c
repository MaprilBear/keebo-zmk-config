/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_kscan_gpio_custom

#include <zephyr/device.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Helper macro
#define PWR_TWO(x) (1 << (x))

// Define row and col cfg
#define _KSCAN_GPIO_CFG_INIT(n, prop, idx) GPIO_DT_SPEC_GET_BY_IDX(n, prop, idx),

// Check debounce config
#define CHECK_DEBOUNCE_CFG(n, a, b) COND_CODE_0(DT_INST_PROP(n, debounce_period), a, b)

// Define the row and column lengths
#define INST_DEMUX_ADDR_GPIOS(n) DT_INST_PROP_LEN(n, in_addr_gpios)
#define INST_DEMUX_DATA_GPIOS(n) DT_INST_PROP_LEN(n, in_data_gpios)
#define COLS(n) DT_INST_PROP(n, cols) 
#define ROWS(n) DT_INST_PROP_LEN(n, output_gpios)
#define POLL_INTERVAL(n) DT_INST_PROP(n, polling_interval_msec)

#define GPIO_INST_INIT(n)                                                                          \
    struct kscan_gpio_irq_callback_##n {                                                           \
        struct CHECK_DEBOUNCE_CFG(n, (k_work), (k_work_delayable)) * work;                         \
        struct gpio_callback callback;                                                             \
        const struct device *dev;                                                                  \
    };                                                                                             \
                                                                                                   \
    struct kscan_gpio_config_##n {                                                                 \
        const struct gpio_dt_spec rows[ROWS(n)];                                                   \
        const struct gpio_dt_spec col_addr[INST_DEMUX_ADDR_GPIOS(n)];                              \
        const struct gpio_dt_spec col_data[INST_DEMUX_DATA_GPIOS(n)];                              \
    };                                                                                             \
                                                                                                   \
    struct kscan_gpio_data_##n {                                                                   \
        kscan_callback_t callback;                                                                 \
        struct k_timer poll_timer;                                                                 \
        struct CHECK_DEBOUNCE_CFG(n, (k_work), (k_work_delayable)) work;                           \
        bool matrix_state[ROWS(n)][COLS(n)];                                                       \
        const struct device *dev;                                                                  \
    };                                                                                             \
    /* IO/GPIO SETUP */                                                                            \
    static const struct gpio_dt_spec *kscan_gpio_output_specs_##n(const struct device *dev) {      \
        const struct kscan_gpio_config_##n *cfg = dev->config;                                     \
        return cfg->rows;                                                                          \
    }                                                                                              \
                                                                                                   \
    static const struct gpio_dt_spec *kscan_gpio_in_addr_specs_##n(const struct device *dev) {     \
        const struct kscan_gpio_config_##n *cfg = dev->config;                                     \
        return cfg->col_addr;                                                                      \
    }                                                                                              \
                                                                                                   \
    static const struct gpio_dt_spec *kscan_gpio_in_data_specs_##n(const struct device *dev) {     \
        const struct kscan_gpio_config_##n *cfg = dev->config;                                     \
        return cfg->col_data;                                                                      \
    }                                                                                              \
    /* POLLING SETUP */                                                                            \
    static void kscan_gpio_timer_handler(struct k_timer *timer) {                                  \
        struct kscan_gpio_data_##n *data =                                                         \
            CONTAINER_OF(timer, struct kscan_gpio_data_##n, poll_timer);                           \
        k_work_submit(&data->work.work);                                                           \
    }                                                                                              \
                                                                                                   \
    /* Read the state of the input GPIOs */                                                        \
    /* This is the core matrix_scan func */                                                        \
    static int kscan_gpio_read_##n(const struct device *dev) {                                     \
        bool submit_follow_up_read = false;                                                        \
        struct kscan_gpio_data_##n *data = dev->data;                                              \
        static bool read_state[ROWS(n)][COLS(n)];                                                  \
        for (int row = 0; row < ROWS(n); row++){                                                   \
            /* reset all rows */                                                                   \         
            for (int i = 0; i < ROWS(n); i++) {                                                    \
                gpio_pin_set_dt(&kscan_gpio_output_specs_##n(dev)[i], 0);                          \
            }                                                                                      \
                                                                                                   \
            /* set our desired row */                                                              \ 
            gpio_pin_set_dt(&kscan_gpio_output_specs_##n(dev)[row], 1);                            \
                                                                                                   \
            /* sense our columns, this driver iterates over the DEMUX_ADDR lines of the column      
               demuxes and checks each DEMUX_DATA output which correpsond to one column in each
               chunk (one chunk is the # of demuxes, aka the number of DEMUX_DATA lines) */        \
            for (int col = 0; col < COLS(n); col += INST_DEMUX_DATA_GPIOS(n)){                     \
                for (int addr = 0; addr < INST_DEMUX_ADDR_GPIOS(n); addr++){                       \
                    uint32_t addr_line_val = (col >> (INST_DEMUX_DATA_GPIOS(n)-1)) & BIT(addr);    \
                    gpio_pin_set_dt(&kscan_gpio_in_addr_specs_##n(dev)[addr], addr_line_val);      \
                }                                                                                  \
                                                                                                   \
                for (int data = 0; data < INST_DEMUX_DATA_GPIOS(n); data++){                       \
                    const struct gpio_dt_spec *data_spec = &kscan_gpio_in_data_specs_##n(dev)[data];    \
                    read_state[row][col + data] = gpio_pin_get_dt(data_spec) > 0;                  \
                    if (read_state[row][col+data]) LOG_DBG("Read active pin at %d,%d", row, col+data);  \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        for (int r = 0; r < ROWS(n); r++) {                                                        \
            for (int c = 0; c < COLS(n); c++) {                                                    \
                bool pressed = read_state[r][c];                                                   \
                submit_follow_up_read = (submit_follow_up_read || pressed);                        \
                if (pressed != data->matrix_state[r][c]) {                                         \
                    LOG_DBG("Sending event at %d,%d state %s", r, c, (pressed ? "on" : "off"));    \
                    data->matrix_state[r][c] = pressed;                                            \
                    data->callback(dev, r, c, pressed);                                            \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        if (submit_follow_up_read) {                                                               \
            CHECK_DEBOUNCE_CFG(n, ({ k_work_submit(&data->work); }),                               \
                               ({ k_work_reschedule(&data->work, K_MSEC(5)); }))                   \
        }                                                                                          \
        return 0;                                                                                  \
    }                                                                                              \
                                                                                                   \
    static void kscan_gpio_work_handler_##n(struct k_work *work) {                                 \
        struct k_work_delayable *d_work = k_work_delayable_from_work(work);                        \
        struct kscan_gpio_data_##n *data = CONTAINER_OF(d_work, struct kscan_gpio_data_##n, work); \
        kscan_gpio_read_##n(data->dev);                                                            \
    }                                                                                              \
                                                                                                   \
    static struct kscan_gpio_data_##n kscan_gpio_data_##n = {};                                    \
                                                                                                   \
    /* KSCAN API configure function */                                                             \
    static int kscan_gpio_configure_##n(const struct device *dev, kscan_callback_t callback) {     \
        LOG_DBG("KSCAN API configure");                                                            \
        struct kscan_gpio_data_##n *data = dev->data;                                              \
        if (!callback) {                                                                           \
            return -EINVAL;                                                                        \
        }                                                                                          \
        data->callback = callback;                                                                 \
        LOG_DBG("Configured GPIO %d", n);                                                          \
        return 0;                                                                                  \
    };                                                                                             \
                                                                                                   \
    /* KSCAN API enable function */                                                                \
    static int kscan_gpio_enable_##n(const struct device *dev) {                                   \
        LOG_DBG("KSCAN API enable");                                                               \
        struct kscan_gpio_data_##n *data = dev->data;                                              \
        /* TODO: we might want a follow up to hook into the sleep state hooks in Zephyr, */        \
        /* and disable this timer when we enter a sleep state */                                   \
        k_timer_start(&data->poll_timer, K_MSEC(POLL_INTERVAL(n)), K_MSEC(POLL_INTERVAL(n)));      \
        return 0;                                                                                  \
    };                                                                                             \
                                                                                                   \
    /* KSCAN API disable function */                                                               \
    static int kscan_gpio_disable_##n(const struct device *dev) {                                  \
        LOG_DBG("KSCAN API disable");                                                              \
        struct kscan_gpio_data_##n *data = dev->data;                                              \
        k_timer_stop(&data->poll_timer);                                                           \
        return 0;                                                                                  \
    };                                                                                             \
                                                                                                   \
    /* GPIO init function*/                                                                        \
    static int kscan_gpio_init_##n(const struct device *dev) {                                     \
        LOG_DBG("KSCAN GPIO init");                                                                \
        struct kscan_gpio_data_##n *data = dev->data;                                              \
        int err;                                                                                   \
        /* configure input devices*/                                                               \
        for (int i = 0; i < INST_DEMUX_ADDR_GPIOS(n); i++) {                                       \
            const struct gpio_dt_spec *in_spec = &kscan_gpio_in_addr_specs_##n(dev)[i];            \
            if (!device_is_ready(in_spec->port)) {                                                 \
                LOG_ERR("Unable to find input GPIO device");                                       \
                return -EINVAL;                                                                    \
            }                                                                                      \
            err = gpio_pin_configure_dt(in_spec, GPIO_OUTPUT_INACTIVE);                            \
            if (err) {                                                                             \
                LOG_ERR("Unable to configure pin %d for address input", in_spec->pin);             \
                return err;                                                                        \
            } else {                                                                               \
                LOG_DBG("Configured pin %d for address input", in_spec->pin);                      \
            }                                                                                      \
            if (err) {                                                                             \
                LOG_ERR("Error adding the callback to the column address device");                 \
                return err;                                                                        \
            }                                                                                      \
        }                                                                                          \
        for (int i = 0; i < INST_DEMUX_DATA_GPIOS(n); i++) {                                       \
            const struct gpio_dt_spec *in_spec = &kscan_gpio_in_data_specs_##n(dev)[i];            \
            if (!device_is_ready(in_spec->port)) {                                                 \
                LOG_ERR("Unable to find input GPIO device");                                       \
                return -EINVAL;                                                                    \
            }                                                                                      \
            err = gpio_pin_configure_dt(in_spec, GPIO_INPUT);                                      \
            if (err) {                                                                             \
                LOG_ERR("Unable to configure pin %d for data input", in_spec->pin);                \
                return err;                                                                        \
            } else {                                                                               \
                LOG_DBG("Configured pin %d for data input", in_spec->pin);                         \
            }                                                                                      \
            if (err) {                                                                             \
                LOG_ERR("Error adding the callback to the column data device");                    \
                return err;                                                                        \
            }                                                                                      \
        }                                                                                          \
        /* configure output devices*/                                                              \
        for (int o = 0; o < ROWS(n); o++) {                                         \
            const struct gpio_dt_spec *out_spec = &kscan_gpio_output_specs_##n(dev)[o];            \
            if (!device_is_ready(out_spec->port)) {                                                \
                LOG_ERR("Unable to find output GPIO device");                                      \
                return -EINVAL;                                                                    \
            }                                                                                      \
            err = gpio_pin_configure_dt(out_spec, GPIO_OUTPUT_INACTIVE);                           \
            if (err) {                                                                             \
                LOG_ERR("Unable to configure pin %d for row output", out_spec->pin);               \
                return err;                                                                        \
            } else {                                                                               \
                LOG_DBG("Configured pin %d for row output", out_spec->pin);                        \
            }                                                                                      \
        }                                                                                          \
        data->dev = dev;                                                                           \
                                                                                                   \
        k_timer_init(&data->poll_timer, kscan_gpio_timer_handler, NULL);                           \
                                                                                                   \
        (CHECK_DEBOUNCE_CFG(n, (k_work_init), (k_work_init_delayable)))(                           \
            &data->work, kscan_gpio_work_handler_##n);                                             \
        return 0;                                                                                  \
    }                                                                                              \
                                                                                                   \
    static const struct kscan_driver_api gpio_driver_api_##n = {                                   \
        .config = kscan_gpio_configure_##n,                                                        \
        .enable_callback = kscan_gpio_enable_##n,                                                  \
        .disable_callback = kscan_gpio_disable_##n,                                                \
    };                                                                                             \
                                                                                                   \
    static const struct kscan_gpio_config_##n kscan_gpio_config_##n = {                            \
        .rows = {DT_FOREACH_PROP_ELEM(DT_DRV_INST(n), output_gpios, _KSCAN_GPIO_CFG_INIT)},        \
        .col_addr = {DT_FOREACH_PROP_ELEM(DT_DRV_INST(n), in_addr_gpios, _KSCAN_GPIO_CFG_INIT)},   \
        .col_data = {DT_FOREACH_PROP_ELEM(DT_DRV_INST(n), in_data_gpios, _KSCAN_GPIO_CFG_INIT)},   \ 
    };                                                                                             \
                                                                                                   \
    DEVICE_DT_INST_DEFINE(n, kscan_gpio_init_##n, NULL, &kscan_gpio_data_##n,                      \
                          &kscan_gpio_config_##n, POST_KERNEL, CONFIG_KSCAN_INIT_PRIORITY,         \
                          &gpio_driver_api_##n);

DT_INST_FOREACH_STATUS_OKAY(GPIO_INST_INIT)
