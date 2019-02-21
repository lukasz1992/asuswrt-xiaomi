all: dm2_detect

dm2_detect: dm_detect.o dm_ftp_parser.o dm_http_parser.o dm_url_parser.o
ifeq ($(NEW_ARM),y)
	$(CC)  -o $@ $^ -Wl,-rpath=/opt/lib -Wl,-dynamic-linker=/opt/lib/ld-uClibc.so.1 -ldl
else
	$(CC)  -o $@ $^ -Wl,-rpath=/opt/lib
endif
clean:
	rm -f *.o dm2_detect 
