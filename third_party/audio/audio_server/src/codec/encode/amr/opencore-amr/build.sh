#!/bin/sh

#./configure --prefix=$(pwd)/install --host=arm-none-eabi --build=x86_64-linux-gnu CFLAGS="-mcpu=cortex-m3 --specs=nosys.specs" LDFLAGS="--specs=nosys.specs" --enable-amrnb-encoder --disable-amrnb-decoder --enable-amrnb-tiny

rm -rf install
make distclean
autoreconf --install && \
./configure --prefix=$(pwd)/install --enable-amrnb-encoder --enable-amrnb-decoder --enable-amrnb-tiny --disable-shared --enable-static && \
make -j6 && \
make install -j6
