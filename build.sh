pushd ../zmk/app

west build -b nrf5340dk_nrf5340_cpuapp --pristine -S zmk-usb-logging --\
 -DSHIELD=keebo -DZMK_CONFIG="/home/april/Repositories/Keebo/sw/zmk-config" \
 -DZMK_EXTRA_MODULES="/home/april/Repositories/Keebo/sw/zmk-config/module/kscan-custom"