#include <lvgl.h>
#include <lvgl_input_device.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include LV_MEM_CUSTOM_INCLUDE

    LOG_MODULE_REGISTER(display_app);

template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args)
{
   return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

constexpr auto LV_FS_FATFS_LETTER = 'N';

constexpr auto REFRESH_RATE = 30;
constexpr auto REFRESH_PERIOD = K_MSEC(1000 / REFRESH_RATE);

static constexpr auto IMAGE_SIZE = DT_PROP(DT_CHOSEN(zephyr_display), width) *
                                   DT_PROP(DT_CHOSEN(zephyr_display), height) *
                                   2; // Pixel Format is R5_G6_B5 = 2 bytes per pixel

alignas(4) std::uint8_t image_buffer[IMAGE_SIZE];

uint16_t fps = 0;


struct MiniCanvas : public lv_canvas_t
{
   std::uint8_t* canvasBuffer;
};

class CanvasObject
{
   protected:
   lv_area_t coords;

   public:
   virtual void tick() = 0;
   virtual void draw(MiniCanvas* canvas) = 0;

   bool inBounds(MiniCanvas* canvas)
   {
      return (this->coords.x1 >= canvas->img.obj.coords.x1) || (this->coords.y1 >= canvas->img.obj.coords.y1) ||
             (this->coords.x2 <= canvas->img.obj.coords.x2) || (this->coords.y2 <= canvas->img.obj.coords.y2);
   }
};

class Image final : public CanvasObject
{
   private:
   lv_fs_file_t* file;

   public:
   Image(lv_area_t coords, lv_fs_file_t* image_file) : file(image_file)
   {
      this->coords = coords;
   }

   void tick() override
   {
   }

   void draw(MiniCanvas* canvas) override
   {
      if (!inBounds(canvas))
      {
         LOG_INF("Image is not in bounds of the canvas, skipping...");
         // return;
      }

      // Check if this image is aligned to the canvas (no x offset and the widths are the same).
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
         lv_fs_seek(file, filePos, LV_FS_SEEK_SET);

         std::uint32_t read_bytes = 0;
         lv_fs_read(file, &canvas->canvasBuffer[0], croppedSize, &read_bytes);
         if (read_bytes != croppedSize)
         {
            LOG_ERR("Failed to read entire selection, only read %u bytes", read_bytes);
         }
         LOG_INF("Finished reading");
      }
      else
      {
         // Arbitrary image position is not currently supported
         LOG_INF("Image is not aligned with the canvas, skipping...");
         return;
      }
   }
};

class RenderEngine
{
   private:
   lv_draw_label_dsc_t fps_label_dsc;
   std::vector<std::unique_ptr<CanvasObject>> canvasElements;
   lv_coord_t fpsX = 260;
   lv_coord_t fpsY = 160;

   public:
   RenderEngine() : canvasElements{}
   {
      lv_draw_label_dsc_init(&fps_label_dsc);
      fps_label_dsc.font = &lv_font_unscii_8;
      fps_label_dsc.color = lv_color_white();
   }

   void addCanvasElement(std::unique_ptr<CanvasObject> object)
   {
      canvasElements.emplace_back(std::move(object));
   }

   void update()
   {
   }

   void draw(MiniCanvas* canvas)
   {
      LOG_INF("Drawing canvas");

      for (auto& object : canvasElements)
      {
         LOG_INF("Drawing canvas object");
         object->draw(canvas);
      }

      char buffer[15];
      snprintk(buffer, 15, "%02d FPS\0", fps);
      lv_canvas_draw_text(reinterpret_cast<lv_obj_t*>(canvas), fpsX - canvas->img.obj.coords.x1,
                          fpsY - canvas->img.obj.coords.y1, 1000, &fps_label_dsc, buffer);

      LOG_INF("Finished drawing canvas");
   }
};

static lv_fs_res_t my_lvgl_close(struct _lv_fs_drv_t* drv, void* file)
{
   int err;

   err = fs_close((struct fs_file_t*)file);
   LV_MEM_CUSTOM_FREE(file);
   return err;
}

// Static screen parameters
// Unfortunately it's not possible to pull the screen capabilities at compile time.
// In order to allocate a fixed size for our image buffers we need to hard code these values.
static constexpr auto SCREEN_WIDTH = DT_PROP(DT_CHOSEN(zephyr_display), width);
static constexpr auto SCREEN_HEIGHT = DT_PROP(DT_CHOSEN(zephyr_display), height);
uint32_t last_frame_time = 0;

#define NUM_FRAMES 11

K_SEM_DEFINE(readSema1, 1, 1);
K_SEM_DEFINE(readSema2, 1, 1);
K_SEM_DEFINE(flushSema1, 0, 1);
K_SEM_DEFINE(flushSema2, 0, 1);

class ScreenManager
{
   public:
   ScreenManager(std::uint8_t* image_buffer)
   {
      renderEngine = make_unique<RenderEngine>();

      // Intialize our MiniCanvases
      lv_canvas_t* lvCanvas = reinterpret_cast<lv_canvas_t*>(lv_canvas_create(lv_scr_act()));
      std::memcpy(&miniCanvas1, lvCanvas, sizeof(lv_canvas_t));
      miniCanvas1.canvasBuffer = image_buffer;
      lv_canvas_set_buffer(reinterpret_cast<lv_obj_t*>(&miniCanvas1), miniCanvas1.canvasBuffer, SCREEN_WIDTH,
                           SCREEN_HEIGHT / 2, LV_IMG_CF_TRUE_COLOR);
      miniCanvas1.img.obj.coords = lv_area_t{0, 0, 319, 85};

      lvCanvas = reinterpret_cast<lv_canvas_t*>(lv_canvas_create(lv_scr_act()));
      std::memcpy(&miniCanvas2, lvCanvas, sizeof(lv_canvas_t));
      miniCanvas2.canvasBuffer = &image_buffer[SCREEN_WIDTH * (SCREEN_HEIGHT / 2) * 2];
      lv_canvas_set_buffer(reinterpret_cast<lv_obj_t*>(&miniCanvas2), miniCanvas2.canvasBuffer, SCREEN_WIDTH,
                           SCREEN_HEIGHT / 2, LV_IMG_CF_TRUE_COLOR);
      miniCanvas2.img.obj.coords = lv_area_t{0, 86, 319, 171};

      // Initialize display descriptor
      display_desc.buf_size = IMAGE_SIZE / 2;
      display_desc.width = 320;
      display_desc.height = 86;
      display_desc.pitch = 0;
   }

   void tick()
   {
      renderEngine->update();
   }

   void draw(MiniCanvas* canvas)
   {
      renderEngine->draw(canvas);
   }

   void flush(MiniCanvas* canvas)
   {
      LOG_INF("Flushing!");
      display_write(display_dev, canvas->img.obj.coords.x1, canvas->img.obj.coords.y1, &display_desc,
                    canvas->canvasBuffer);
      LOG_INF("Flushing complete!");
   }

   void addImage(lv_fs_file_t* file)
   {
      renderEngine->addCanvasElement(make_unique<Image>(lv_area_t{0, 0, 319, 171}, file));
   }

   void loop()
   {
      lv_fs_file_t file;

      addImage(&file);

      while (1)
      {
         tick();
      load:
         uint32_t frame_time = k_cycle_get_32();
         fps = 1000 / k_cyc_to_ms_floor32(frame_time - last_frame_time);
         LOG_INF("Current FPS = %d, Elapsed time = %d", fps, k_cyc_to_ms_floor32(frame_time - last_frame_time));
         last_frame_time = frame_time;
         char buffer[50];
         static int count = 0;
         if (count >= NUM_FRAMES)
         {
            count %= NUM_FRAMES;
         }
         snprintk(buffer, 50, "/NAND:/frame_%d.bin", count);
         LOG_INF("Loading frame %s", buffer);
         lv_res_t res = lv_fs_open(&file, buffer, LV_FS_MODE_RD);
         if (res != LV_FS_RES_OK)
         {
            LOG_ERR("File %s failed to open", buffer);
            count = 0;
            goto load;
         }
         count++;
         file.drv->close_cb = my_lvgl_close;

         k_sem_take(&readSema1, K_FOREVER);
         draw(&miniCanvas1);
         k_sem_give(&flushSema1);
         k_sem_take(&readSema2, K_FOREVER);
         draw(&miniCanvas2);
         k_sem_give(&flushSema2);

         // The LVGL version that ZMK uses has a memory leak on file close.
         lv_fs_close(&file);
      }
   }

   void flushLoop()
   {
      while (1)
      {
         k_sem_take(&flushSema1, K_FOREVER);
         flush(&miniCanvas1);
         k_sem_give(&readSema1);
         k_sem_take(&flushSema2, K_FOREVER);
         flush(&miniCanvas2);
         k_sem_give(&readSema2);
      }
   }

   private:
   static constexpr auto display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   std::unique_ptr<RenderEngine> renderEngine;
   struct display_buffer_descriptor display_desc;
   MiniCanvas miniCanvas1;
   MiniCanvas miniCanvas2;
};

ScreenManager* screen = nullptr;

K_SEM_DEFINE(flushStartSema, 0, 1);

int display_thread(void)
{

   const struct device* display_dev;

   k_sleep(K_MSEC(1000));

   display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   if (!device_is_ready(display_dev))
   {
      LOG_ERR("Device not ready, aborting test");
      return 0;
   }

   k_sleep(K_MSEC(1000)); // let the flash disk settle

   display_blanking_off(display_dev);

   LOG_PRINTK("Display Width: %d\nDisplay Height : %d\nDisplay Image Size : % d\n", SCREEN_WIDTH, SCREEN_HEIGHT,
              IMAGE_SIZE);

   screen = new ScreenManager(image_buffer);
   k_sem_give(&flushStartSema);
   screen->loop();
}

K_THREAD_DEFINE(dsp_thread, 4096, display_thread, NULL, NULL, NULL, 2, 0, 0);

void flush_thread()
{
   k_sem_take(&flushStartSema, K_FOREVER);
   screen->flushLoop();
}

K_THREAD_DEFINE(flushing_thread, 2048, flush_thread, NULL, NULL, NULL, 2, 0, 0);
