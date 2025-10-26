#pragma once

#include "canvas_object.hpp"
#include "mini_canvas.hpp"
#include "render_engine.hpp"
#include <cstdint>
#include <memory>
#include <zephyr/drivers/display.h>

#include "screen.hpp"
#include "utils.hpp"

class ScreenManager
{
   private:
   // Static screen params
   static constexpr auto SCREEN_WIDTH = 320;
   static constexpr auto SCREEN_HEIGHT = 172;
   static constexpr auto IMAGE_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 2;
   alignas(4) static std::uint8_t image_buffer[IMAGE_SIZE];
   volatile bool running = true;

   static constexpr auto display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   std::unique_ptr<RenderEngine> renderEngine;
   struct display_buffer_descriptor display_desc;
   MiniCanvas miniCanvas1;
   MiniCanvas miniCanvas2;

   Screen* currentScreen;

   ScreenManager();

   public:
   static ScreenManager& getScreenManager()
   {
      static ScreenManager screenManager{};
      return screenManager;
   }

   ScreenManager(ScreenManager const&) = delete;
   void operator=(ScreenManager const&) = delete;

   void tick();
   void draw(MiniCanvas* canvas);
   void flush(MiniCanvas* canvas);
   void loop();

   void setScreen(Screen* screen)
   {
      currentScreen = screen;
   }

   Screen* getScreen()
   {
      return currentScreen;
   }

   static void flushLoop();

   void pause()
   {
      running = false;
   }
   void resume()
   {
      running = true;
   }
};