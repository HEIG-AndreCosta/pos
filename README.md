
# Repository ``pos_student`` 
 
## Scripts

A set of scripts to manage the Docker container used to create a ``rootfs`` from ``buildroot``

```
- dbuild.sh <arg> : Build the container image to build the rootfs based on buildroot
- drun.sh         : Run the container in an interactive session (bash shell)
- dautorun.sh     : Run the build of rootfs (invoking the ./build_rootfs.sh script)
- dcp.sh          : Copy the generated rootfs.cpio from the container to the current directory
- build_rootfs.sh : Perform the build of rootfs in ``root/`` directory

```


