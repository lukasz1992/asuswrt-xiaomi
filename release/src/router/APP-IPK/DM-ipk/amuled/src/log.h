#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define LocalHost       "127.0.0.1"
#define Serv_Port       8936

#define MAX_NAMELEN  256
#define MAX_SHORTNAMELEN  128
#define MAX_SHORTLEN  32
#define SIZE_4G		(int64_t)4*1024*1024*1024

#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define HTTP    		0x60
#define FTP     		0x61
#define BT      		0x62
#define NZB      		0x63
#define NNTP      		0x64
#define ED2K      		0x65

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
#define S_NOTCOMPLETED           0x83    //nzb downloaded file is incomplete
#define S_DISKFULL				0x84     //disk is full
#define S_TYPE_OF_ERROR				0x85     //disk is Vfat File more than 4G
#define S_HASH                  0x86        //the flag for creating hash

char base_dir_path[MAX_NAMELEN];


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

typedef struct INFO_FTP{
	char  proxy[MAX_SHORTNAMELEN];
	char  host[MAX_SHORTNAMELEN];
	uint32_t  port;
	char  user_name[MAX_SHORTLEN];
	char  password[MAX_SHORTLEN];
	double  elapsed;
	float  rate;  // K/s
	char serial[200];
	char pid[5];
	char vid[5];
	int partitionnum;
}Info_ftp;

typedef struct INFO_HTTP{
	char  proxy[MAX_SHORTNAMELEN];
	double  elapsed;
	float  rate;  // K/s
	char serial[200];
	char pid[5];
	char vid[5];
	int partitionnum;
}Info_http;

typedef struct INFO_NZB{
	double  elapsed;
	float  rate;  // K/s
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

#define INFO_SIZE  (sizeof(File_info));

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

#define LOG_SIZE  (sizeof(Log_struc))

typedef struct TORRENT_FILES
{
	char  filename[512];
	double  progress;
	uint64_t  filecompletedsize;
	uint64_t  filetotalsize;
}Tr_files;
