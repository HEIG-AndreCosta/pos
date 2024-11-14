echo Building the rootfs in the container

docker run --privileged --interactive -t --mount source=rootfs,target=/root/pos24-main/rootfs pos24

