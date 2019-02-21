
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/klog.h>

#include "dm.h"

#define ACK_SUCESS	0x38	// reply for remote
#define ACK_FAIL 	0x39
#define BT_EXIST 	0x32
#define HEAVY_FULL      0x34
#define LIGHT_FULL	0x36
#define TOTAL_FULL	0x40
#define NNTP_FULL	0x42
#define ACK_LIMIT	0x44
#define DISK_FULL	0x46
#define ED2K_FULL	0x47
#define NOW_HASHING     0x48 //2012.06.27 eric added
#define UNKNOW 	0x30




/* CGI helper functions */
extern void init_cgi(char *query);
extern char * get_cgi(char *name);

#define websGetVar(wp, var, default) (get_cgi(var) ? : default)


int chk_on_process(char *download_name, char *infohash);
