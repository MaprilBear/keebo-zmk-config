#define DT_DRV_COMPAT zmk_behavior_app
// Dependencies
#include <drivers/behavior.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util_macro.h>
#include <zmk/behavior_queue.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>

#include <dt-bindings/app_keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// Instance-Unique Data Struct (Optional)
// struct behavior_<behavior_name> _data
// {
//    bool example_data_param1;
//    bool example_data_param2;
//    bool example_data_param3;
// };

// Instance-Unique Config Struct (Optional)
struct behavior_app_config
{
   bool placeholder;
   struct zmk_behavior_binding bindings[];
};

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

   void switchScreensC(int);

   void processKeyPress(int, struct zmk_behavior_binding_event);

   void invokeBinding(int num)
   {
      const struct device* dev = zmk_behavior_get_binding("app");
      const struct behavior_app_config* cfg = dev->config;

      struct zmk_behavior_binding_event event = {0, 1, 0};

      LOG_PRINTK("Invoking binding %d: ", num);
      LOG_PRINTK("%s %d\n", cfg->bindings[num].behavior_dev, cfg->bindings[num].param1);

      zmk_behavior_invoke_binding(&cfg->bindings[num], event, true);
   }

#ifdef __cplusplus
}
#endif

static int on_keymap_binding_pressed(struct zmk_behavior_binding* binding, struct zmk_behavior_binding_event event)
{

   LOG_PRINTK("Uhhhh, hello? %d %s\n", binding->param1, binding->behavior_dev);

   switch (binding->param1)
   {
      case APP_A:
      case APP_B:
      case APP_INC:
      case APP_DEC:
         processKeyPress(binding->param1, event);
         break;
      case APP_MAIN:
         switchScreensC(0);
         break;
      case APP_SETTINGS:
         switchScreensC(1);
         break;
   }
   return ZMK_BEHAVIOR_OPAQUE;
}



#define TRANSFORMED_BEHAVIORS(n) {LISTIFY(DT_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), n)}

    // API Structure
    static const struct behavior_driver_api behavior_app_driver_api = {.binding_pressed = on_keymap_binding_pressed};

static struct behavior_app_config behavior_app_config_inst = {.placeholder = false,
                                                              .bindings = TRANSFORMED_BEHAVIORS(DT_NODELABEL(app))};

BEHAVIOR_DT_INST_DEFINE(0,          // Instance Number (Equal to 0 for behaviors that don't require multiple instances,
                                    //                  Equal to n for behaviors that do make use of multiple instances)
                        NULL, NULL, // Initialization Function, Power Management Device Pointer (Both Optional)
                        NULL,       // Behavior Data Pointer (optional)
                        &behavior_app_config_inst,                        // Behavior Configuration Pointer (Optional)
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, // Initialization Level, Device Priority
                        &behavior_app_driver_api);                        // API Structure

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */