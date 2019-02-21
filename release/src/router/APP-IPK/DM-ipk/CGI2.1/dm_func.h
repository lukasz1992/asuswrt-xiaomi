#ifndef DM_FUNC_H
#define DM_FUNC_H 1

#include"dm.h"

int tmp_enable, tmp_shour, tmp_smin, tmp_ehour, tmp_emin, tmp_day;
char nv_enable_time[4],nv_data[8], nv_time1[10], nv_time2[10];
char gen_serial[50],gen_product[10],gen_vonder[10];//eric added for disk umount-->mount and then diskname changes
int gen_partition;

typedef struct TM
{
int tm_min;
int tm_hour;
int tm_wday;
}tm;


struct datetime{
	char start[6];		// start time
	char stop[6];		// stop time
	char tmpstop[6];	// cross-night stop time
} __attribute__((packed));

void calculate_queue(struct Lognote *phead);
char *my_nstrchr(const char chr,char *str,int n);
int Sem_close(Sem_t *);
int chk_on_process(char *, char *);
void init_path();
//20120821 magic modify{
//char *getbasepath();
char *getrouterconfig();
char *getdmconfig();
//20120821 magic modify}
char *getlogid(char *);
int isOnProc(uint8_t status);
int read_log(Log_struc *, char *);
int remove_torrent(char*  , uint8_t );
void check_alive();
void Clear_log(char* );
int  Close_sem(char *);
int  Sem_open(Sem_t *, const char *, int, ... );
int  Sem_post(Sem_t *);
int  Sem_wait(Sem_t *);
void delet(char *s,int d);
//void char_to_ascii(char *output, char *input);
int check_download_time();
void small_sleep(float nsec);
int decode_path(char *url);
int detect_process(char * process_name);
int64_t check_disk_space(char *path);
static int in_sched(int now_mins, int now_dow, int sched_begin, int sched_end, int sched_begin2, int sched_end2, int sched_dow);  //2012.07.10 magic added for cross-night
int timecheck_item(char *activeDate, char *activeTime, char *activeTime2);  //2012.07.10 magic added for cross-night
struct disk_info{
    int port;
    int partitionport;
    char *product;
    char *vendor;
    char *serialnum;
    char *mountpath;
    char *diskname;
    char *diskpartition;
    struct disk_info *next;
};

int singledisk;
char dldir[100];

char router_timezone[64];

#define GENERALFILE "/opt/etc/dm2_general.conf"
#define LOGFILE "/tmp/dm2_log"

static char* str2upper(char *str);
static int uptime();

struct disk_info *follow_disk_info,*follow_disk_info_start,*follow_disk_tmp;
#endif
