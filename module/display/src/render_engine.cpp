#include "render_engine.hpp"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "canvas_object.hpp"
#include "screen.hpp"

LOG_MODULE_DECLARE(display_app);

RenderEngine::RenderEngine()
{
   lv_draw_label_dsc_init(&fps_label_dsc);
   fps_label_dsc.font = &lv_font_unscii_8;
   fps_label_dsc.color = lv_color_white();
}

void RenderEngine::draw(Screen& screen, MiniCanvas* canvas)
{
   // LOG_INF("Drawing canvas");

   for (auto& object : screen.elements)
   {
      // LOG_INF("Drawing canvas object");
      object->draw(canvas);
   }

   char buffer[15];
   snprintk(buffer, 15, "%02d FPS\0", fps);
   lv_canvas_draw_text(reinterpret_cast<lv_obj_t*>(canvas), fpsX - canvas->img.obj.coords.x1,
                       fpsY - canvas->img.obj.coords.y1, 1000, &fps_label_dsc, buffer);

   // LOG_INF("Finished drawing canvas");
}