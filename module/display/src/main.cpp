#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <cstring>
#include <memory>
#include <utility>

#include "screen_manager.hpp"
#include "animated_image.hpp"
#include "utils.hpp"

LOG_MODULE_REGISTER(display_app);

std::unique_ptr<ScreenManager> screen = nullptr;

K_SEM_DEFINE(flushStartSema, 0, 1);

int display_thread(void)
{
   const struct device* display_dev;

   k_sleep(K_MSEC(1000));

   display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   if (!device_is_ready(display_dev))
   {
      LOG_ERR("Device not ready, aborting test");
      return 0;
   }

   k_sleep(K_MSEC(1000)); // let the flash disk settle

   display_blanking_off(display_dev);

   screen = make_unique<ScreenManager>();

   auto animation = make_unique<AnimatedImage>(lv_area_t{0, 0, 319, 171}, "/NAND:/frame_", ".bin", 11);
   screen->addElement(std::move(animation));

   k_sem_give(&flushStartSema);
   screen->loop();

   return 0;
}

K_THREAD_DEFINE(dsp_thread, 4096, display_thread, NULL, NULL, NULL, 2, 0, 0);

void flush_thread()
{
   k_sem_take(&flushStartSema, K_FOREVER);
   screen->flushLoop();
}

K_THREAD_DEFINE(flushing_thread, 2048, flush_thread, NULL, NULL, NULL, 2, 0, 0);
