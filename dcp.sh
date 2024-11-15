echo Retrieving rootfs.cpio from the docker container

docker run --rm -v "$(pwd):/mnt/host" -t -t --mount source=rootfs,target=/root/pos24-main/rootfs pos24 sh -c "cp /root/pos24-main/rootfs/board/virt32/rootfs.cpio /mnt/host/"

