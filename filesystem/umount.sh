#!/bin/bash

if [ "$PLATFORM" == "" ]; then
    if [ "$1" == "" ]; then
        echo "PLATFORM must be defined (virt32, virt64, rpi4, rpi4_64)"
        echo "You can invoke umount.sh <platform>"
        exit 0
    fi

    PLATFORM=$1
fi

if [ -d fs ]; then
    sync -f fs
fi

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64" ]; then
    devpaths=$(losetup --associated sdcard.img.$PLATFORM --output NAME --noheadings)
    devname=$(echo $devpaths | sed 's/\/dev\/\(.*\)/\1/g')

    if [ "$devpaths" == "" ]; then
        # The image isn't mounted
        exit 0
    fi
fi

if [ "$devname" == "" ]; then
    echo "Specify the device name of MMC (ex: sdb or mmcblk0 or other...)"
    read devname

    devpaths="/dev/$devname"
fi

devname=$(echo $devname | sed 's/\([^ ]*[0-9]\)/\1p/g')

while mounted_dev=$(mount -l | grep -E "/dev/($(echo $devname | sed 's/ /|/g'))" | sed -n 's/^\(\/dev\/[^ ]*\).*$/\1/p') && [ "$mounted_dev" != "" ]; do
    # Ensure that nothing is using a file in mounted device
    while lsof +f -- ${mounted_dev}; do
        echo "Waiting for target to be unbusy"
        sleep 0.5
    done

    sudo umount ${mounted_dev}
done

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64" ]; then
    sudo losetup --detach $devpaths
fi
