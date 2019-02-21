#CC = mipsel-linux-gcc
#CC = arm-linux-uclibc-gcc
CFLAGS = -g -rdynamic -D_FILE_OFFSET_BITS=64 -I/opt/include -I/opt/trendchip/mips-linux-uclibc/usr/include -I/opt/trendchip/mips-linux-uclibc/usr/include/libxml2
LDFLAGS = -ffunction-sections -fdata-sections -L/opt/trendchip/mips-linux-uclibc/usr/lib -Wl,-rpath,/opt/lib -Wl,-rpath-link,/opt/trendchip/mips-linux-uclibc/usr/lib
LIBS = -lcurl -lxml2 -lpthread

all:dm2_rss

dm2_rss:rss.o
	$(CC) $(CFLAGS) $(LDFLAGS) rss.c -o dm2_rss $(LIBS)

.PHONY:clean
clean:
	rm -rf dm2_rss *.o
