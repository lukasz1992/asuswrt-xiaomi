#CC = arm-linux-uclibc-gcc

all: dm_uploadbt.cgi dm_print_status.cgi dm_apply.cgi dm_disk_info.cgi

dm_uploadbt.cgi: dm_uploadbt_cgi.o dm_cgi.o dm_btdecode.o dm_func.o
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $^  -Wl,-rpath=/opt/lib 

dm_print_status.cgi:dm_print_status.o dm_cgi.o dm_func.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^  -Wl,-rpath=/opt/lib

dm_apply.cgi: dm_apply_cgi.o dm_cgi.o dm_func.o  dm_url_parser.o dm_http_parser.o dm_ftp_parser.o
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $^ -Wl,-rpath=/opt/lib

dm_disk_info.cgi: dm_disk_info.o dm_cgi.o
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $^ -Wl,-rpath=/opt/lib

clean:
	rm -f *.o *.cgi 
