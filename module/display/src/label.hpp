#include "canvas_object.hpp"
#include "draw/lv_draw_label.h"
#include "mini_canvas.hpp"
#include "misc/lv_area.h"
#include <functional>
#include <lvgl.h>
#include <string>

class Label : public CanvasObject
{
   private:
   lv_draw_label_dsc_t labelDesc{};
   std::string text{};
   std::function<bool(lv_draw_label_dsc_t&, std::string&)> tickCallback{};

   public:
   void setDesc(std::function<void(lv_draw_label_dsc_t& desc)> const& callback)
   {
      callback(labelDesc);
   }

   void setText(std::string const& str)
   {
      text = str;
   }

   void setTickCallback(std::function<bool(lv_draw_label_dsc_t&, std::string&)> const& callback)
   {
      tickCallback = callback;
   }

   bool tick() override
   {
      if (tickCallback)
      {
         return tickCallback(labelDesc, text);
      }

      return false;
   }

   void draw(MiniCanvas* canvas) override
   {
      lv_canvas_draw_text(reinterpret_cast<lv_obj_t*>(canvas), coords.x1 - canvas->img.obj.coords.x1,
                          coords.y1 - canvas->img.obj.coords.y1, 1000, &labelDesc, text.c_str());
   }

   Label(lv_point_t coords) : CanvasObject(lv_area_t{coords.x, coords.y, 0, 0})
   {
   }
};