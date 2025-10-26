#pragma once

#include "canvas_object.hpp"
#include "zephyr/logging/log.h"
#include <memory>
#include <vector>

#include "utils.hpp"

struct Screen final
{
   std::vector<CanvasObject*> elements{};

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