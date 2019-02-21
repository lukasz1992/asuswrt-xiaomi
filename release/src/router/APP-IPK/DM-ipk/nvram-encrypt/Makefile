#include ../common.mak
#include $(SRCBASE)/.config

#OBJS += $(if $(wildcard spwenc.c), spwenc.o, prebuild/spwenc.o)

all: 
	@$(CC) $(CFLAGS) -c spwenc.c


clean:
	rm *.o

