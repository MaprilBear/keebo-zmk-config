#include "slider.hpp"
#include "mini_canvas.hpp"
#include "misc/lv_area.h"
#include "widgets/lv_canvas.h"
#include "zephyr/logging/log.h"
#include "utils.hpp"

LOG_MODULE_DECLARE(display_app);

lv_area_t operator-(lv_area_t const& coord1, lv_area_t const& coord2)
{
   return lv_area_t
   {
      coord1.x1 - coord2.x1, coord1.y1 - coord2.y1, coord1.x2 - coord2.x2, coord1.y2 - coord2.y2,
   };
}


bool Slider::tick()
{
   bool retVal = setting.getValue() != oldValue;
   oldValue = setting.getValue();
   return retVal;
}

void Slider::draw(MiniCanvas* canvas)
{
   lv_area_t adjustedCoords = coords - canvas->img.obj.coords;
   lv_coord_t boundWidth = coords.x2 - coords.x1;
   lv_coord_t boundHeight = coords.y2 - coords.y1;

   LOG_PRINTK("X1: %d\nY1: %d\n", adjustedCoords.x1, adjustedCoords.y1);
   LOG_PRINTK("W: %d\nH: %d\n", boundWidth, boundHeight);

   // Draw a bounding box the size of the coords
   lv_canvas_draw_rect(reinterpret_cast<lv_obj_t*>(canvas), adjustedCoords.x1, adjustedCoords.y1,
                       boundWidth, boundHeight, &boundingRectDesc);

   // Calculate the percentage that should be filled
   std::int32_t range = setting.getMax() - setting.getMin();
   std::int32_t percent = (100 * (setting.getValue() - setting.getMin())) / range;
   std::int32_t sliderWidth = (boundWidth * percent) / 100;

   LOG_PRINTK("Range: %d\n Percent: %d\n Slider Width: %d\n", range, percent, sliderWidth);

   // Draw the slider
   lv_canvas_draw_rect(reinterpret_cast<lv_obj_t*>(canvas), adjustedCoords.x1, adjustedCoords.y1,
                       sliderWidth, boundHeight, &slideRectDesc);
}