this-makefile := $(lastword $(MAKEFILE_LIST))
abs_srctree := $(realpath $(dir $(this-makefile)))

-include $(abs_srctree)/../../../build.conf

KDIR = $(abs_srctree)/../../linux

obj-m := access_mod.o

access_mod-objs += rpisense.o accel.o

.PHONY: module clean

module: $(abs_srctree)/../../../build.conf
	@make -C $(KDIR) M=$(abs_srctree) modules
	@mkdir -p ../out/$(PLATFORM)
	@cp *.ko ../out/$(PLATFORM)

clean:
	@make -C $(KDIR) M=$(abs_srctree) clean


