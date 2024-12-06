#!/bin/bash
echo "-------------------umount initrd ---------------"

if [ $# -ne 1 ]; then
        echo "Usage: ./umount_initrd <board>"
	echo "Please provide the board name (virt32, rpi4)"
	exit 0
fi 
echo "Here: board is $1"
cd fs
truncate -s 0 ../board/$1/initrd.cpio
find . | sudo cpio -o --format='newc' >> ../board/$1/initrd.cpio
cd ..
sudo rm -rf fs
