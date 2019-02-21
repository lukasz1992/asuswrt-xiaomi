#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

#define CONFIG_FILE "/opt/etc/dm2_rss.conf"
#define RSS_QUEUE_NUM 20

int rss_check_time;

char urllist[10][128];   //config已有
char namelist[10][128];
char new_pubdate[10][64];    //rss最近一次更新时间
int itemcount[10];      //种子总数目
int item_dl_count[10];   //已经下载的种子数目
int dlcount; //total dl BT file num
//char logurl[49];
char logurl[128];

xmlDocPtr doc;
xmlNodePtr root;

int urlcount, namecount;

int quitting;//process exit safety
int run_again;  //run_again

int rss_xml_get_success;//get rss xml file is completed

char *base_path;

char rss_file[128];
char rss_path[128];
char rss_date_file[128];
#define LOG_FILE "/tmp/dm2_rss.log"

char jqs_path[128];

int jqs_count;

