#pragma once

#include <lvgl.h>

namespace Utils
{
   template <class T> constexpr T clamp(T val, T max, T min)
   {
      return (val > max) ? max : ((val < min) ? min : val);
   }
}