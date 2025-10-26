#include "image.hpp"

#include "canvas_object.hpp"
#include "mini_canvas.hpp"
#include <lvgl.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(display_app);

Image::Image(lv_area_t coords) : CanvasObject(coords)
{
}

void Image::draw(MiniCanvas* canvas)
{
   if (!inBounds(canvas))
   {
      LOG_INF("Image is not in bounds of the canvas, skipping...");
      return;
   }

   if (file.file_d == nullptr)
   {
      LOG_ERR("File is null :(");
      return;
   }

   // Check if this image is left aligned to the canvas (no x offset and the widths are the same).
   // If the image is aligned, the image can be rendered onto the canvas in one go since we do not need to stop
   // streaming image file data every line to crop and pad.
   std::uint16_t imageWidth = this->coords.x2 - this->coords.x1 + 1;
   std::uint16_t imageHeight = this->coords.y2 - this->coords.y1 + 1;
   auto& canvasCoords = canvas->img.obj.coords;

   if (this->coords.x1 == 0 && canvas->img.w == imageWidth)
   {
      // Skip over the image header and any rows that are obscured by the canvas' current bound
      std::uint32_t filePos = 4; // Skip over image header from LVGL v8 conversion tool
      std::uint32_t canvasPos = 0;
      std::int32_t delta = this->coords.y1 - canvasCoords.y1;
      if (delta < 0)
      {
         // The image is positioned above the start of the canvas viewable area
         // We need to seek futher into the image data to begin reading into canvas[0][0] at the right place.
         filePos = static_cast<std::uint32_t>(-delta) * imageWidth * 2;
      }
      else if (delta > 0)
      {
         // The image is positioned below the start of the canvas vierable area
         // We need to begin reading data into an offset in the canvas, not the file.
         canvasPos = delta * imageWidth * 2;
      }
      else
      {
         // Canvas and image are top-left aligned, nothing to do.
      }

      // Adjust the size of data to be read based on how many of the image rows are in bounds
      std::uint16_t visibleRows = 0;
      if ((this->coords.y1 - canvasCoords.y1) < 0 && (this->coords.y2 - canvasCoords.y2 > 0))
      {
         // Image starts before and ends after. The canvas bounds are within the entire image bounds.
         visibleRows = canvas->img.h;
      }
      else if ((this->coords.y1 - canvasCoords.y1) < 0 && (this->coords.y2 - canvasCoords.y2 < 0))
      {
         // Image starts before and ends before.
         visibleRows = this->coords.y2 - canvasCoords.y1;
      }
      else if ((this->coords.y1 - canvasCoords.y1) > 0 && (this->coords.y2 - canvasCoords.y2 > 0))
      {
         // Image starts after and ends after.
         visibleRows = canvasCoords.y2 - this->coords.y1;
      }
      else if ((this->coords.y1 - canvasCoords.y1) > 0 && (this->coords.y2 - canvasCoords.y2 < 0))
      {
         // Image starts after and ends before. The image bounds are entirely within the canvas bounds
         visibleRows = imageHeight;
      }
      else
      {
         // Image and canvas are perfectly aligned and the same size
         visibleRows = canvas->img.h;
      }

      std::uint32_t croppedSize = visibleRows * imageWidth * 2;
      lv_fs_seek(&file, filePos, LV_FS_SEEK_SET);

      std::uint32_t read_bytes = 0;
      lv_fs_read(&file, &canvas->canvasBuffer[0], croppedSize, &read_bytes);
      if (read_bytes != croppedSize)
      {
         LOG_ERR("Failed to read entire selection, only read %u bytes", read_bytes);
      }
      // LOG_INF("Finished reading");
   }
   else
   {
      // Arbitrary image position is not currently supported
      LOG_INF("Image is not aligned with the canvas, skipping...");
      return;
   }
}