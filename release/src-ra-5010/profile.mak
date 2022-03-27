export EXTRACFLAGS := -DLINUX26 -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -funit-at-a-time -Wno-pointer-sign -mtune=mips32r2 -mips32r2

EXTRACFLAGS += -DLINUX30

export EXTRACFLAGS
