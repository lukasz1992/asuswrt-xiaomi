
# Make sure that ARCH, KERNEL_DIR and CROSS_COMPILE are set

ifndef KERNEL_DIR
  ERROR_KERNEL_DIR_NOT_SET:=YES
endif


#COMPILER:=$(CC) -mips32r2 -msoft-float
#  HOST_COMPILER:=gcc
ifndef ARCH
  ERROR_ARCH_NOT_SET:=YES
endif

#ifeq ($(KERNELRELEASE),)
#   PWD:=$(shell pwd)
#endif
PWD :=drivers/net/cryptoDriver
SRC_APP=./source
SRC_UTILS=./source/utils

PATH_BUILD=$(PWD)


# Include Paths
INCLUDE_APP:= \
 -I$(PATH_BUILD)/include


INCLUDE_FLAGS_ALL:= \
  $(INCLUDE_APP)


# Compiler Flags
WARNING_FLAGS=
DEBUG_FLAGS=-g
CODE_COVERAGE_FLAGS=
PROFILING_FLAGS=
BACKWARD_COMPATIBILITY_FLAGS=

EXTRA_CFLAGS += $(BACKWARD_COMPATIBILITY_FLAGS)
EXTRA_CFLAGS += $(INCLUDE_FLAGS_ALL)
EXTRA_CFLAGS += $(WARNING_FLAGS)
EXTRA_CFLAGS += $(DEBUG_FLAGS)
EXTRA_CFLAGS += $(CODE_COVERAGE_FLAGS)
EXTRA_CFLAGS += $(PROFILING_FLAGS)
EXTRA_CFLAGS += -DRT_EIP93_DRIVER
EXTRA_CFLAGS += -DRT_EIP93_DRIVER_DEBUG -DRT_EIP93_DRIVER_DEBUG_H
EXTRA_CFLAGS += -DMTK_CRYPTO_DRIVER
#EXTRA_CFLAGS += -I$(MODULES_KPROFILE_DIR)
#EXTRA_CFLAGS += -DEXPORT_SYMTAB -D__KERNEL_SYSCALLS__

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

.PHONY = clean clean_module

obj-m	+= crypto_k.o
crypto_k-objs:= \
	$(SRC_UTILS)/mtk_sLib.o \
  $(SRC_UTILS)/mtk_arm.o \
  $(SRC_UTILS)/mtk_ringHelper.o \
  $(SRC_UTILS)/mtk_hwDevice.o \
  $(SRC_UTILS)/mtk_hwDmaResource.o \
  $(SRC_UTILS)/mtk_dmaBufHelper.o \
  $(SRC_APP)/mtk_pktProcess.o \
  $(SRC_APP)/mtk_eip93Init.o \
  $(SRC_APP)/mtk_pecInit.o \
  $(SRC_APP)/mtk_interruptHelper.o	\
	$(SRC_APP)/mtk_init.o

#obj-m	+= crypto_k.o
#crypto_k-objs += \
#	./cryptoDriver_init.o
#crypto_k-objs += ./lib_crypto_k.obj

##obj-m	+= crypto_k.o
#obj-m += lib_crypto_k.o
#lib_crypto_k-objs+= \
  $(SRC_UTILS)/mtk_sLib.o \
  $(SRC_UTILS)/mtk_arm.o \
  $(SRC_UTILS)/mtk_ringHelper.o \
  $(SRC_UTILS)/mtk_hwDevice.o \
  $(SRC_UTILS)/mtk_hwDmaResource.o \
  $(SRC_UTILS)/mtk_dmaBufHelper.o \
  $(SRC_APP)/mtk_pktProcess.o \
  $(SRC_APP)/mtk_eip93Init.o \
  $(SRC_APP)/mtk_pecInit.o \
  $(SRC_APP)/mtk_interruptHelper.o \
	$(SRC_APP)/mtk_init.o

#crypto_k-objs += ./lib_crypto_k.obj
	

clean:	clean_module

clean_module:
	# this is run in $PATH_BUILD where the Makefile resides
	find . \( -name "*.o" -o -name ".*.cmd" -o -name "*.bak"  \) -type f -print | xargs rm -f
	rm -rf .tmp_versions
	rm -f *.o  .*.cmd  .*.flags  *.mod.c  *.ko  *.symvers  *.markers  *.order
	
# end of file	
