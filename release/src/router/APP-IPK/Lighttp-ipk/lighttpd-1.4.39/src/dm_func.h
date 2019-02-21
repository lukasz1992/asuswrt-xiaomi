#ifndef _DM_FUNC_H_
#define _DM_FUNC_H_

#include "dm_hook.h"
#include "dm_url_parser.h"
#include "dm_http_parser.h"
#include "dm_ftp_parser.h"
#include "base.h"

static volatile sig_atomic_t srv_save_shutdown = 0;  //2011.07.27 magic added

int quenum;

int allow_download,allow_download_tmp;

int tmp_enable, tmp_shour, tmp_smin, tmp_ehour, tmp_emin, tmp_day;
char nv_enable_time[4],nv_data[8], nv_time1[10], nv_time2[10];

int redirect;
unsigned int login_ip;	// the logined ip
time_t login_timestamp;	// the timestamp of the logined ip
unsigned int login_ip_tmp;	// the ip of the current session.
unsigned int last_login_ip;	// the last logined ip


typedef struct TM
{
int tm_min;
int tm_hour;
int tm_wday;
}tm;

int pids(char *);
char *getrouterconfig();
char *getdmconfig();
//20120821 magic modify}
void init_path();
int Sem_open(Sem_t *sem, const char *pathname, int oflag, ... );
int Close_sem(char* logid);
int Sem_wait(Sem_t *sem);
int Sem_post(Sem_t *sem);
int Sem_close(Sem_t *sem);
int Jqs_create();
char *getlogid(const char *logname);
int isOnProc(uint8_t status);
int read_log(Log_struc *slog, char *id);
int remove_torrent(char *torrent_name);
void Clear_log(char* id);
void check_alive();
int chk_on_process(char *download_name, char *infohash);
void print_log(struct Lognote *phead);
void refresh_jqs();
int freelognote();
struct Lognote * createnote(int id, char *infohash, char *url, char *real_url, char *filenums, char *d_type, int status ,int times);
int addlognote(char *url, char *infohash, char *real_url, char *filenums, struct Lognote *phead ,char *d_type);
void dellognote(int id, struct Lognote *phead);
void insertnote(struct Lognote *note, struct Lognote *phead);
void initlognote(struct Lognote *phead);
void init_tmp_dst(char *dst, char *src, int len);
int DM_ADD(char* cmd, char* url ,char *infohash, char* real_url, char *filenums, char* d_type);
int insertnote2(struct Lognote *note);
void initlognote2();
int check_time_loop();
int DM_CTRL(char* cmd, char* task_id , char* d_type);
void ctrl_download();
int check_download_time();
//void http_login_cache(unsigned int ip);
void http_login_cache(connection *con);
void http_login(unsigned int ip);
int http_login_check(void);
void http_login_timeout(unsigned int ip);
void http_logout(unsigned int ip);
void run_onswap();
void run_offswap();
int detect_process(char * process_name);
int pids(char *appname);
static int in_sched(int now_mins, int now_dow, int sched_begin, int sched_end, int sched_begin2, int sched_end2, int sched_dow);  //2012.07.10 magic added for cross-night
int timecheck_item(char *activeDate, char *activeTime, char *activeTime2);  //2012.07.10 magic added for cross-night
//2012.07.31 magic added for Transfer Char to ASCII{
int char_to_ascii_safe(const char *output, const char *input, int outsize); 
void char_to_ascii(const char *output, const char *input);
int ascii_to_char_safe(const char *output, const char *input, int outsize);
void ascii_to_char(const char *output, const char *input);
const char *find_word(const char *s_buffer, const char *word);
int remove_word(char *s_buffer, const char *word);
//2012.07.31 magic added for Transfer Char to ASCII}
void calculate_queue(struct Lognote *phead);
int decode_url(char *passwd,int tag);

// use define to instead of long string
#define aMuleCMD "/opt/bin/dm2_amuled -c /tmp/APPS/DM2/Config/dm2_amule/ &"
#define niceCMD "/bin/nice -n 19 "
#define CPU3Taskset "/usr/bin/taskset -c 2 "
#define aMule_Lock "/tmp/APPS/DM2/Config/dm2_amule/muleLock"

void start_amule();

#ifdef DM_MIPSBIG
int check_mips_type();
#endif

#endif
