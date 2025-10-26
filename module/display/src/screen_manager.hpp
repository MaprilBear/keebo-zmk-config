#pragma once

#include "canvas_object.hpp"
#include "mini_canvas.hpp"
#include "render_engine.hpp"
#include <cstdint>
#include <zephyr/drivers/display.h>

class ScreenManager
{
   private:
    // Static screen params
    static constexpr auto SCREEN_WIDTH = 320;
    static constexpr auto SCREEN_HEIGHT = 172;
    static constexpr auto IMAGE_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 2;
    alignas(4) static std::uint8_t image_buffer[IMAGE_SIZE];

    static constexpr auto display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    std::unique_ptr<RenderEngine> renderEngine;
    struct display_buffer_descriptor display_desc;
    MiniCanvas miniCanvas1;
    MiniCanvas miniCanvas2;

    public:
    ScreenManager();

    void tick();
    void draw(MiniCanvas* canvas);
    void flush(MiniCanvas* canvas);
    void addElement(std::unique_ptr<CanvasObject> element);
    void loop();
    void flushLoop();
};