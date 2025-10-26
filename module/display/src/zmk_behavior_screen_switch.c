
#define DT_DRV_COMPAT zmk_behavior_screen_switch
// Dependencies
#include <drivers/behavior.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

// #include "main.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// Instance-Unique Data Struct (Optional)
// struct behavior_<behavior_name> _data
// {
//    bool example_data_param1;
//    bool example_data_param2;
//    bool example_data_param3;
// };

// // Instance-Unique Config Struct (Optional)
// struct behavior_<behavior_name> _config
// {
//    bool example_config_param1;
//    bool example_config_param2;
//    bool example_config_param3;
// };

// Initialization Function (Optional)
// static int<behavior_name> _init(const struct device* dev)
// {
//    return 0;
// };

// K_SEM_DEFINE(screenSwitchSema, 0, 1);

#ifdef __cplusplus
extern "C"
{
#endif

   void switchScreensC();

#ifdef __cplusplus
}
#endif

static int on_keymap_binding_pressed(struct zmk_behavior_binding* binding, struct zmk_behavior_binding_event event)
{
   LOG_INF("Uhhhh, hello?");
   switchScreensC();
   return 0;
}

// API Structure
static const struct behavior_driver_api behavior_screen_switch_driver_api = {.binding_pressed =
                                                                                 on_keymap_binding_pressed};

BEHAVIOR_DT_INST_DEFINE(0,          // Instance Number (Equal to 0 for behaviors that don't require multiple instances,
                                    //                  Equal to n for behaviors that do make use of multiple instances)
                        NULL, NULL, // Initialization Function, Power Management Device Pointer (Both Optional)
                        NULL,       // Behavior Data Pointer (optional)
                        NULL,       // Behavior Configuration Pointer (Optional)
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, // Initialization Level, Device Priority
                        &behavior_screen_switch_driver_api);               // API Structure

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */