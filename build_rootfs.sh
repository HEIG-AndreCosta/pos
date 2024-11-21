cd rootfs
make virt32_defconfig
make -j$(nproc)
