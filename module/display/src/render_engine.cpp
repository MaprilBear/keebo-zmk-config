#include "render_engine.hpp"
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "canvas_object.hpp"
#include "screen.hpp"

LOG_MODULE_DECLARE(display_app);

RenderEngine::RenderEngine()
{
}

void RenderEngine::draw(Screen& screen, MiniCanvas* canvas)
{
   // LOG_INF("Drawing canvas");

   for (auto& object : screen.elements)
   {
      // LOG_INF("Drawing canvas object");
      object->draw(canvas);
   }

   // LOG_INF("Finished drawing canvas");
}