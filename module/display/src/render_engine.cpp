#include "render_engine.hpp"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <cstring>

#include "misc/lv_color.h"
#include "screen.hpp"
#include "widgets/lv_canvas.h"

LOG_MODULE_DECLARE(display_app);

RenderEngine::RenderEngine()
{
}

void RenderEngine::draw(Screen& screen, MiniCanvas* canvas)
{
   if (screen.hasBackground)
   {
      lv_canvas_fill_bg(reinterpret_cast<lv_obj_t*>(canvas), screen.backgroundColor, LV_OPA_100);
   }

   for (auto& object : screen.elements)
   {
      // LOG_INF("Drawing canvas object");
      object->draw(canvas);
   }

   // LOG_INF("Finished drawing canvas");
}