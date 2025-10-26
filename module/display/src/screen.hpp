#pragma once

#include "canvas_object.hpp"
#include "misc/lv_color.h"
#include "zephyr/logging/log.h"
#include <memory>
#include <vector>

#include "utils.hpp"

struct Screen final
{
    std::vector<CanvasObject*> elements{};
    lv_color_t backgroundColor;
    bool hasBackground = false;

   void tick()
   {
      for (auto& elem : elements)
      {
         if (elem != nullptr)
         {
            elem->tick();
         }
      }
   }
};