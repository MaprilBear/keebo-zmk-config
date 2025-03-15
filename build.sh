pushd ../zmk/app
 
 west build -b seeeduino_xiao_ble $1 -S cdc-acm-console -- \
 -DSHIELD=keypad \
 -DZMK_CONFIG=/home/april/Repositories/Keebo/sw/zmk-config \
 -DZMK_EXTRA_MODULES="/home/april/Repositories/Keebo/sw/zmk-config/module/kscan_custom;/home/april/Repositories/Keebo/sw/zmk-config/module/usb_mass;/home/april/Repositories/Keebo/sw/zmk-config/module/display" \
 -DEXTRA_CONF_FILE=/home/april/Repositories/Keebo/sw/zmk-config/module/display/prj.conf \
 -DEXTRA_CONF_FILE=/home/april/Repositories/Keebo/sw/zmk-config/module/usb_mass/prj.conf
 #-DEXTRA_DTC_OVERLAY_FILE=/home/april/Repositories/Keebo/sw/zmk-config/module/display/boards/nrf5340dk_nrf5340_cpuapp.overlay 