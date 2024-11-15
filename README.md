
# POS Repository

## Directories

Here is a description of the main directories of pos repository

```

- linux/             : Main directory for all Linux components
- linux/linux/       : Linux kernel source tree
- linux/usr/         : User space applications and modules which will be transfered in the rootfs during the deployment
- linux/usr/module/  : Kernel modules which will be inserted dynamically via insmod
- rootfs/            : Root tree of rootfs components, including Makefile to build the buildroot based rootfs
- buildroot/         : Source tree of buildroot


```

## Scripts

A set of scripts to manage the Docker container used to create a ``rootfs`` from ``buildroot``

```
- dbuild.sh <arg> : Build the container image to build the rootfs based on buildroot
- drun.sh         : Run the container in an interactive session (bash shell)
- dautorun.sh     : Run the build of rootfs (invoking the ./build_rootfs.sh script)
- dcp.sh          : Copy the generated rootfs.cpio from the container to the current directory
- build_rootfs.sh : Perform the build of rootfs in ``root/`` directory

```


