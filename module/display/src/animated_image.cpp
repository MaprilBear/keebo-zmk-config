#include "animated_image.hpp"

#include "image.hpp"
#include <lvgl.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>

#include <string>
#include <cstdint>

#include LV_MEM_CUSTOM_INCLUDE

LOG_MODULE_DECLARE(display_app);

namespace
{
   lv_fs_res_t my_lvgl_close(struct _lv_fs_drv_t* drv, void* file)
   {
      int err;

      err = fs_close((struct fs_file_t*)file);
      LV_MEM_CUSTOM_FREE(file);
      return err;
   }
}

AnimatedImage::AnimatedImage(lv_area_t coords, std::string prefix, std::string suffix, std::uint8_t numFrames)
    : Image(coords), counter(0), numFrames(numFrames), filePrefix(prefix), fileSuffix(suffix)
{
}

bool AnimatedImage::tick()
{
   // LOG_PRINTK("Ticking animation\n");
   // The LVGL version that ZMK uses has a memory leak on file close.
   if (file.file_d != nullptr)
   {
      lv_fs_close(&file);
   }

   std::string fileName = generateFileName();
   // LOG_PRINTK("Opening %s\n", fileName.c_str());
   lv_res_t res = lv_fs_open(&file, fileName.c_str(), LV_FS_MODE_RD);
   if (res != LV_FS_RES_OK)
   {
      LOG_ERR("File %s failed to open", fileName.c_str());
      counter = 0;
      return false;
   }
   counter = (counter + 1) % numFrames;
   file.drv->close_cb = my_lvgl_close;

   return true;
}