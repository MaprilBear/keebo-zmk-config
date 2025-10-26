# docker exec -w /workspaces/zmk/app zmk-dev  west build -b seeeduino_xiao_ble -p -S cdc-acm-console -- `
#  -DSHIELD=keypad -DZMK_CONFIG=/workspaces/zmk-config `
#  -DZMK_EXTRA_MODULES="/workspaces/zmk-config/module/display;/workspaces/zmk-config/module/usb_mass" `
#  -DEXTRA_CONF_FILE="/workspaces/zmk-config/module/display/prj.conf;/workspaces/zmk-config/module/usb_mass/prj.conf"
param([string]$flags="")

Push-Location ../zmk/app

west build -b seeeduino_xiao_ble $flags -S cdc-acm-console -- `
 -DSHIELD=keypad -DZMK_CONFIG="C:\Users\April\Documents\Repos\Keebo\sw\zmk-config" `
 -DZMK_EXTRA_MODULES="C:\Users\April\Documents\Repos\Keebo\sw\zmk-config\module\display;C:\Users\April\Documents\Repos\Keebo\sw\zmk-config\module\usb_mass" `
 -DEXTRA_CONF_FILE="C:\Users\April\Documents\Repos\Keebo\sw\zmk-config\module\display\prj.conf; C:\Users\April\Documents\Repos\Keebo\sw\zmk-config\module\usb_mass\prj.conf"

 Pop-Location