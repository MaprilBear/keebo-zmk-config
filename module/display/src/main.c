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
LOG_MODULE_REGISTER(display_app, CONFIG_ZMK_LOG_LEVEL);

void my_log_cb(lv_log_level_t level, const char* buf){
  //LOG_DBG("hello! %s", buf);
	printf(buf);
}

volatile int32_t tmp = 0;

int display_thread(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}
	//lv_example_bar_3();
	// display_blanking_off(display_dev);
	// while (1) {
	// 	LOG_DBG("tick!");
	// 	lv_task_handler();
	// 	//k_sleep(K_MSEC(100));
	// }

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
	LV_IMG_DECLARE(frame_40);
	LV_IMG_DECLARE(frame_41);
	LV_IMG_DECLARE(frame_42);
	LV_IMG_DECLARE(frame_43);
	LV_IMG_DECLARE(frame_44);
	LV_IMG_DECLARE(frame_45);
	LV_IMG_DECLARE(frame_46);
	LV_IMG_DECLARE(frame_47);
	LV_IMG_DECLARE(frame_48);
	LV_IMG_DECLARE(frame_49);
	LV_IMG_DECLARE(frame_50);
	LV_IMG_DECLARE(frame_51);
	LV_IMG_DECLARE(frame_52);
	LV_IMG_DECLARE(frame_53);
	LV_IMG_DECLARE(frame_54);
	LV_IMG_DECLARE(frame_55);
	LV_IMG_DECLARE(frame_56);
	LV_IMG_DECLARE(frame_57);
	

	lv_obj_t* icon = lv_img_create(lv_scr_act());
	
	//lv_obj_t* icon2 = lv_img_create(lv_scr_act());
	//lv_obj_t* gif = lv_gif_create(lv_scr_act());
	//lv_img_set_src(icon, &frame_0);
	//lv_img_set_src(icon2, &caitlyn);

	// lv_gif_set_src(gif, &promare);
	// lv_img_set_zoom(gif, 600);
	// lv_obj_align(gif, LV_ALIGN_CENTER, 0, 0);
	// lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);
	//lv_task_handler();
	display_blanking_off(display_dev);

	//bool temp = false;

	//lv_log_register_print_cb(my_log_cb);

	while (1) {
		//LOG_DBG("tick!");
		switch (tmp++ % 14){
			//case 0: lv_img_set_src(icon, &frame_40); break;
			//case 1: lv_img_set_src(icon, &frame_41); break;
			case 0: lv_img_set_src(icon, &frame_42); break;
			case 1: lv_img_set_src(icon, &frame_43); break;
			case 2: lv_img_set_src(icon, &frame_44); break;
			case 3: lv_img_set_src(icon, &frame_45); break;
			case 4: lv_img_set_src(icon, &frame_46); break;
			case 5: lv_img_set_src(icon, &frame_47); break;
			case 6: lv_img_set_src(icon, &frame_48); break;
			case 7: lv_img_set_src(icon, &frame_49); break;
			case 8: lv_img_set_src(icon, &frame_50); break;
			case 9: lv_img_set_src(icon, &frame_51); break;
			case 10: lv_img_set_src(icon, &frame_52); break;
			case 11: lv_img_set_src(icon, &frame_53); break;
			case 12: lv_img_set_src(icon, &frame_54); break;
			case 13: lv_img_set_src(icon, &frame_55); break;
			// case 0: lv_img_set_src(icon, &frame_0); break;
			// case 1: lv_img_set_src(icon, &frame_1); break;
			// case 2: lv_img_set_src(icon, &frame_2); break;
			// case 3: lv_img_set_src(icon, &frame_3); break;
			// case 4: lv_img_set_src(icon, &frame_4); break;
			// case 5: lv_img_set_src(icon, &frame_5); break;
			// case 6: lv_img_set_src(icon, &frame_6); break;
			// case 7: lv_img_set_src(icon, &frame_7); break;
			// case 8: lv_img_set_src(icon, &frame_8); break;
			// case 9: lv_img_set_src(icon, &frame_9); break;
			// case 10: lv_img_set_src(icon, &frame_10); break;
			// case 11: lv_img_set_src(icon, &frame_11); break;
			// case 12: lv_img_set_src(icon, &frame_12); break;
			// case 13: lv_img_set_src(icon, &frame_13); break;
			// case 14: lv_img_set_src(icon, &frame_14); break;
			// case 15: lv_img_set_src(icon, &frame_15); break;
			// case 16: lv_img_set_src(icon, &frame_16); break;
			// case 17: lv_img_set_src(icon, &frame_17); break;
			// case 18: lv_img_set_src(icon, &frame_18); break;
			// case 19: lv_img_set_src(icon, &frame_19); break;
			// case 20: lv_img_set_src(icon, &frame_20); break;
			// case 21: lv_img_set_src(icon, &frame_21); break;
			//case 22: lv_img_set_src(icon, &frame_22); break;
			//case 23: lv_img_set_src(icon, &frame_23); break;
			//case 24: lv_img_set_src(icon, &frame_24); break;
		}
		//lv_img_set_zoom(icon, 120);
		lv_task_handler();
		k_sleep(K_MSEC(1000/30));
	}
}



K_THREAD_DEFINE(dsp_thread, 2048,
                display_thread, NULL, NULL, NULL,
                1, 0, 0);
