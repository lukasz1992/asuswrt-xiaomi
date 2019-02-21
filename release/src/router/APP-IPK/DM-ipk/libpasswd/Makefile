#include ../common.mak
#include $(SRCBASE)/.config

#CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
#LDFLAGS += $(EXTRALDFLAGS) -lcrypt
#
#LDFLAGS = -L$(TOOLCHAIN)/../arm-buildroot-linux-uclibcgnueabi/sysroot/usr/lib -lcrypt 
#CFLAGS += -Os -Wall  -fPIC
INSTALL = install

OBJS = passwd.o

all: libpasswd.so

libpasswd.so: $(OBJS)
	@echo " [libpasswd] LD $@"
	@$(CC) $(LDFLAGS_P) -shared -o $@ $^


install: all
	@echo " [libpasswd] Installing as $(SRCBASE)/opt/lib/libpasswd.so"
	@echo "TOOLCAHIN: $(TOOLCHAIN)"
	@install -d $(SRCBASE)/opt/lib
	@install -m 755 libpasswd.so $(SRCBASE)/opt/lib
	@$(STRIP) $(SRCBASE)/opt/lib/libpasswd.so

clean:
	rm -f *.o *.so *.a .*.depend *.prep

%.o: %.c .%.depend
	@echo " [libpasswd] CC $@"
	@$(CC) $(CFLAGS_P) -o $@ -c $<

%.o: prebuild/%.o
	@echo " [libpasswd] cp $@"
	@cp -f $< $@

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS_P) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS_P) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS_P) -E $< > $@

-include $(OBJS:%.o=.%.depend)

