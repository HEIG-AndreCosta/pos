#!/bin/bash

# Launch script for SO3

N_QEMU_INSTANCES=`ps -A | grep qemu | wc -l`
GDB_PORT_BASE=1234
USR_OPTION=$1

QEMU_MAC_ADDR=`printf 'DE:AD:BE:EF:%02X:%02X\n' $(($RANDOM % 255)) $((N_QEMU_INSTANCES))`
GDB_PORT=$(($GDB_PORT_BASE + $N_QEMU_INSTANCES))


    echo -e "\033[01;36mMAC addr: " ${QEMU_MAC_ADDR} "\033[0;37m"
    echo -e "\033[01;36mGDB port: " ${GDB_PORT} "\033[0;37m"

    while read var; do
        if [ "$var" != "" ]; then
                export $(echo $var | sed -e 's/ //g' -e /^$/d -e 's/://g' -e /^#/d)
        fi
    # The following wizard ensure there is a final return line as read need it
    done < <(cat build.conf; echo)

    if [ "$PLATFORM" == "virt64" ]; then
	echo Starting on ARM64
    sudo qemu-system-aarch64 $@ ${USR_OPTION} \
 	-smp 4  \
	-serial mon:stdio  \
	-M virt -cpu cortex-a72  \
	-device virtio-blk-device,drive=hd0 \
	-drive if=none,file=filesystem/sdcard.img.virt64,id=hd0,format=raw,file.locking=off \
	-m 1024 \
	-kernel u-boot/build/virt64/u-boot \
	-nographic \
	-netdev tap,id=n1,script=scripts/qemu-ifup.sh,downscript=scripts/qemu-ifdown.sh \
	-device virtio-net-device,netdev=n1,mac=${QEMU_MAC_ADDR} \
        -gdb tcp::${GDB_PORT}

    else
   echo Starting on ARM32
   sudo qemu/build/arm-softmmu/qemu-system-arm $@ ${USR_OPTION} \
  	-smp 2 \
	-serial mon:stdio  \
	-M virt   -cpu cortex-a15 \
	-device virtio-blk-device,drive=hd0 \
	-drive if=none,file=filesystem/sdcard.img.virt32,id=hd0,format=raw,file.locking=off \
	-m 1024 \
	-kernel u-boot/build/virt32/u-boot \
	-net tap,script=scripts/qemu-ifup.sh,downscript=scripts/qemu-ifdown.sh -net nic,macaddr=${QEMU_MAC_ADDR} \
	-nographic \
	-gdb tcp::${GDB_PORT}

    fi

