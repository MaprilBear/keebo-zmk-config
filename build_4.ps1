Push-Location ../zmk4_1/app

west build -b xiao_ble $flags -S cdc-acm-console -- `
 -DSHIELD=keypad `
 -DZMK_CONFIG="C:\Users\April\Documents\Repos\Keebo\sw\zmk-config" `
 -DZMK_EXTRA_MODULES="C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\display_app;C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\usb_mass" `
 -DEXTRA_CONF_FILE="C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\display_app\prj.conf; C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\usb_mass\prj.conf" `
 -DEXTRA_DTC_OVERLAY_FILE="C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\display_app\boards\xiao_ble.overlay;C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\usb_mass\boards\xiao_ble.overlay"

Pop-Location