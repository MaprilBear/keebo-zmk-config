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

class CanvasObject
{
   protected:
   lv_area_t coords;

   public:
   virtual void tick() = 0;
   virtual void draw(lv_canvas_t* canvas, std::uint8_t* canvasData) = 0;

   bool inBounds(lv_canvas_t* canvas)
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

   void draw(lv_canvas_t* canvas, std::uint8_t* canvasData) override
   {
      if (!inBounds(canvas))
      {
         LV_LOG_INFO("Image is not in bounds of the canvas, skipping...");
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
         auto filePos = 4; // Skip over image header from LVGL v8 conversion tool
         auto canvasPos = 0;
         auto delta = this->coords.y1 - canvasCoords.y1;
         if (delta < 0)
         {
            // The image is positioned above the start of the canvas viewable area
            // We need to seek futher into the image data to begin reading into canvas[0][0] at the right place.
            filePos += delta * imageWidth;
         }
         else if (delta > 0)
         {
            // The image is positioned below the start of the canvas vierable area
            // We need to begin reading data into an offset in the canvas, not the file.
            canvasPos += delta * imageWidth;
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
            visibleRows = imageHeight;
         }

         std::uint32_t croppedSize = visibleRows * imageWidth * 2;
         lv_fs_seek(file, filePos, LV_FS_SEEK_SET);

         LOG_PRINTK("Image Pos = {%d, %d} {%d, %d}\n", this->coords.x1, this->coords.y1, this->coords.x2,
                    this->coords.y2);
         LOG_PRINTK("Canvas Pos = {%d, %d} {%d, %d}\n", canvasCoords.x1, canvasCoords.y1, canvasCoords.x2,
                    canvasCoords.y2);
         LOG_PRINTK("Visible rows = %d\n", visibleRows);
         LOG_PRINTK("Cropped size = %d\n", croppedSize);
         LOG_PRINTK("File start pos = %d\n", filePos);
         LOG_PRINTK("Canvas start pos = %d\n", canvasPos);

         std::uint32_t read_bytes = 0;
         lv_fs_read(file, &canvasData[canvasPos], croppedSize, &read_bytes);
         if (read_bytes != croppedSize)
         {
            LV_LOG_ERROR("Failed to read entire selection, only read %d bytes", read_bytes);
         }
         LV_LOG_INFO("Finished reading 1st half");
      }
      else
      {
         // Arbitrary image position is not currently supported
         LV_LOG_INFO("Image is not aligned with the canvas, skipping...");
         return;
      }
   }
};

class RenderEngine
{
   private:
   std::uint16_t canvasWidth;
   std::uint16_t canvasHeight;
   lv_canvas_t* canvas;
   lv_draw_rect_dsc_t rect_dsc;
   lv_draw_rect_dsc_t fps_rect_dsc;
   lv_draw_label_dsc_t label_dsc;
   lv_draw_label_dsc_t fps_label_dsc;
   uint8_t colors[3];
   int16_t draw_pos = 0;
   std::vector<std::unique_ptr<CanvasObject>> canvasElements;
   std::uint8_t* image_buf;

   public:
   RenderEngine(std::uint16_t width, std::uint16_t height, std::uint8_t* image_buffer)
       : canvasWidth(width), canvasHeight(height), canvasElements{}, image_buf(image_buffer)
   {
      canvas = reinterpret_cast<lv_canvas_t*>(lv_canvas_create(lv_scr_act()));
      lv_canvas_set_buffer(reinterpret_cast<lv_obj_t*>(canvas), image_buffer, width, height, LV_IMG_CF_TRUE_COLOR);
      canvas->img.obj.coords = lv_area_t{0, 0, 319, 171};
      init();
   }

   void addCanvasElement(std::unique_ptr<CanvasObject> object)
   {
      canvasElements.emplace_back(std::move(object));
   }

   void init()
   {
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.radius = 10;
      rect_dsc.bg_opa = LV_OPA_80;
      // rect_dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
      // rect_dsc.bg_grad.stops[0].color = lv_palette_main(LV_PALETTE_RED);
      // rect_dsc.bg_grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
      rect_dsc.border_width = 2;
      rect_dsc.border_opa = LV_OPA_90;
      rect_dsc.border_color = lv_color_black();
      rect_dsc.shadow_width = 5;
      rect_dsc.shadow_ofs_x = 5;
      rect_dsc.shadow_ofs_y = 5;

      lv_draw_rect_dsc_init(&fps_rect_dsc);
      // rect_dsc.radius = 0;
      rect_dsc.bg_opa = LV_OPA_50;
      fps_rect_dsc.bg_color = lv_color_black();
      // rect_dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
      // rect_dsc.bg_grad.stops[0].color = lv_palette_main(LV_PALETTE_RED);
      // rect_dsc.bg_grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
      fps_rect_dsc.border_width = 1;
      fps_rect_dsc.border_opa = LV_OPA_90;
      fps_rect_dsc.border_color = lv_color_white();

      lv_draw_label_dsc_init(&label_dsc);
      lv_draw_label_dsc_init(&fps_label_dsc);
      fps_label_dsc.font = &lv_font_unscii_8;
      fps_label_dsc.color = lv_color_white();
   }

   void update()
   {
      sys_rand_get(colors, 3);
      label_dsc.color = lv_color_make(colors[0], colors[1], colors[2]);

      draw_pos = (draw_pos + 2) % 172;
   }

   void draw()
   {
      LV_LOG_INFO("Drawing canvas");
      char buffer[10];
      snprintk(buffer, 10, "%02d FPS", fps);
      lv_canvas_draw_text(reinterpret_cast<lv_obj_t*>(canvas), 260, 160, 1000, &fps_label_dsc, buffer);

      for (auto& object : canvasElements)
      {
         LV_LOG_INFO("Drawing canvas object");
         object->draw(canvas, image_buf);
      }

      LV_LOG_INFO("Finished drawing canvas");
   }
};

// Static screen parameters
// Unfortunately it's not possible to pull the screen capabilities at compile time.
// In order to allocate a fixed size for our image buffers we need to hard code these values.
static constexpr auto SCREEN_WIDTH = DT_PROP(DT_CHOSEN(zephyr_display), width);
static constexpr auto SCREEN_HEIGHT = DT_PROP(DT_CHOSEN(zephyr_display), height);

class ScreenRenderer
{
   public:
   ScreenRenderer(std::uint8_t* image_buffer) : image_buf(image_buffer)
   {
      renderEngine = make_unique<RenderEngine>(SCREEN_WIDTH, SCREEN_HEIGHT, image_buffer);
   }

   void tick()
   {
      renderEngine->update();
   }

   void draw()
   {
      renderEngine->draw();
   }

   void flush()
   {
      display_write(display_dev, 0, 0, &display_desc, image_buf);
   }

   void addImage(lv_fs_file_t* file)
   {
      renderEngine->addCanvasElement(make_unique<Image>(lv_area_t{0, 0, 319, 171}, file));
   }

   private:
   static constexpr auto display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   std::unique_ptr<RenderEngine> renderEngine;
   struct display_buffer_descriptor display_desc{IMAGE_SIZE, SCREEN_WIDTH, SCREEN_HEIGHT, 0};
   std::uint8_t* image_buf;
};

static lv_fs_res_t my_lvgl_close(struct _lv_fs_drv_t* drv, void* file)
{
   int err;

   err = fs_close((struct fs_file_t*)file);
   LV_MEM_CUSTOM_FREE(file);
   return err;
}

uint32_t last_frame_time = 0;

#define NUM_FRAMES 11

int display_thread(void)
{

   const struct device* display_dev;

   display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   if (!device_is_ready(display_dev))
   {
      LOG_ERR("Device not ready, aborting test");
      return 0;
   }

   k_sleep(K_MSEC(1000)); // let the flash disk settle

   display_blanking_off(display_dev);

   k_sleep(K_MSEC(1000)); // let the flash disk settle

   LOG_PRINTK("Display Width: %d\nDisplay Height : %d\nDisplay Image Size : % d\n", SCREEN_WIDTH, SCREEN_HEIGHT,
              IMAGE_SIZE);

   ScreenRenderer screen(image_buffer);

   lv_fs_file_t file;

   struct display_buffer_descriptor display_desc;
   display_desc.buf_size = IMAGE_SIZE;
   display_desc.width = 320;
   display_desc.height = 172;
   display_desc.pitch = 0;

   k_sleep(K_MSEC(1000));

   screen.addImage(&file);

   while (1)
   {
      screen.tick();
   load:
      uint32_t frame_time = k_cycle_get_32();
      fps = 1000 / k_cyc_to_ms_floor32(frame_time - last_frame_time);
      LV_LOG_INFO("Current FPS = %d, Elapsed time = %d", fps, k_cyc_to_ms_floor32(frame_time - last_frame_time));
      last_frame_time = frame_time;
      char buffer[50];
      static int count = 0;
      if (count >= NUM_FRAMES)
      {
         count %= NUM_FRAMES;
      }
      snprintk(buffer, 50, "/NAND:/frame_%d.bin", count);
      LV_LOG_INFO("Loading frame %s", buffer);
      lv_res_t res = lv_fs_open(&file, buffer, LV_FS_MODE_RD);
      if (res != LV_FS_RES_OK)
      {
         LV_LOG_ERROR("File %s failed to open", buffer);
         count = 0;
         goto load;
      }
      count++;
      file.drv->close_cb = my_lvgl_close;

      screen.draw();
      screen.flush();

      // The LVGL version that ZMK uses has a memory leak on file close.
      lv_fs_close(&file);
   }
}

K_THREAD_DEFINE(dsp_thread, 2048, display_thread, NULL, NULL, NULL, 2, 0, 0);
