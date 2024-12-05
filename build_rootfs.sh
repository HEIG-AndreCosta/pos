git checkout rootfs.src
cd rootfs.src
make virt32_defconfig
make -j$(nproc)
