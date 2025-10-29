cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl)

zephyr_include_directories(include)
target_include_directories(app PRIVATE include)

FILE(GLOB app_sources src/*.c src/*.cpp)
target_sources(app PRIVATE ${app_sources})

