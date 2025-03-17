/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <malloc.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(display_app, CONFIG_ZMK_LOG_LEVEL);

#define LV_FS_FATFS_LETTER 'N'

#define DISPLAY_DEVICE DT_CHOSEN(zephyr_display)
const struct device *const display_dev = DEVICE_DT_GET(DISPLAY_DEVICE);

#define IMAGE_SPLITS 2
#define IMAGE_SIZE (320 * 172 * 2)
#define REFRESH_RATE 30
#define REFRESH_PERIOD (K_MSEC(1000 / REFRESH_RATE))

__attribute__ ((aligned (4))) uint8_t image_buffer1[IMAGE_SIZE / 2];
__attribute__ ((aligned (4))) uint8_t image_buffer2[IMAGE_SIZE / 2];

K_SEM_DEFINE(display_sema, 0, 1);
K_SEM_DEFINE(read1_sema, 1, 1);
K_SEM_DEFINE(read2_sema, 1, 1);
K_SEM_DEFINE(flush1_sema, 0, 1);
K_SEM_DEFINE(flush2_sema, 0, 1);


void timer_thread(){
	k_sem_give(&display_sema);
}

K_TIMER_DEFINE(display_timer, timer_thread, NULL);

lv_obj_t * canvas1;
lv_obj_t * canvas2;
lv_draw_rect_dsc_t rect_dsc;
lv_draw_rect_dsc_t fps_rect_dsc;
lv_draw_label_dsc_t label_dsc;
lv_draw_label_dsc_t fps_label_dsc;
uint8_t colors[3];

int16_t draw_pos = 0;

void canvas_init(){
	canvas1 = lv_canvas_create(lv_scr_act());
	canvas2 = lv_canvas_create(lv_scr_act());

	lv_canvas_set_buffer(canvas1, &image_buffer1, 320, 172 / 2, LV_IMG_CF_TRUE_COLOR);
	lv_canvas_set_buffer(canvas2, &image_buffer2, 320, 172 / 2, LV_IMG_CF_TRUE_COLOR);

	lv_draw_rect_dsc_init(&rect_dsc);
	rect_dsc.radius = 10;
	rect_dsc.bg_opa = LV_OPA_80;
	//rect_dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
	//rect_dsc.bg_grad.stops[0].color = lv_palette_main(LV_PALETTE_RED);
	//rect_dsc.bg_grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
	rect_dsc.border_width = 2;
	rect_dsc.border_opa = LV_OPA_90;
	rect_dsc.border_color = lv_color_black();
	rect_dsc.shadow_width = 5;
	rect_dsc.shadow_ofs_x = 5;
	rect_dsc.shadow_ofs_y = 5;

	lv_draw_rect_dsc_init(&fps_rect_dsc);
	//rect_dsc.radius = 0;
	rect_dsc.bg_opa = LV_OPA_50;
	fps_rect_dsc.bg_color = lv_color_black();
	//rect_dsc.bg_grad.dir = LV_GRAD_DIR_HOR;
	//rect_dsc.bg_grad.stops[0].color = lv_palette_main(LV_PALETTE_RED);
	//rect_dsc.bg_grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
	fps_rect_dsc.border_width = 1;
	fps_rect_dsc.border_opa = LV_OPA_90;
	fps_rect_dsc.border_color = lv_color_white();

	lv_draw_label_dsc_init(&label_dsc);
	lv_draw_label_dsc_init(&fps_label_dsc);
	fps_label_dsc.font = &lv_font_unscii_8;
	fps_label_dsc.color = lv_color_white();
}

void canvas_update(){
	sys_rand_get(colors, 3);
	label_dsc.color = lv_color_make(colors[0], colors[1], colors[2]);

	draw_pos = (draw_pos + 2) % 172;
}

uint16_t fps = 0;

void canvas_draw(lv_obj_t* canvas, lv_coord_t y){
	
	LV_LOG_INFO("Drawing canvas");
	//lv_canvas_draw_rect(canvas, 20, draw_pos - y, 150, 70, &rect_dsc);
	//lv_canvas_draw_text(canvas, 40, draw_pos - y, 150, &label_dsc, "GAY");

	//lv_canvas_draw_rect(canvas, 0, 0 - y, 50, 50, &fps_rect_dsc);
	char buffer[10];
	snprintk(buffer, 10, "%02d FPS", fps % 100);
	lv_canvas_draw_text(canvas, 260, 160 - y, 1000, &fps_label_dsc, buffer);
	LV_LOG_INFO("Finished drawing canvas");
	
}

uint32_t last_frame_time = 0;

void read_thread(){
	lv_fs_file_t file;
	k_timer_start(&display_timer, REFRESH_PERIOD, REFRESH_PERIOD);

	struct display_buffer_descriptor display_desc;
	display_desc.buf_size = IMAGE_SIZE / 2;
	display_desc.width = 320;
	display_desc.height = 86;
	display_desc.pitch = 0;

	k_sleep(K_MSEC(1000));

	while(1){
		//k_sem_take(&read1_sema, K_FOREVER);
		canvas_update(0);
		load:
		k_sem_take(&display_sema, K_FOREVER);
		uint32_t frame_time = k_cycle_get_32();
		fps = 1000 / k_cyc_to_ms_floor32(frame_time - last_frame_time);
		LV_LOG_INFO("Current FPS = %d, Elapsed time = %d", fps, k_cyc_to_ms_floor32(frame_time - last_frame_time));
		last_frame_time = frame_time;
		char buffer[50];
		static int count = 0;
		snprintk(buffer, 50, "/NAND:/frame_%d.bin\0", count);
		LV_LOG_INFO("Loading frame %s", buffer);
		lv_res_t res = lv_fs_open(&file, buffer, LV_FS_MODE_RD);
		if (res != LV_FS_RES_OK){
			LV_LOG_ERROR("File %s failed to open", buffer);
			count = 0;
			goto load;
		}
		count++;

		LV_LOG_INFO("Reading 1st half into buffer");
		int read_bytes;
		lv_fs_seek(&file, 4, LV_FS_SEEK_SET); // skip over random junk
		lv_fs_read(&file, image_buffer1, IMAGE_SIZE / 2, &read_bytes);
		if (read_bytes != IMAGE_SIZE / 2){
			LV_LOG_ERROR("Failed to read entire selection, only read %d bytes", read_bytes);
		}
		LV_LOG_INFO("Finished reading 1st half");

		canvas_draw(canvas1, 0);

		//display_write(display_dev, 0, 0, &display_desc, image_buffer1);
		
		k_sem_give(&flush1_sema);
		k_sem_take(&read2_sema, K_FOREVER);

		LV_LOG_INFO("Reading 2nd half into buffer");
		lv_fs_seek(&file, 4 + (IMAGE_SIZE / 2), LV_FS_SEEK_SET); // skip over random junk
		lv_fs_read(&file, image_buffer2, IMAGE_SIZE / 2, &read_bytes);
		if (read_bytes != IMAGE_SIZE / 2){
			LV_LOG_ERROR("Failed to read entire selection, only read %d bytes", read_bytes);
		}

		LV_LOG_INFO("Finished reading 2nd half");

		lv_fs_close(&file);

		//display_write(display_dev, 0, 86, &display_desc, image_buffer2);

		canvas_draw(canvas2, (lv_coord_t) 86);

		k_sem_give(&flush2_sema);
	}
}

void flush_thread(){
	struct display_buffer_descriptor display_desc;
	display_desc.buf_size = IMAGE_SIZE / 2;
	display_desc.width = 320;
	display_desc.height = 86;
	display_desc.pitch = 0;

	while (1){
		k_sem_take(&flush1_sema, K_FOREVER);

		LV_LOG_INFO("Flushing 1st half");
		display_write(display_dev, 0, 0, &display_desc, image_buffer1);
		LV_LOG_INFO("Finished flushing 1st half");

		k_sem_give(&read1_sema);
		k_sem_take(&flush2_sema, K_FOREVER);

		LV_LOG_INFO("Flushing 2nd half");
		display_write(display_dev, 0, 86, &display_desc, image_buffer2);
		LV_LOG_INFO("Finished flushing 2nd half");

		k_sem_give(&read2_sema);
	}
}

K_THREAD_DEFINE(reading_thread, 2048,
	read_thread, NULL, NULL, NULL,
	2, 0, 0);

K_THREAD_DEFINE(flushing_thread, 1024,
		flush_thread, NULL, NULL, NULL,
		2, 0, 0);


void my_log_cb(const char* buf){
	printk("%s\n", buf);
}

lv_obj_t * custom_anim_img;

void next_frame(){

	LV_LOG_INFO("Loading next frame");
	char buffer[50];
	static int count = 0;
	snprintk(buffer, 50, "/NAND:/frame_%d.bin\0", count);
	LV_LOG_INFO("Loading frame %s", buffer);

	// load entire file into buffer
	lv_fs_file_t f;
	lv_res_t res = lv_fs_open(&f, buffer, LV_FS_MODE_RD);
	if (res != LV_FS_RES_OK){
		LV_LOG_ERROR("File %s failed to open", buffer);
		if (count > 0){
			count = 0;
			next_frame();
			return;
		} else {
			return;
		}
	}
	count++;
	int read_bytes;
	lv_fs_seek(&f, 4, LV_FS_SEEK_SET); // skip over random junk
	lv_fs_read(&f, image_buffer1, IMAGE_SIZE, &read_bytes);
	if (read_bytes != IMAGE_SIZE){
		LV_LOG_ERROR("Failed to read entire file, only read %d bytes", read_bytes);
	}

	LV_LOG_INFO("read %d bytes", read_bytes);

	lv_fs_close(&f);

	LV_LOG_INFO("Finished loading frame");

	// draw it our selves :)
	struct display_buffer_descriptor display_desc;
	display_desc.buf_size = 110080;
	display_desc.width = 320;
	display_desc.height = 172;
	display_desc.pitch = 0;
	LV_LOG_INFO("Flushing frame to display");
	//display_write(display_dev, 0, 0, &display_desc, image_buffer);
	LV_LOG_INFO("Finished flushing");

	///lv_img_cache_invalidate_src(custom_anim_img);

	//lv_img_set_src(custom_anim_img, &custom_image);
	

	//k_sleep(K_MSEC(1000));
}


int display_thread(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	//lv_log_register_print_cb(my_log_cb);

	k_sleep(K_MSEC(1000)); // let the flash disk settle

	display_blanking_off(display_dev);

	canvas_init();

	k_sleep(K_MSEC(1000)); // let the flash disk settle

	k_sem_give(&display_sema);

	return;

	while (1){
		next_frame();

		
	}
	return;

	
	
	while (1) {
		LV_LOG_INFO("Ticking LVGL task handler");
		lv_task_handler();
		LV_LOG_INFO("Finished Ticking");
		next_frame();
	}
}

K_THREAD_DEFINE(dsp_thread, 2048,
                display_thread, NULL, NULL, NULL,
                2, 0, 0);
