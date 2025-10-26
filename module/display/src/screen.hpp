#pragma once

#include "canvas_object.hpp"
#include "zephyr/logging/log.h"
#include <memory>
#include <vector>

#include "utils.hpp"

struct Screen final
{
    std::vector<std::unique_ptr<CanvasObject>> elements{};

    void tick()
    {
        for (auto& elem : elements)
        {
           elem->tick();
        }
    }
};