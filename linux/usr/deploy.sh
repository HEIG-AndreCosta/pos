#!/bin/bash

if [ "$PLATFORM" == "" ]; then
    if [ "$1" == "" ]; then
        echo "PLATFORM must be defined (vexpress, rpi4)"
        echo "You can invoke mount.sh <partition_nr> <platform>"
        exit 0
    fi

    PLATFORM=$1
fi
# Deploy usr apps into the second partition
echo Deploying usr apps into the second partition...
cd ../../filesystem
./mount.sh 2
sudo cp ../linux/usr/out/$PLATFORM/* fs/root
./umount.sh
