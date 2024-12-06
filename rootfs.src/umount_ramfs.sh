#!/bin/bash

if [ $# -ne 1 ]; then
        echo "Usage: ./umount_ramfs <board>"
	echo "Please provide the board name (virt32, rpi4, virt64, rpi4_64)"
	exit 0
fi

echo "Here: board is $1"

sync

while [ "$(findmnt fs)" != "" ]; do
	sudo umount fs
done

loops=$(losetup --associated ./board/$1/rootfs.fat --output NAME --noheadings)
sudo losetup --detach $loops

sudo rm -rf fs
