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
LOG_MODULE_REGISTER(display_app, CONFIG_ZMK_LOG_LEVEL);

#define LV_FS_FATFS_LETTER 'N'

//LV_IMG_DECLARE(caitlyn);
//LV_IMG_DECLARE(subaru);
//LV_IMG_DECLARE(subaru_eat);
//LV_IMG_DECLARE(promare);
// LV_IMG_DECLARE(frame_0);
// LV_IMG_DECLARE(frame_1);
// LV_IMG_DECLARE(frame_2);
// LV_IMG_DECLARE(frame_3);
// LV_IMG_DECLARE(frame_4);
// LV_IMG_DECLARE(frame_5);
// LV_IMG_DECLARE(frame_6);
// LV_IMG_DECLARE(frame_7);
// LV_IMG_DECLARE(frame_8);
// LV_IMG_DECLARE(frame_9);
// LV_IMG_DECLARE(frame_10);
// LV_IMG_DECLARE(frame_11);
// LV_IMG_DECLARE(frame_12);
// LV_IMG_DECLARE(frame_13);
// LV_IMG_DECLARE(frame_14);
// LV_IMG_DECLARE(frame_15);
// LV_IMG_DECLARE(frame_16);
// LV_IMG_DECLARE(frame_17);
// LV_IMG_DECLARE(frame_18);
// LV_IMG_DECLARE(frame_19);
// LV_IMG_DECLARE(frame_20);
// LV_IMG_DECLARE(frame_21);
//LV_IMG_DECLARE(frame_22);
//LV_IMG_DECLARE(frame_23);
//LV_IMG_DECLARE(frame_24);
// LV_IMG_DECLARE(frame_40);
// LV_IMG_DECLARE(frame_41);
// LV_IMG_DECLARE(frame_42);
// LV_IMG_DECLARE(frame_43);
// LV_IMG_DECLARE(frame_44);
// LV_IMG_DECLARE(frame_45);
// LV_IMG_DECLARE(frame_46);
// LV_IMG_DECLARE(frame_47);
// LV_IMG_DECLARE(frame_48);
// LV_IMG_DECLARE(frame_49);
// LV_IMG_DECLARE(frame_50);
// LV_IMG_DECLARE(frame_51);
// LV_IMG_DECLARE(frame_52);
// LV_IMG_DECLARE(frame_53);
// LV_IMG_DECLARE(frame_54);
// LV_IMG_DECLARE(frame_55);
//LV_IMG_DECLARE(frame_56);
//LV_IMG_DECLARE(frame_57);

LV_IMG_DECLARE(miku_miku);



void my_log_cb(const char* buf){
	printk("%s\n", buf);
}

#define NUM_FRAMES 31

lv_obj_t * custom_anim_img;

void next_frame(){
	//LV_LOG_INFO("Loading next frame");
	char buffer[50];
	static int count = 0;
	
	snprintk(buffer, 50, "/NAND:/frame_%d.bin", count++ % NUM_FRAMES);
	LV_LOG_INFO("Loading frame %s", buffer);
	lv_img_set_src(custom_anim_img, buffer);
	LV_LOG_INFO("Finished loading frame");
}


int display_thread(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	lv_log_register_print_cb(my_log_cb);

	k_sleep(K_MSEC(2000));

	// register file system
	//filesys_init();

	// load all frames 

	// lv_obj_t * test = lv_img_create(lv_scr_act());
	// lv_img_set_src(test, "/NAND:/MIKU.bin");

	//k_sleep(K_MSEC(2000));

	//lv_example_bar_3();
	// display_blanking_off(display_dev);
	// while (1) {
	// 	LOG_DBG("tick!");
	// 	lv_task_handler();
	// 	//k_sleep(K_MSEC(100));
	// }

	
	

	//lv_obj_t* icon = lv_img_create(lv_scr_act());
	
	//lv_obj_t* icon2 = lv_img_create(lv_scr_act());
	//lv_obj_t* gif = lv_gif_create(lv_scr_act());
	//lv_img_set_src(icon, &frame_0);
	//lv_img_set_src(icon2, &caitlyn);

	// lv_gif_set_src(gif, &promare);
	// lv_img_set_zoom(gif, 600);
	// lv_obj_align(gif, LV_ALIGN_CENTER, 0, 0);
	// lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);
	//lv_task_handler();


	// lv_obj_t * animimg0 = lv_animimg_create(lv_scr_act());
	// lv_obj_center(animimg0);
	// lv_animimg_set_src(animimg0, (const void **) anim_imgs, NUM_FRAMES);
	// lv_animimg_set_duration(animimg0, 50000);
	// lv_animimg_set_repeat_count(animimg0, LV_ANIM_REPEAT_INFINITE);
	// lv_animimg_start(animimg0);

	custom_anim_img = lv_img_create(lv_scr_act());
	next_frame();

	display_blanking_off(display_dev);
	

	//bool temp = false;

	

	while (1) {
		//LOG_DBG("tick!");
		//lv_img_set_zoom(icon, 120);
		LV_LOG_INFO("Ticking LVGL task handler");
		lv_task_handler();
		LV_LOG_INFO("Finished Ticking");
		next_frame();
		//k_yield();
		//k_sleep(K_MSEC(1));
		
	}
}



K_THREAD_DEFINE(dsp_thread, 2048,
                display_thread, NULL, NULL, NULL,
                2, 0, 0);
