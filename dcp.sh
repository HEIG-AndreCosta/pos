echo Retrieving rootfs.cpio from the docker container

container_id=$(docker create pos24)
docker cp "$container_id":/root/pos24-main/rootfs/board/virt32/rootfs.cpio .

