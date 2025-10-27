#pragma once

#include <cstdint>
#include <functional>

namespace
{
   template <class T> constexpr T clamp(T val, T max, T min)
   {
      return (val > max) ? max : ((val < min) ? min : val);
   }
} // namespace

class Setting
{
};

class SliderSetting : public Setting
{
   protected:
   std::int32_t value;
   std::int32_t max;
   std::int32_t min;
   std::function<void(std::int32_t)> callback;

   public:
   SliderSetting(std::int32_t start, std::int32_t max, std::int32_t min,
                 std::function<void(std::int32_t)> const& callback)
       : value(start), max(max), min(min), callback(callback)
   {
   }

   void addDelta(std::int32_t delta)
   {
      std::int32_t newValue = clamp<std::int32_t>(value + delta, max, min);
      std::int32_t actualDelta = newValue - value;
      callback(actualDelta);
      value = newValue;
   }

   void setValue(std::int32_t val)
   {
      std::int32_t newValue = clamp<std::int32_t>(val, max, min);
      std::int32_t delta = newValue - value;
      callback(delta);
      value = newValue;
   }
};

class ToggleSetting : public Setting
{
   protected:
   bool currentToggle;
   std::function<void(bool)> callback;

   public:
   ToggleSetting(bool start) : currentToggle(start)
   {
   }

   void toggle()
   {
      currentToggle = !currentToggle;
      callback(currentToggle);
   }
};