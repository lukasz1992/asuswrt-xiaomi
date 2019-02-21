include ../common.mak
include $(SRCBASE)/.config
EXEC = switch
ifeq ($(CONFIG_P5_RGMII_TO_MT7530_MODE),y)
EXEC += switch_7530 
endif
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth -Werror
CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h

ifeq ($(RTCONFIG_RALINK_RT6855),y)
CFLAGS += -DRTCONFIG_RALINK_RT6855
SRC=switch_gsw.c
else
ifeq ($(RTCONFIG_RALINK_MT7620),y)
CFLAGS += -DRTCONFIG_RALINK_MT7620
SRC=switch_gsw.c
else
ifeq ($(RTCONFIG_RALINK_MT7621),y)
CFLAGS += -DRTCONFIG_RALINK_MT7621
SRC=switch_gsw.c
else
ifeq ($(RTCONFIG_RALINK_RT6855A),y)
CFLAGS += -DRTCONFIG_RALINK_RT6855A

SRC=switch_gsw.c
else
ifeq ($(RTCONFIG_RARCH_MT7623),y)
CFLAGS += -DRTCONFIG_RARCH_MT7623
SRC=switch_gsw.c
else
SRC=switch.c
endif
endif
endif
endif
endif

all: $(EXEC)
ifeq ($(CONFIG_P5_RGMII_TO_MT7530_MODE),y)
switch_7530: $(SRC) $(CONF_H)
	$(CC) $(CFLAGS) -DSWITCH_7530 $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)
endif

switch: $(SRC) $(CONF_H)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/switch
ifeq ($(CONFIG_P5_RGMII_TO_MT7530_MODE),y)
	$(ROMFSINST) /bin/switch_7530
endif

install: all
	install -D $(EXEC) $(INSTALLDIR)/bin/$(EXEC)
	$(STRIP) $(INSTALLDIR)/bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

