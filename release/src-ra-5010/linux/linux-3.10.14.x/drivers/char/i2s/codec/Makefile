#EXTRA_CFLAGS += -Werror
KBUILD_CFLAGS += -I$(srctree)

ifeq ($(CONFIG_ARCH_MT7623),y)
ccflags-y += -I$(srctree)/../mediatek/platform/mt7623/kernel/drivers/i2c
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8751.o
endif

ifeq ($(CONFIG_RALINK_RT2883),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8751.o
endif

ifeq ($(CONFIG_RALINK_RT3350),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8751.o
endif

ifeq ($(CONFIG_RALINK_RT3883),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
ifeq ($(CONFIG_RALINK_I2C_WM8960), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8960.o
endif
ifeq ($(CONFIG_RALINK_I2C_WM8750), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif
endif

ifeq ($(CONFIG_RALINK_RT6855),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif

ifeq ($(CONFIG_RALINK_RT6855A),y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
ifeq ($(CONFIG_RALINK_I2C_WM8960), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8960.o
endif
ifeq ($(CONFIG_RALINK_I2C_WM8750), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
ifeq ($(CONFIG_RALINK_I2C_WM8960), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8960.o
endif
ifeq ($(CONFIG_RALINK_I2C_WM8750), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7628),y)
ifeq ($(CONFIG_RALINK_I2C_WM8960), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8960.o
endif
ifeq ($(CONFIG_RALINK_I2C_WM8750), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif
endif

ifeq ($(CONFIG_ARCH_MT7623),y)
ifeq ($(CONFIG_RALINK_I2C_WM8960), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8960.o
endif
ifeq ($(CONFIG_RALINK_I2C_WM8750), y)
obj-$(CONFIG_RALINK_I2C_CODEC) += i2c_wm8750.o
endif
endif

clean:
	rm -rf ./*.mod.* ./*.ko ./*.o ./*.bak ./.*.cmd
	rm -fr modules.order
	rm -ff modules.builtin
