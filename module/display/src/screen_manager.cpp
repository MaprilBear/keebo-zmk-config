#include "screen_manager.hpp"

#include <lvgl.h>

#include <cstdint>

#include "utils.hpp"
#include <cstring>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(display_app);

namespace
{
   K_SEM_DEFINE(readSema1, 1, 1);
   K_SEM_DEFINE(readSema2, 1, 1);
   K_SEM_DEFINE(flushSema1, 0, 1);
   K_SEM_DEFINE(flushSema2, 0, 1);
} // namespace

std::uint8_t ScreenManager::image_buffer[IMAGE_SIZE];

ScreenManager::ScreenManager()
{
   renderEngine = make_unique<RenderEngine>();

   // Intialize our MiniCanvases
   lv_canvas_t* lvCanvas = reinterpret_cast<lv_canvas_t*>(lv_canvas_create(lv_scr_act()));
   std::memcpy(&miniCanvas1, lvCanvas, sizeof(lv_canvas_t));
   miniCanvas1.canvasBuffer = image_buffer;
   lv_canvas_set_buffer(reinterpret_cast<lv_obj_t*>(&miniCanvas1), miniCanvas1.canvasBuffer, SCREEN_WIDTH,
                        SCREEN_HEIGHT / 2, LV_IMG_CF_TRUE_COLOR);
   miniCanvas1.img.obj.coords = lv_area_t{0, 0, 319, 85};

   lvCanvas = reinterpret_cast<lv_canvas_t*>(lv_canvas_create(lv_scr_act()));
   std::memcpy(&miniCanvas2, lvCanvas, sizeof(lv_canvas_t));
   miniCanvas2.canvasBuffer = &image_buffer[SCREEN_WIDTH * (SCREEN_HEIGHT / 2) * 2];
   lv_canvas_set_buffer(reinterpret_cast<lv_obj_t*>(&miniCanvas2), miniCanvas2.canvasBuffer, SCREEN_WIDTH,
                        SCREEN_HEIGHT / 2, LV_IMG_CF_TRUE_COLOR);
   miniCanvas2.img.obj.coords = lv_area_t{0, 86, 319, 171};

   // Initialize display descriptor
   display_desc.buf_size = IMAGE_SIZE / 2;
   display_desc.width = 320;
   display_desc.height = 86;
   display_desc.pitch = 0;
}

void ScreenManager::tick()
{
   renderEngine->tick();
}

void ScreenManager::draw(MiniCanvas* canvas)
{
   renderEngine->draw(canvas);
}

void ScreenManager::flush(MiniCanvas* canvas)
{
   LOG_INF("Flushing...");
   display_write(display_dev, canvas->img.obj.coords.x1, canvas->img.obj.coords.y1, &display_desc,
                 canvas->canvasBuffer);
   LOG_INF("Flushing complete!");
}

void ScreenManager::addElement(std::unique_ptr<CanvasObject> element)
{
   renderEngine->addCanvasElement(std::move(element));
}

void ScreenManager::loop()
{
   while (1)
   {
      tick();

      k_sem_take(&readSema1, K_FOREVER);
      draw(&miniCanvas1);
      k_sem_give(&flushSema1);

      k_sem_take(&readSema2, K_FOREVER);
      draw(&miniCanvas2);
      k_sem_give(&flushSema2);
   }
}

void ScreenManager::flushLoop()
{
   while (1)
   {
      k_sem_take(&flushSema1, K_FOREVER);
      flush(&miniCanvas1);
      k_sem_give(&readSema1);

      k_sem_take(&flushSema2, K_FOREVER);
      flush(&miniCanvas2);
      k_sem_give(&readSema2);
   }
}
