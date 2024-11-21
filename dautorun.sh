echo Building the rootfs in the container

docker run --privileged -t --env PATH="/opt/toolchains/gcc-linaro-11.3.1-2022.06-x86_64_arm-linux-gnueabihf/bin:$PATH" --mount source=rootfs,target=/root/pos24-main/rootfs pos24 sh -c ./build_rootfs.sh 


