
#pragma once

#include <lvgl.h>

#include <memory>
#include <vector>

#include "canvas_object.hpp"
#include "screen.hpp"


class RenderEngine
{
   private:
   lv_draw_label_dsc_t fps_label_dsc{};
   lv_coord_t fpsX = 260;
   lv_coord_t fpsY = 160;
   std::uint16_t fps = 0;
   std::uint32_t last_frame_time = 0;

   public:
   std::uint8_t drawBackground = 0;

   RenderEngine();

   void draw(Screen& screen, MiniCanvas* canvas);
};