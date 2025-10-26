#pragma once

#include <lvgl.h>
#include <cstdint>

struct MiniCanvas : public lv_canvas_t
{
   std::uint8_t* canvasBuffer;
};