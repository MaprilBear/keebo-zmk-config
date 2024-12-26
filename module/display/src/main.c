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

#include <nrfx_clock.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

int hfclk(const struct device *unused) {
  return nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);
}
SYS_INIT(hfclk, EARLY, 0);

volatile int32_t tmp = 0;

int display_thread(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	//LV_IMG_DECLARE(caitlyn);
	LV_IMG_DECLARE(subaru);
	LV_IMG_DECLARE(subaru_eat);

	lv_obj_t* icon = lv_img_create(lv_scr_act());
	lv_obj_t* gif = lv_gif_create(lv_scr_act());
	lv_img_set_src(icon, &subaru);
	lv_gif_set_src(gif, &subaru_eat);
	lv_img_set_zoom(gif, 400);
	lv_obj_set_pos(gif, 20, 60);
	//lv_img_set_angle(icon, 90);

	lv_task_handler();
	display_blanking_off(display_dev);


	while (1) {
		lv_task_handler();
		k_sleep(K_MSEC(20));
	}
}

K_THREAD_DEFINE(dsp_thread, 4096,
                display_thread, NULL, NULL, NULL,
                1, 0, 0);
