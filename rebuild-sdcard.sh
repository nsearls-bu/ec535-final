#!/bin/bash

dd if=/dev/zero of=sdcard.img bs=1M count=512
parted --align=opt --script -- ./sdcard.img mktable msdos \
    mkpart primary fat16 0\% 63MiB \
    mkpart primary ext4 64MiB 100\% \
    set 1 boot on \
    unit MiB \
    print
dd if=bootfs.img of=sdcard.img bs=1M seek=1 conv=notrunc
dd if=rootfs.img of=sdcard.img bs=1M seek=64 conv=notrunc