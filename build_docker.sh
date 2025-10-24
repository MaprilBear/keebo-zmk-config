pushd ../zmk/app

west build -b seeeduino_xiao_ble -p -S cdc-acm-console -- \
 -DSHIELD=keypad \
 -DZMK_CONFIG=/workspaces/zmk-config \
 -DZMK_EXTRA_MODULES="/workspaces/zmk-config/module/display;/workspaces/zmk-config/module/usb_mass" \
 -DEXTRA_CONF_FILE="/workspaces/zmk-config/module/display/prj.conf;/workspaces/zmk-config/module/usb_mass/prj.conf"