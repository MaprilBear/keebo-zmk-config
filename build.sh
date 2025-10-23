pushd ../zmk/app
 
 west build -b seeeduino_xiao_ble -p -S cdc-acm-console -- \
 -DSHIELD=keypad \
 -DZMK_CONFIG=C:/Users/April/Documents/Repos/Keebo/sw/zmk-config \
 -DZMK_EXTRA_MODULES="C:/Users/April/Documents/Repos/Keebo/sw/zmk-config/module/display;C:/Users/April/Documents/Repos/Keebo/sw/zmk-config/module/usb_mass" \
 -DEXTRA_CONF_FILE="C:/Users/April/Documents/Repos/Keebo/sw/zmk-config/module/display/prj.conf;C:/Users/April/Documents/Repos/Keebo/sw/zmk-config/module/usb_mass/prj.conf"

read -p "Press any key to continue" x