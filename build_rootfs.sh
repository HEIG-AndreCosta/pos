cd rootfs.src
make virt32_defconfig
make -j$(nproc) --silent

#make virt64_defconfig
#make -j$(nproc) --silent

#make rpi4_defconfig
#make -j$(nproc) --silent

# make rpi4_64_defconfig
# make -j$(nproc) --silent
