# GNU Makefile for building user apps in the SOO environment
#
# Modif. 2015
#
# First setup the building environment


GCC       = arm-linux-gnueabihf-
CC           = $(GCC)gcc
AS           = $(GCC)as
AR           = $(GCC)ar
LD           = $(GCC)ld
CPP          = $(GCC)cpp
OBJDUMP	     = $(GCC)objdump

gccincdir := $(shell $(CC) -print-file-name=include)
LIBGCC_PATH := $(shell dirname `$(CC) $(USR_CFLAGS) -print-libgcc-file-name`)

AFLAGS      := -D__ASSEMBLY__

# Compiler Options - Optimisation, warnings and compiler execution parameters
USR_CFLAGS      := -O0 -pipe -Wall  -D__ARM__ -marm  -mno-thumb-interwork  -g

# Compiler Options - Libraries parameters
USR_CFLAGS      += -ffreestanding -I. -Isrc/include -fno-common  

# Aggressive flags
USR_CFLAGS	+= -Wall -Werror 

# libc required
USR_CFLAGS	+= -std=c99  -D_GNU_SOURCE

LDFLAGS      = -N -warn-common -warn-constructors -warn-multiple-gp -L $(LIBGCC_PATH) -lgcc 


# Our source apps
-include src/Makefile

USR_DIR = $(PWD)
export USR_DIR

USR_CFLAGS += -I$(LIBC_DIR)/include

.PHONY: all clean check_out_dir modules

# make executes the first rule it finds in the Makefiles, unless it is specified
# on the comand line
all: check_out_dir $(patsubst %,out/%,$(TARGETS)) modules

modules:
	@make -C module module
	
check_out_dir:
	@mkdir -p out

.SECONDARY: $(patsubst %,src/%.o,$(TARGETS))

# Compiler
out/%: src/%.c 
	$(CC) $(USR_CFLAGS) $< -o $@

# Generic targets
clean: 
	rm -rf out
	make -C module clean

	
	
