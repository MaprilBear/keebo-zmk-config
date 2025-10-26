#pragma once

#include "canvas_object.hpp"
#include "mini_canvas.hpp"
#include <lvgl.h>

class Image : public CanvasObject
{
   protected:
   lv_fs_file_t file;

   public:
   Image(lv_area_t coords);

   void draw(MiniCanvas* canvas) override;
};