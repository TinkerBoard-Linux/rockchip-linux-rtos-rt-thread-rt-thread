#! /bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

usage() {
	echo "usage: ./mkimage.sh [partition_setting]"
}

CUR_DIR=$(pwd)
TOOLS=$CUR_DIR/../tools
IMAGE=$(pwd)/Image

../tools/boot_merger ./Image/rk2118_ddr.ini
../tools/boot_merger ./Image/rk2118_no_ddr.ini
rm $IMAGE/rk2118_db_loader.bin

rm -rf $CUR_DIR/rtthread.bin $IMAGE/rtthread.img $CUR_DIR/rtthread_bak.bin $IMAGE/rtthread_bak.img $IMAGE/Firmware*

$RTT_EXEC_PATH/arm-none-eabi-objcopy -O binary rtthread.elf rtthread.bin
./align_bin_size.sh $CUR_DIR/rtthread.bin
cp -r $CUR_DIR/rtthread.bin $IMAGE/rtthread.img
$TOOLS/resource_header_tool pack --json $IMAGE/config.json $IMAGE/rtthread.img > /dev/null

if [ -f "rtthread_bak.elf" ]; then
    $RTT_EXEC_PATH/arm-none-eabi-objcopy -O binary rtthread_bak.elf rtthread_bak.bin
    cp -r $CUR_DIR/rtthread_bak.bin $IMAGE/rtthread_bak.img
    $TOOLS/resource_header_tool pack --json $IMAGE/config.json $IMAGE/rtthread_bak.img > /dev/null
fi

echo 'Image: rtthread image is ready'

if [ ! -n "$1" ] ;then
    ./check_xip_addr.py board/common/setting.ini
    if [ ! $? -eq 0 ] ;then
        echo "mkimage fail"
        exit
    fi
    $TOOLS/firmware_merger/firmware_merger -p board/common/setting.ini $IMAGE/
else
    ./check_xip_addr.py $1
    if [ ! $? -eq 0 ] ;then
        echo "mkimage fail"
        exit
    fi 
    $TOOLS/firmware_merger/firmware_merger -p $1 $IMAGE/
fi
