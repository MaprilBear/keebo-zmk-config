#pragma once

#include "mini_canvas.hpp"
#include "misc/lv_area.h"
#include <lvgl.h>

class CanvasObject
{
   protected:
   lv_area_t coords{};

   public:
   CanvasObject(lv_area_t coords) : coords(coords)
   {
   }

   virtual bool tick() { return false; }
   virtual void draw(MiniCanvas* canvas) {}

   bool inBounds(MiniCanvas* canvas)
   {
      return (this->coords.x1 >= canvas->img.obj.coords.x1) || (this->coords.y1 >= canvas->img.obj.coords.y1) ||
             (this->coords.x2 <= canvas->img.obj.coords.x2) || (this->coords.y2 <= canvas->img.obj.coords.y2);
   }

   void setPosition(lv_point_t newCoords)
   {
      coords.x2 = (coords.x2 - coords.x1) + newCoords.x;
      coords.y2 = (coords.y2 - coords.y1) + newCoords.y;
      coords.x1 = newCoords.x;
      coords.y1 = newCoords.y;
   }
};