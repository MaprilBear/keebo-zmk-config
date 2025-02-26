pushd ../zmk/app/build/zephyr

#west flash
sudo mount -t drvfs e: /mnt/e -o uid=$(id -u $USER),gid=$(id -g $USER),metadata
pv zmk.uf2 > /mnt/e/zmk.uf2