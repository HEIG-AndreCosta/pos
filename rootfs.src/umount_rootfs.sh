#!/bin/bash
echo "-------------------umount rootfs ---------------"

if [ $# -ne 1 ]; then
        echo "Usage: ./umount_rootfs <board>"
	echo "Please provide the board name (virt32, virt64, rpi4, rpi4_64)"
	exit 0
fi 
echo "Here: board is $1"
cd fs
truncate -s 0 ../board/$1/rootfs.cpio
sudo find . | sudo cpio -o --format='newc' >> ../board/$1/rootfs.cpio
cd ..
sudo rm -rf fs


