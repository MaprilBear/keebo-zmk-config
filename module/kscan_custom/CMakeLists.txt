zephyr_library()

zephyr_library_sources_ifdef(CONFIG_KSCAN_CUSTOM src/kscan_gpio_custom.c)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
