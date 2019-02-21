export EXTRACFLAGS := -DLINUX26 -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32

EXTRACFLAGS += -DLINUX30

export EXTRACFLAGS
