#pragma once

#include "canvas_object.hpp"
#include "setting.hpp"
#include <lvgl.h>

class Slider : public CanvasObject
{
    private:
    SliderSetting& setting;
    std::int32_t oldValue = 0;
    lv_draw_rect_dsc_t boundingRectDesc{};
    lv_draw_rect_dsc_t slideRectDesc{};

    public:
    Slider(lv_area_t coords, SliderSetting& setting) : CanvasObject(coords), setting(setting)
    {
        lv_draw_rect_dsc_init(&boundingRectDesc);
        lv_draw_rect_dsc_init(&slideRectDesc);
    }

    void setDesc(std::function<void(lv_draw_rect_dsc_t& boundingDesc, lv_draw_rect_dsc_t& slideDesc)> const& callback)
    {
       callback(boundingRectDesc, slideRectDesc);
    }

    bool tick() override;

    void draw(MiniCanvas* canvas) override;
};