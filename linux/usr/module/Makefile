

KDIR = ../../linux

obj-m := access_mod.o

access_mod-objs += access.o rpisense.o

module:
	if [ ! -f ${KDIR}/Module.symvers ]; then \
		echo "Generating Module.symvers..." ; \
		make -C ${KDIR} modules ; \
	fi

	@make -C $(KDIR) M=$(PWD)/module modules
	@cp *.ko ../out
	
clean:
	@make -C $(KDIR) M=$(PWD)/module clean


