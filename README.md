
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



## Labo 12

1. Trouver le fichier et la fonction qui s’occupe d’afficher le tout premier message de U-Boot

Le message est affiché dans la fonction display_options dans le fichier ./u-boot/lib/display_options.c

```c
int display_options(void)
{
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(true, buf, sizeof(buf));
	printf("%s", buf);

	return 0;
}
```


2. Depuis le terminal de U-Boot appeler la commande version, examiner la Backtrace

```bash
backtrace
#0  do_version (cmdtp=0x7ffe3b04, flag=0, argc=1, argv=0x7ef1a720) at /home/andre/dev/heig-vd/pos/pos24/u-boot/cmd/version.c:27
#1  0x7ff7bf60 in cmd_call (repeatable=0x7ee17d74, argv=0x7ef1a720, argc=1, flag=<optimized out>, cmdtp=0x7ffe3b04) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/command.c:580
#2  cmd_process (flag=<optimized out>, argc=1, argv=0x7ef1a720, repeatable=0x7ffe5608, ticks=0x0) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/command.c:635
#3  0x7ff72d74 in run_pipe_real (pi=0x7ef1a668) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli_hush.c:1676
#4  run_list_real (pi=pi@entry=0x7ef1a668) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli_hush.c:1873
#5  0x7ff72f00 in run_list (pi=0x7ef1a668) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli_hush.c:2022
#6  parse_stream_outer (inp=0x7ee17e5c, inp@entry=0x7ee17e54, flag=flag@entry=2) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli_hush.c:3206
#7  0x7ff73490 in parse_file_outer () at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli_hush.c:3289
#8  0x7ff7b4a8 in cli_loop () at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/cli.c:229
#9  0x7ff714e8 in main_loop () at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/main.c:66
#10 0x7ff73ad0 in run_main_loop () at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/board_r.c:584
#11 0x7ff73d6c in initcall_run_list (init_sequence=0x7ffe02ac) at /home/andre/dev/heig-vd/pos/pos24/u-boot/include/initcall.h:46
#12 board_init_r (new_gd=<optimized out>, dest_addr=<optimized out>) at /home/andre/dev/heig-vd/pos/pos24/u-boot/common/board_r.c:822
#13 0x7ff59ac4 in ?? () at /home/andre/dev/heig-vd/pos/pos24/u-boot/arch/arm/lib/crt0.S:179
Backtrace stopped: previous frame identical to this frame (corrupt stack?)
```

U-boot tourne en boucle (`cli_loop`) en lisant les caractères qui arrivent sur la cli. Notons qu'il n'y a pas d'interruptions au sein de u-boot.
Une fois la detection d'un caractère de newline, il va prendre la ligne et essayer de mapper la commande entrée à la fonction correspondante (`cmd_process`).
Une fois la bonne fonction trouvé, nous arrivons enfin à `do_version` qui doit s'occuper d'éxecuter la commande en soit (dans ce cas, afficher la version)
