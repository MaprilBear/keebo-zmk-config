// #include "main.h"
#include <lvgl.h>
#include <sstream>
#include <string>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <iomanip>

#include "animated_image.hpp"
#include "draw/lv_draw_label.h"
#include "label.hpp"
#include "misc/lv_area.h"
#include "misc/lv_color.h"
#include "screen.hpp"
#include "screen_manager.hpp"

#include <zmk/events/activity_state_changed.h>

LOG_MODULE_REGISTER(display_app);

K_SEM_DEFINE(flushStartSema, 0, 1);

Screen* mainScreen;
Screen* settingsScreen;

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

   ScreenManager& screenManager = ScreenManager::getScreenManager();

   //setup a simple screen with an animation and FPS label
   mainScreen = new Screen();
   mainScreen->elements.emplace_back(new AnimatedImage(lv_area_t{0, 0, 319, 171}, "/NAND:/frame_", ".bin", 11));

   auto fpsLabel = new Label(lv_point_t{260, 160});
   fpsLabel->setDesc(
       [](lv_draw_label_dsc_t& desc)
       {
          lv_draw_label_dsc_init(&desc);
          desc.font = &lv_font_unscii_8;
          desc.color = lv_color_white();
       });
   fpsLabel->setTickCallback(
       [](lv_draw_label_dsc_t& desc, std::string& text)
       {
          static std::uint32_t last_frame_time = 0;
          static std::uint32_t fps = 0;

          uint32_t frame_time = k_cycle_get_32();
          fps = 1000 / k_cyc_to_ms_floor32(frame_time - last_frame_time);
          last_frame_time = frame_time;

          std::stringstream stream;
          stream << std::setfill('0') << std::setw(2);
          stream << std::to_string(fps) << " FPS";

          text = stream.str();
       });
   mainScreen->elements.emplace_back(fpsLabel);

   // setup settings screen for later
   // settingsScreen = new Screen();
   // settingsScreen->hasBackground = true;
   // settingsScreen->backgroundColor = lv_color_white();

   // auto myLabel = new Label(lv_point_t{100, 100});
   // myLabel->setText("Hello!");
   // myLabel->setDesc(
   //     [](lv_draw_label_dsc_t& desc)
   //     {
   //        lv_draw_label_dsc_init(&desc);
   //        desc.font = &lv_font_unscii_16;
   //        desc.color = lv_color_black();
   //     });
   // settingsScreen->elements.emplace_back(myLabel);

   screenManager.setScreen(mainScreen);

   screenManager.loop();

   return 0;
}

K_THREAD_DEFINE(dsp_thread, 4096, display_thread, NULL, NULL, NULL, 2, 0, 0);

// Hook into ZMK's activity event to pause the display when we enter idle
// Eventually this would also shutoff the backlight, but right now the keypad doesn't have backlight controls :)
// This is implemented incorrectly on purpose. Linker errors occur with as_zmk_activity_state_changed() not being
// defined for some reason.
static int display_activity_listener(const zmk_event_t* eh)
{
   auto stateChange = reinterpret_cast<const zmk_activity_state_changed_event*>(eh);
   switch (stateChange->data.state)
   {
      case ZMK_ACTIVITY_ACTIVE:
         ScreenManager::getScreenManager().resume();
         break;
      case ZMK_ACTIVITY_IDLE:
      case ZMK_ACTIVITY_SLEEP:
         ScreenManager::getScreenManager().pause();
         break;
   }
   return 0;
}

ZMK_LISTENER(activity, display_activity_listener);
ZMK_SUBSCRIPTION(activity, zmk_activity_state_changed);
