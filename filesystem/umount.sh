#!/bin/bash

sudo umount fs

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64"  ]; then
    sudo losetup -D
fi
