#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef WRITE2FLASH
#include <linux/mtd/mtd.h>
#endif
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/klog.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>



//#include "common.h"
//#include <bcmnvram.h>
//#include "httpd.h" 

#pragma once

#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAX_NAMELEN  256
#define MAX_SHORTNAMELEN  128
#define MAX_SHORTLEN  32
#define MAX_SHOWS  100	// webpage max display lists, including completed and errors
#define MAX_QUES  50	// on_queue ones

#define MAXLINE  	2048
#define LISTENQ  	1024
#define RECV_TIMEVAL	2
#define MAX_PORTLEN  	8
#define MAX_SEEDLEN  	6

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
//static char *test_str;


#define HEAD_FLAG  	0x20
#define CONTENT_FLAG  	0x30

//#define TRUE	0x01
//#define FALSE	0x00

// action options

#define SHOW  		0x40	// show status
#define ADD  		0x41	// add job( fork a process to exec )
#define ADD_NO_ANSWER  	0x91	// add job( fork a process to exec )
#define RESU  		0x42	// resume the job( process )			// resume
#define PAUS  		0x43	// pause the job				// pause
#define STOP  		0x44	// stop the job( kill process )
#define CLEAR  		0x45	// delete the log				// clear
#define DEL  		0x46	// STOP & CLEAR( delete job & delete the log )	// cancel
//#define CLEAN  		0x47	// STOP & CLEAR & remove the file
#define OPENSEM		0x48	// reOpen semaphore and initial to 1
#define CLOSESEM	0x49	// close semaphore when download is stoped or completed
#define CLEARQ  	0x70	// delete the queue				// clear
#define WORKTIME        0x83    // client issue wpd time

#define HTTP	0x60
#define FTP	0x61
#define BT 	0x62
#define NZB 	0x63
#define NNTP 	0x64	//Allen NNTP
#define ED2K 	0x65

// nvram variable name
//#define NVVAL_POOL		"apps_pool"
//#define NVVAL_SHARE		"apps_share"
//#define NVVAL_DOWNLOAD_DIR	"usb_dm_completed_dir"
//#define NVVAL_PROXY		"usb_dm_proxy"
//#define NVVAL_START_HOUR	"usb_dm_start_hour"
//#define NVVAL_START_MINIUTE	"usb_dm_start_mini"
//#define NVVAL_GIFT		"usb_dm_gift_select"
//#define NVVAL_URL		"usb_dm_url"
//#define NVVAL_HTTP_PROXY	"usb_dm_http_proxy"
//#define NVVAL_FTP_HOST		"usb_dm_ftp_host"
//#define NVVAL_FTP_PORT		"usb_dm_ftp_port"
//#define NVVAL_FTP_USER		"usb_dm_ftp_user"
//#define NVVAL_FTP_PASS		"usb_dm_ftp_pass"
//#define NVVAL_FTP_PROXY		"usb_dm_ftp_proxy"
//#define NVVAL_SEEDTIME		"apps_dl_seed"

//Allen 
#define NVVAL_POOL		"apps_pool"
#define NVVAL_SHARE		"apps_share"
#define NVVAL_DL_X		"apps_dl_x"
#define NVVAL_SEEDTIME		"apps_dl_seed"
#define DOWNLOAD_FOLDER		"Download"

// SZ-Alicia, 090901 {
//./Allen #define DM_ROOT   nvram_safe_get(nvram_safe_get("usb_user_choice"))         // 2008.12 SZ-Wind for multi-disk 
#define DM_ROOT   "/tmp/harddisk/part0"         // Allen tmp

#define NVVAL_RSS_PERIOD	"rss_period"
#define NVVAL_RSS_UPDATE	"rss_update"
// SZ-Alicia, 090901 }

#define F_CONNECTING	1
#define F_READING	2
#define F_COMPLETE	4

#define LOCALHOST "127.0.0.1"
#define LocalHost "127.0.0.1"
//#define BT_PORT 32636 
//#define Bt_Port 32636
#define BT_PORT 9092 
#define NZB_PORT 6789 
#define SNARF_PORT     	3490 //magic added for snarfmaster 2011.04.12
#define SERV_PORT  	8936
#define Serv_Port	8936
#define GIFT_PORT       1213
#define FLV_PORT	8937
#define MAXBTURLLEN 	128
#define CMDMAXLEN MAXBTURLLEN+8
#define BT_HASHLEN 	41
#define S_TOKEN		"d"
#define C_TOKEN		'd'
#define S_END		'E'

#define PS_RUN		'R'
#define PS_PAUSED	'z'
#define PS_STOP		's'

#define A_LOG		'L'
#define A_SEM		'S'

#define TITLELEN	5

#define JQS_ID		-10


#define S_PROCESSING		0x70	// process status is processing
#define S_PAUSED		0x71	// process is paused
#define S_COMPLETED		0x72	// download completed and sucessful
#define S_DEAD_OF_DELETED	0x73	// download failed because of user delete
#define S_DEAD_OF_ERROR		0x74	// downlaod failed because of download error
#define S_INITIAL		0x75	// download initial, usual in bt
#define S_NOTBEGIN		0x76	// download queue
#define S_PARSER_FAIL           0x77    // parser url failed add by gauss
#define S_NEEDACCOUNT		0x78    // ftp need usrname or password
#define S_CONNECTFAIL           0x79   //nzb connected fail
#define S_SSLFAIL               0X80    //nzb ssl fail
#define S_ACCOUNTFAIL           0x81    //nzb account fail
#define S_RECVFAIL              0x82    //nzb soket rece fail
#define S_NOTCOMPLETED          0x83    //nzb downloaded file is incomplete
#define S_DISKFULL				0x84     //disk is full
#define S_TYPE_OF_ERROR				0x85     //disk is Vfat File more than 4G
#define S_HASH                  0x86        //the flag for creating hash
#define S_MOVE4GBERROR          0x87
#define S_MOVEDISKFULL          0x88
#define S_SEEDING		0x89	// download completed and sucessful Seeding

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

time_t now, load_check_timestamp, load_checkurl_timestamp, load_checkbt_timestamp, load_checked2k_timestamp, utility_check_timestamp;
int on_heavy_counts, on_light_counts, on_nntp_counts, on_ed2k_counts, total_nntp_counts, total_heavy_counts, total_light_counts, total_counts,on_heavy_seeding_counts,on_heavy_hashing_counts;
int heavy_queue,light_queue,nntp_queue,ed2k_queue,total_queue;
#define LOAD_CHECK_TIMEVAL	10	
#define UTILITY_CHECK_TIMEVAL	5
//#define MAX_ON_HEAVY	4
//#define MAX_ON_HEAVY	10
int MAX_ON_HEAVY;
#define MAX_ON_LIGHT	20	//light 6->20
//#define MAX_QUEUES	20	// (heavy + light) 30->20 20090826
//#define MAX_QUEUES	30
int MAX_QUEUES;
#define MAX_ON_NNTP	1	//Allen NNTP
int MAX_ON_ED2K;


#define HEAVY	9
#define LIGHT	1
#define NNTP_TYPE	5	//Allen NNTP

char  Share_dir[MAX_NAMELEN];
char Base_dir[MAX_NAMELEN];
char  In_dir[MAX_NAMELEN];
char  Seeds_dir[MAX_NAMELEN];
char  Log_dir[MAX_NAMELEN];
char  Sem_dir[MAX_NAMELEN];
char  jqs_dir[MAX_NAMELEN];
char  sem_path[MAX_NAMELEN];

char Download_dir_path[1024];

char lan_ip_addr[16];
char router_name[64];
char appname[16];
void _exec(char* pid_rtn, char *const argv[], int timeout, int maxfd);


#define SEM_MAGIC       0x89674523
#define MAX_NSEM	100
#define MAX_HASHLEN  	128		// 41 -> 128 for NNTP
                                                                                                                                               
#ifdef SEM_FAILED
#undef SEM_FAILED
#define SEM_FAILED      ((Sem_t *)(-1))
#endif
                                                                                                                                               
//./#define JOB_QUE_FN	"-10"
#define JOB_QUE_FN	"-10"

typedef struct{
        int sem_fd[2];
        int sem_magic;
	//./int logid;
	char  logid[MAX_HASHLEN];
}Sem_t;
                   
Sem_t  Sem[MAX_NSEM];
Sem_t  jqs_sem;
Sem_t  truncate_sem;

int  jqs_sem_use;
char  jqs_semName[256];
//char  jqs_file[256];
char  truncate_semName[32];

//./int  Sem_create(int);
//./int  Log_create(int);
//./int  Close_sem(int);
int  Sem_create(char *);
int  Log_create(char *);
int  Close_sem(char *);

int  Sem_close(Sem_t *);
int  Sem_open(Sem_t *, const char *, int, ... );
int  Sem_unlink(const char *);
int  Sem_post(Sem_t *);
int  Sem_wait(Sem_t *);

typedef struct LOGS_CHECK{
        char id[BT_HASHLEN];
	uint8_t  status;
}Logs_ID;

Logs_ID  logs_id[MAX_SHOWS];

int  ques_id[MAX_QUES];

int  first_log;

struct sockaddr_in  servaddr;

fd_set  R_set, W_set;
int  seed_time;


int remain_lists ;

char  opline[MAX_SHORTNAMELEN];
char  sharepath[MAX_NAMELEN], logpath[MAX_NAMELEN], sempath[MAX_NAMELEN];
char  jqs_file[MAX_NAMELEN], jqs_onweb[MAX_NAMELEN];
#ifdef DM_I686
char  jqs_file_bak[MAX_NAMELEN];
#endif
int jqs_sem_use;

struct Http_request_add{
	char  urlName[MAX_NAMELEN];
	char  proxy[MAX_SHORTNAMELEN];
};


struct Ftp_request_add{
	char  pathFile[MAX_NAMELEN];
	char  proxy[MAX_SHORTNAMELEN];
	char  host[MAX_SHORTNAMELEN];
	uint32_t  port;
	char  user[MAX_SHORTLEN];
	char  password[MAX_SHORTLEN];
};


struct Bt_request_add{
	char  torrentName[MAX_NAMELEN];
	uint32_t  seedTime;
};

struct NNTP_request_add{				//Allen NNTP
	char  nzbFile[MAX_NAMELEN];
	char  user[MAX_SHORTLEN];
	char  password[MAX_SHORTLEN];
	char  host[MAX_SHORTNAMELEN];
	int   use_SSL;
};

union Content_of_add{
        struct Http_request_add  http_add;
        struct Ftp_request_add  ftp_add;
        struct Bt_request_add  bt_add;
	struct NNTP_request_add  nntp_add;		//Allen NNTP
};
typedef struct Request_Head{
        uint8_t  flag;
        uint8_t  action;
        uint8_t  type;  // download type: http, ftp, bt
        uint32_t  Plen;  // despict the rest content packets's len (or extra for semaphore id)
        char  packetNum[BT_HASHLEN];  // rest packet numbers //2008.7 Gavin ,for rtorrent
}Phead;


#define HEAD_SIZE  (sizeof(Phead))


typedef struct Request_Add{
        uint8_t  cflag;
        char store_dst[MAX_NAMELEN];
        int ni_cv;
        union Content_of_add  addJob;
}Padd;


#define ADD_SIZE  (sizeof(Padd))
/*

typedef struct Request_Others{
        uint8_t  cflag;
        char  procName[MAX_SHORTLEN];
}Pothers;


#define NORMAL_SIZE  (sizeof(Pothers))
*/
typedef struct WORKPERIOD{
        int s_wday;
        int e_wday;
        int s_hr;
        int e_hr;
        int s_min;
        int e_min;
}wpd;
                                                                                                                                               
#define WPD_SIZE        sizeof(wpd)
#define WPDNUM          10

typedef struct INFO_BT{
        int id;
        uint32_t pieceSize;
        uint32_t pieceCount;
	uint8_t isPrivate;
        char comment[1024];
	char creator[32];
	time_t dateCreated;
	char downloadDir[64];
	uint64_t have;
	uint64_t haveValid;
	float  availability;
	uint64_t  downloadedsize;
	uint64_t  uploadedsize;
	float ratio;
	char errorString[256];
        double  downloadSpeed;		// current download speed
        double  uploadSpeed;		// current upload speed
        int  uploadpeers;
        int  downloadpeers;
	double health;	//Health ratio
	uint32_t fileCount;
        uint32_t trackCount;
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_bt;

typedef struct INFO_ED2K{
        int id;
        uint32_t pieceSize;
        uint32_t pieceCount;
	uint8_t isPrivate;
        char comment[1024];
	char creator[32];
	time_t dateCreated;
	char downloadDir[64];
	uint64_t have;
	uint64_t haveValid;
	float  availability;
	uint64_t  downloadedsize;
	uint64_t  uploadedsize;
	float ratio;
	char errorString[256];
        double  downloadSpeed;		// current download speed
        double  uploadSpeed;		// current upload speed
        int  uploadpeers;
        int  downloadpeers;
	double health;	//Health ratio
	uint32_t fileCount;
        uint32_t trackCount;
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_ed2k;

/*typedef struct INFO_BT{     //rtorrent
        char  proxy[MAX_SHORTNAMELEN];
        uint32_t  seed_time;
        uint32_t  npeers;
        uint32_t  dps;          // download pieces
        uint32_t  tps;          // total pieces
        uint32_t  aps;          // available pieces
        uint32_t  adr;          // average download speed
        uint32_t  aur;          // average upload speed
        uint32_t  cdr;          // current download speed
        uint32_t  cur;          // current upload speed
        uint32_t  rts;          // times of "refuse of error"
}Info_bt;*/
typedef struct INFO_FTP{
	char  proxy[MAX_SHORTNAMELEN];
	char  host[MAX_SHORTNAMELEN];
	uint32_t  port;
	char  user_name[MAX_SHORTLEN];
	char  password[MAX_SHORTLEN];
        double  elapsed;
        float  rate;  // K/s

        char Destination[MAX_NAMELEN];
        time_t  Created_time;
        char URL[MAX_NAMELEN];
        time_t Start_Time;
        char Time_Left[15];
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_ftp;

typedef struct INFO_HTTP{
        char  proxy[MAX_SHORTNAMELEN];
        double  elapsed;
        float  rate;  // K/s
        //add info
        char Destination[MAX_NAMELEN];
        time_t  Created_time;
        char URL[MAX_NAMELEN];
        //add status
        time_t Start_Time;
        char Time_Left[15]; //HH:MM:SS
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_http;

typedef struct INFO_NNTP{				//Allen NNTP
	char	  host[MAX_SHORTNAMELEN];
	char	  user_name[MAX_SHORTLEN];
	char	  password[MAX_SHORTLEN];
	uint32_t  ds;		// downloaded size
	uint32_t  ts;		// total size
	uint32_t  cdr;		// current download speed
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_nntp;

typedef struct INFO_NZB{
        double  elapsed;
        float  rate;  // K/s
        char Destination[MAX_NAMELEN];
        time_t  Created_time;
        time_t Start_Time;
        char Time_Left[15];
        uint32_t fileCount;
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_nzb;

typedef union FILE_INFO{
	Info_bt  iBt;
	Info_ftp  iFtp;
	Info_http  iHttp;
        Info_nzb  iNzb;
        Info_ed2k  iEd2k;
}File_info;

typedef struct NZB_FILE {
        char name[MAX_NAMELEN];
        uint64_t size;
        float percent_done;
}nzb_file;



#define INFO_SIZE  (sizeof(File_info));

typedef struct ADD_UNIT{
	uint8_t type;	// bt, ftp, http
	Padd  padd;
        struct ADD_UNIT  *next;
}AU;
#define AU_SIZE  (sizeof(AU))
typedef struct ADD_LIST{
        AU  *front;
        AU  *rear;
}Add_list;

Add_list add_list;

struct Lognote{
    int id;
    char url[2048];
    char real_url[2048];
    char type[10];
    int status;
    int checknum;
    char filenum[1024];
    char infohash[256];  //leo added for utility
    struct Lognote *next;
};

struct Lognote *head;

AU *heavy_jobs_head, *heavy_jobs_rear, *light_jobs_head, *light_jobs_rear;
//./pid_t on_heavy_jobs[MAX_ON_HEAVY], on_light_jobs[MAX_ON_LIGHT];
//char on_heavy_jobs[MAX_ON_HEAVY][MAX_HASHLEN], on_light_jobs[MAX_ON_LIGHT][MAX_HASHLEN];

//AU *nntp_jobs_head, *nntp_jobs_rear;		//Allen NNTP
//char on_nntp_jobs[MAX_ON_NNTP][MAX_HASHLEN];

struct Lognote *jqs_head, *jqs_rear;  
typedef struct LOG_STRUCT{
	char id[256];
        time_t  begin_t;
        time_t  end_t;
        time_t  now_t;
	char  fullname[MAX_NAMELEN];
        char  filename[MAX_SHORTNAMELEN];	// store the real download url(name is converted in http)
	char  store_dst[MAX_NAMELEN];		// store the real download url
        uint64_t  filesize;
        uint8_t  download_type; // BT, HTTP, FTP
        uint8_t  status;        // S_PROCESSING, S_PAUSED, S_COMPLETED, S_DEAD_OF_DELETED, S_DEAD_OF_ERROR
        int  error;             // S_DEAD_OF_ERROR
        float  progress;
        File_info  ifile;
}Log_struc;

typedef struct FILES_DETAIL
{
    char  filename[512];
    double  progress;
    uint64_t  filecompletedsize;
    uint64_t  filetotalsize;
}File_detail;

typedef struct TRACKER_STRUCT
{
    int downloadCount;
    uint8_t hasAnnounced;
    uint8_t hasScraped;
    char host[1024];
    char announce[1024];
    char scrape[1024];
    uint8_t isBackup;
    int announceState;
    int scrapeState;
    int lastAnnouncePeerCount;
    char lastAnnounceResult[128];
    time_t lastAnnounceStartTime;
    uint8_t lastScrapeSucceeded;
    uint8_t lastAnnounceTimedOut;
    time_t lastAnnounceTime;
    char lastScrapeResult[128];
    time_t lastScrapeStartTime;
    uint8_t lastAnnounceSucceeded;
    uint8_t lastScrapeTimedOut;
    time_t lastScrapeTime;
    int leecherCount;
    time_t nextAnnounceTime;
    time_t nextScrapeTime;
    int seederCount;
    int tier;
    uint32_t id;
}Tracker_struct;

typedef struct NZB_CONFIG
{
    char host[48];
    char port[24];
    char encryption[24];
    char username[64];
    char password[64];
    char connections[32];
    char downrate[24];

}Nzb_config;
                                                                                                                                            
wpd wpd_list[WPDNUM];

long uptime(void);
