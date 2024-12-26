cmake_minimum_required(VERSION 3.20.0)

list(APPEND CONF_FILES prj.conf)
list(APPEND dts_files boards/nrf5340dk_cpuapp.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})