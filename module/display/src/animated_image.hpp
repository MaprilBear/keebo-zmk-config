#pragma once

#include "image.hpp"
#include <cstdint>
#include <lvgl.h>
#include <string>
#include <zephyr/fs/fs.h>

class AnimatedImage final : public Image
{
   private:
   std::uint32_t counter = 0;
   std::uint16_t numFrames = 0;
   std::string filePrefix{};
   std::string fileSuffix{};

   std::string generateFileName()
   {
      return filePrefix + std::to_string(counter) + fileSuffix;
   }

   public:
   AnimatedImage(lv_area_t coords, std::string prefix, std::string suffix, std::uint8_t numFrames);

   bool tick() override;
};