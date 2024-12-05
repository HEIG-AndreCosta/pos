#!/bin/bash

usage()
{
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Where OPTIONS are:"
  echo "  -b    Deploy boot (kernel, U-boot, etc.)"
  echo "  -r    Deploy rootfs (secondary) in the second partition"
  echo "  -u    Deploy Linux usr apps"
  echo "  -s    Deploy SO3 usr apps"
  echo ""

  exit 1
}

while getopts "brus" o; do
  case "$o" in
    b)
      deploy_boot=y
      ;;
    r)
      deploy_rootfs=y
      ;;
    u)
      deploy_usr=y
      ;;
    s)
      deploy_usr_so3=y
      ;;
    *)
      usage
      ;;
  esac
done

if [ $OPTIND -eq 1 ]; then usage; fi

while read var; do
if [ "$var" != "" ]; then
  export $(echo $var | sed -e 's/ //g' -e /^$/d -e 's/://g' -e /^#/d)
fi
# The following wizard ensure there is a final return line as read need it
done < <(cat build.conf; echo)

# We now have ${PLATFORM} which names the platform base
# and ${PLATFORM_TYPE} to be used when the type is required.
# Note that ${PLATFORM_TYPE} can be equal to ${PLATFORM} if no type is specified.

if [ "$PLATFORM" != "virt32" -a "$PLATFORM" != "virt64" ]; then
    echo "Specify the device name of MMC (ex: sdb or mmcblk0 or other...)"
    read devname
    export devname="$devname"
fi

cd filesystem
./umount.sh
cd ../

if [ "$deploy_usr_so3" == "y" ]; then

    # Deploy the usr apps related to the agency
    # Only for SO3
    cd so3/usr
    ./build.sh
    ./deploy.sh ${PLATFORM}
    cd ../..
    deploy_boot=y
fi

if [ "$deploy_boot" == "y" ]; then
    # Deploy files into the boot partition (first partition)
    echo Deploying boot files into the first partition...

    cd target
    ./mkuboot.sh ${PLATFORM}
    cd ../filesystem
    ./mount.sh 1
    sudo rm -rf fs/*
    [ -f ../target/${PLATFORM}.itb ] && sudo cp ../target/${PLATFORM}.itb fs/ && echo ITB deployed.
    sudo cp ../u-boot/uEnv.d/uEnv_${PLATFORM}.txt fs/uEnv.txt

    if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "virt64" ]; then
        # Nothing else ...
        ./umount.sh
        cd ..
    fi

    if [ "$PLATFORM" == "rpi4" ]; then
        sudo cp -r ../bsp/rpi4/* fs/
        sudo cp ../u-boot/build/rpi4/u-boot.bin fs/kernel7.img
        ./umount.sh
        cd ..
    fi

    if [ "$PLATFORM" == "rpi4_64" ]; then
        sudo cp -r ../bsp/rpi4/* fs/
        sudo cp ../u-boot/build/rpi4_64/u-boot.bin fs/kernel8.img
        ./umount.sh
        cd ..
    fi
fi

if [ "$deploy_rootfs" == "y" ]; then
    # Deploy of the rootfs (second partition)
    # Only used for Linux
    cd rootfs
    ./deploy.sh
    cd ..
fi

if [ "$deploy_usr" == "y" ]; then

    # Deploy the usr apps related to the agency
    # Only for Linux
    cd linux/usr
    ./deploy.sh
    cd ../..
fi
