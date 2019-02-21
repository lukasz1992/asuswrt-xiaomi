###############################################################################
#  Jan 2007 Bruce Chang
#
#  Initial Release
#
#
#
###############################################################################
EXTRA_CFLAGS += -Werror

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

obj-y    := reset.o init.o irq.o \
            memory.o printf.o cmdline.o setup.o time.o patchinfo.o

ifeq ($(CONFIG_MIPS_MT_SMP),y)
	obj-y += malta-amon.o
endif

obj-$(CONFIG_KERNEL_NVRAM) += nvram.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_DWC_OTG) += lm.o
obj-$(CONFIG_RALINK_TIMER_WDG) +=  ralink_wdt.o
obj-$(CONFIG_RALINK_TIMER_DFS) +=  rt_timer.o
obj-$(CONFIG_RT_DMA) +=  dev-dma.o
obj-$(CONFIG_MTK_MTD_NAND) +=  dev-nand.o
ifeq ($(CONFIG_RALINK_MT7621),y)
obj-$(CONFIG_MTD_ANY_RALINK) +=  dev-nand.o
endif
#obj-$(CONFIG_I2C_RALINK) +=  dev-i2c.o

obj-$(CONFIG_RALINK_RT3883) +=  ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_RT3352) +=  ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_RT5350) +=  ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_RT6855) +=  ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_MT7620) +=  ehci_ohci.o udc.o
obj-$(CONFIG_RALINK_MT7628) +=  ehci_ohci.o udc.o
