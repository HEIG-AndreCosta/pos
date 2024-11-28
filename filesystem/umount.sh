#!/bin/bash

if [ "$PLATFORM" == "" ]; then
    if [ "$1" == "" ]; then
        echo "PLATFORM must be defined (virt32, virt64, rpi4, rpi4_64)"
        echo "You can invoke umount.sh <platform>"
        exit 0
    fi

    PLATFORM=$1
fi

sync -f fs

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64" ]; then
    devpaths=$(losetup --associated sdcard.img.$PLATFORM --output NAME --noheadings)
    devname=$(echo $devpaths | sed 's/\/dev\///g')
fi

if [ "$devname" == "" ]; then
    echo "Specify the device name of MMC (ex: sdb or mmcblk0 or other...)"
    read devname

    devpaths="/dev/$devname"
fi

mounted_dev=$(mount -l | grep -E "/dev/($(echo $devname | sed 's/ /|/g'))p" | sed -n 's/^\(\/dev\/[^ ]*\).*$/\1/p')

if [ "$mounted_dev" != "" ]; then
    # Ensure that nothing is using a file in mounted device
    while lsof +f -- ${mounted_dev}; do
        echo "Waiting for target to be unbusy"
        sleep 0.5
    done

    sudo umount ${mounted_dev}
fi

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64" ]; then
    sudo losetup --detach $devpaths
fi
