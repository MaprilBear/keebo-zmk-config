pushd ../zmk/app
 
 west build -b seeeduino_xiao_ble $1 -S cdc-acm-console -- \
 -DSHIELD=keypad \
 -DZMK_CONFIG=/home/april/Repositories/Keebo/sw/zmk-config \
 -DZMK_EXTRA_MODULES="/home/april/Repositories/Keebo/sw/zmk-config/module/display;/home/april/Repositories/Keebo/sw/zmk-config/module/usb_mass" \
 -DEXTRA_CONF_FILE="/home/april/Repositories/Keebo/sw/zmk-config/module/display/prj.conf;/home/april/Repositories/Keebo/sw/zmk-config/module/usb_mass/prj.conf"
 #-DEXTRA_DTC_OVERLAY_FILE=/home/april/Repositories/Keebo/sw/zmk-config/module/display/boards/nrf5340dk_nrf5340_cpuapp.overlay 