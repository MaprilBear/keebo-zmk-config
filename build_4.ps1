param([string]$flags="")

Push-Location ../zmk4_1/app

west build $flags -b xiao_ble -S cdc-acm-console -- `
 -DSHIELD=keypad `
 -DZMK_CONFIG="C:\Users\April\Documents\Repos\Keebo\sw\zmk-config" `
 -DZMK_EXTRA_MODULES="C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\display_app;C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\usb_mass" `
 -DEXTRA_CONF_FILE="C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\display_app\prj.conf; C:\Users\April\Documents\Repos\Keebo\sw\zmk-modules\usb_mass\prj.conf"
Pop-Location