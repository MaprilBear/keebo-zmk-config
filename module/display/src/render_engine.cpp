#include "render_engine.hpp"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "canvas_object.hpp"

LOG_MODULE_DECLARE(display_app);

RenderEngine::RenderEngine() : canvasElements{}
{
   lv_draw_label_dsc_init(&fps_label_dsc);
   fps_label_dsc.font = &lv_font_unscii_8;
   fps_label_dsc.color = lv_color_white();
}

void RenderEngine::addCanvasElement(std::unique_ptr<CanvasObject> object)
{
   canvasElements.emplace_back(std::move(object));
}

void RenderEngine::tick()
{
    // Process statistics
   uint32_t frame_time = k_cycle_get_32();
   fps = 1000 / k_cyc_to_ms_floor32(frame_time - last_frame_time);
   LOG_INF("Current FPS = %d, Elapsed time = %d", fps, k_cyc_to_ms_floor32(frame_time - last_frame_time));
   last_frame_time = frame_time;

   for (auto& object : canvasElements)
   {
      object->tick();
   }
}

void RenderEngine::draw(MiniCanvas* canvas)
{
   // LOG_INF("Drawing canvas");

   for (auto& object : canvasElements)
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