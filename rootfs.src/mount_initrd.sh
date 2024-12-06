#!/bin/bash
echo "-------------------mount initrd ---------------"

if [ $# -ne 1 ]; then
        echo "Usage: ./mount_initrd <board>"
	echo "Please provide the board name (virt32, rpi4, virt64, rpi4_64)"
	exit 0
fi 
echo "Here: board is $1"
sudo rm -rf fs
mkdir fs
cd fs
sudo cpio -id < ../board/$1/initrd.cpio
cd ..
