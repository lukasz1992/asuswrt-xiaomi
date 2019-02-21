#include <sys/swap.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stddef.h>
#include "server.h"
#include "dm_func.h"

#ifdef USE_OPENSSL
# include <openssl/md5.h>
#else
# include "md5.h"
#endif

#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];
#ifdef USE_OPENSSL
#define IN const
#else
#define IN
#endif
#define OUT

void CvtHex(
        IN HASH Bin,
        OUT HASHHEX Hex
        );

/*pids()*/
enum {
	PSSCAN_PID      = 1 << 0,
	PSSCAN_PPID     = 1 << 1,
	PSSCAN_PGID     = 1 << 2,
	PSSCAN_SID      = 1 << 3,
	PSSCAN_UIDGID   = 1 << 4,
	PSSCAN_COMM     = 1 << 5,
	/* PSSCAN_CMD      = 1 << 6, - use read_cmdline instead */
	PSSCAN_ARGV0    = 1 << 7,
	/* PSSCAN_EXE      = 1 << 8, - not implemented */
	PSSCAN_STATE    = 1 << 9,
	PSSCAN_VSZ      = 1 << 10,
	PSSCAN_RSS      = 1 << 11,
	PSSCAN_STIME    = 1 << 12,
	PSSCAN_UTIME    = 1 << 13,
	PSSCAN_TTY      = 1 << 14,
	PSSCAN_SMAPS    = (1 << 15) * 0,
	PSSCAN_ARGVN    = (1 << 16) * 1,
	PSSCAN_START_TIME = 1 << 18,
	/* These are all retrieved from proc/NN/stat in one go: */
	PSSCAN_STAT     = PSSCAN_PPID | PSSCAN_PGID | PSSCAN_SID
	| PSSCAN_COMM | PSSCAN_STATE
	| PSSCAN_VSZ | PSSCAN_RSS
	| PSSCAN_STIME | PSSCAN_UTIME | PSSCAN_START_TIME
	| PSSCAN_TTY,
};

#define PROCPS_BUFSIZE 1024

static int read_to_buf(const char *filename, void *buf)
{
	int fd;
	/* open_read_close() would do two reads, checking for EOF.
		 * When you have 10000 /proc/$NUM/stat to read, it isn't desirable */
	int ret = -1;
	fd = open(filename, O_RDONLY);
	if (fd >= 0) {
		ret = read(fd, buf, PROCPS_BUFSIZE-1);
		close(fd);
	}
	((char *)buf)[ret > 0 ? ret : 0] = '\0';
	return ret;
}

void* xzalloc(size_t size)
{
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void* xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		perror("no memory");
	return ptr;
}

void* xrealloc_vector_helper(void *vector, unsigned sizeof_and_shift, int idx)
{
	int mask = 1 << (unsigned char)sizeof_and_shift;

	if (!(idx & (mask - 1))) {
		sizeof_and_shift >>= 8; /* sizeof(vector[0]) */
		vector = xrealloc(vector, sizeof_and_shift * (idx + mask + 1));
		memset((char*)vector + (sizeof_and_shift * idx), 0, sizeof_and_shift * (mask + 1));
	}
	return vector;
}

#define xrealloc_vector(vector, shift, idx) \
	xrealloc_vector_helper((vector), (sizeof((vector)[0]) << 8) + (shift), (idx))

typedef struct procps_status_t {
	DIR *dir;
	unsigned char shift_pages_to_bytes;
	unsigned char shift_pages_to_kb;
	/* Fields are set to 0/NULL if failed to determine (or not requested) */
	unsigned int argv_len;
	char *argv0;
	/* Everything below must contain no ptrs to malloc'ed data:
		 * it is memset(0) for each process in procps_scan() */
	unsigned long vsz, rss; /* we round it to kbytes */
	unsigned long stime, utime;
	unsigned long start_time;
	unsigned pid;
	unsigned ppid;
	unsigned pgid;
	unsigned sid;
	unsigned uid;
	unsigned gid;
	unsigned tty_major,tty_minor;
	char state[4];
	/* basename of executable in exec(2), read from /proc/N/stat
		 * (if executable is symlink or script, it is NOT replaced
		 * by link target or interpreter name) */
	char comm[16];
	/* user/group? - use passwd/group parsing functions */
} procps_status_t;

static procps_status_t* alloc_procps_scan(void)
{
	unsigned n = getpagesize();
	procps_status_t* sp = xzalloc(sizeof(procps_status_t));
	sp->dir = opendir("/proc");
	while (1) {
		n >>= 1;
		if (!n) break;
		sp->shift_pages_to_bytes++;
	}
	sp->shift_pages_to_kb = sp->shift_pages_to_bytes - 10;
	return sp;
}
void BUG_comm_size(void)
{
}
#define ULLONG_MAX     (~0ULL)
#define UINT_MAX       (~0U)

static unsigned long long ret_ERANGE(void)
{
	errno = ERANGE; /* this ain't as small as it looks (on glibc) */
	return ULLONG_MAX;
}
static unsigned long long handle_errors(unsigned long long v, char **endp, char *endptr)
{
	if (endp) *endp = endptr;

	/* errno is already set to ERANGE by strtoXXX if value overflowed */
	if (endptr[0]) {
		/* "1234abcg" or out-of-range? */
		if (isalnum(endptr[0]) || errno)
			return ret_ERANGE();
		/* good number, just suspicious terminator */
		errno = EINVAL;
	}
	return v;
}
unsigned bb_strtou(const char *arg, char **endp, int base)
{
	unsigned long v;
	char *endptr;

	if (!isalnum(arg[0])) return ret_ERANGE();
	errno = 0;
	v = strtoul(arg, &endptr, base);
	if (v > UINT_MAX) return ret_ERANGE();
	return handle_errors(v, endp, endptr);
}

const char* bb_basename(const char *name)
{
	const char *cp = strrchr(name, '/');
	if (cp)
		return cp + 1;
	return name;
}

static int comm_match(procps_status_t *p, const char *procName)
{
	int argv1idx;

	/* comm does not match */
	if (strncmp(p->comm, procName, 15) != 0)
		return 0;

	/* in Linux, if comm is 15 chars, it may be a truncated */
	if (p->comm[14] == '\0') /* comm is not truncated - match */
		return 1;

	/* comm is truncated, but first 15 chars match.
		 * This can be crazily_long_script_name.sh!
		 * The telltale sign is basename(argv[1]) == procName. */

	if (!p->argv0)
		return 0;

	argv1idx = strlen(p->argv0) + 1;
	if (argv1idx >= p->argv_len)
		return 0;

	if (strcmp(bb_basename(p->argv0 + argv1idx), procName) != 0)
		return 0;

	return 1;
}

void free_procps_scan(procps_status_t* sp)
{
	closedir(sp->dir);
	free(sp->argv0);
	free(sp);
}

procps_status_t* procps_scan(procps_status_t* sp, int flags)
{
	struct dirent *entry;
	char buf[PROCPS_BUFSIZE];
	char filename[sizeof("/proc//cmdline") + sizeof(int)*3];
	char *filename_tail;
	long tasknice;
	unsigned pid;
	int n;
	struct stat sb;

	if (!sp)
		sp = alloc_procps_scan();

	for (;;) {
		entry = readdir(sp->dir);
		if (entry == NULL) {
			free_procps_scan(sp);
			return NULL;
		}
		pid = bb_strtou(entry->d_name, NULL, 10);
		if (errno)
			continue;

		/* After this point we have to break, not continue
				 * ("continue" would mean that current /proc/NNN
				 * is not a valid process info) */

		memset(&sp->vsz, 0, sizeof(*sp) - offsetof(procps_status_t, vsz));

		sp->pid = pid;
		if (!(flags & ~PSSCAN_PID)) break;

		filename_tail = filename + sprintf(filename, "/proc/%d", pid);

		if (flags & PSSCAN_UIDGID) {
			if (stat(filename, &sb))
				break;
			/* Need comment - is this effective or real UID/GID? */
			sp->uid = sb.st_uid;
			sp->gid = sb.st_gid;
		}

		if (flags & PSSCAN_STAT) {
			char *cp, *comm1;
			int tty;
			unsigned long vsz, rss;

			/* see proc(5) for some details on this */
			strcpy(filename_tail, "/stat");
			n = read_to_buf(filename, buf);
			if (n < 0)
				break;
			cp = strrchr(buf, ')'); /* split into "PID (cmd" and "<rest>" */
			/*if (!cp || cp[1] != ' ')
								break;*/
			cp[0] = '\0';
			if (sizeof(sp->comm) < 16)
				BUG_comm_size();
			comm1 = strchr(buf, '(');
			/*if (comm1)*/
			strncpy(sp->comm, comm1 + 1, sizeof(sp->comm));

			n = sscanf(cp+2,
					   "%c %u "               /* state, ppid */
					   "%u %u %d %*s "        /* pgid, sid, tty, tpgid */
					   "%*s %*s %*s %*s %*s " /* flags, min_flt, cmin_flt, maj_flt, cmaj_flt */
					   "%lu %lu "             /* utime, stime */
					   "%*s %*s %*s "         /* cutime, cstime, priority */
					   "%ld "                 /* nice */
					   "%*s %*s "             /* timeout, it_real_value */
					   "%lu "                 /* start_time */
					   "%lu "                 /* vsize */
					   "%lu "                 /* rss */
					   /*	"%lu %lu %lu %lu %lu %lu " rss_rlim, start_code, end_code, start_stack, kstk_esp, kstk_eip */
					   /*	"%u %u %u %u "         signal, blocked, sigignore, sigcatch */
					   /*	"%lu %lu %lu"          wchan, nswap, cnswap */
					   ,
					   sp->state, &sp->ppid,
					   &sp->pgid, &sp->sid, &tty,
					   &sp->utime, &sp->stime,
					   &tasknice,
					   &sp->start_time,
					   &vsz,
					   &rss);
			if (n != 11)
				break;
			/* vsz is in bytes and we want kb */
			sp->vsz = vsz >> 10;
			/* vsz is in bytes but rss is in *PAGES*! Can you believe that? */
			sp->rss = rss << sp->shift_pages_to_kb;
			sp->tty_major = (tty >> 8) & 0xfff;
			sp->tty_minor = (tty & 0xff) | ((tty >> 12) & 0xfff00);

			if (sp->vsz == 0 && sp->state[0] != 'Z')
				sp->state[1] = 'W';
			else
				sp->state[1] = ' ';
			if (tasknice < 0)
				sp->state[2] = '<';
			else if (tasknice) /* > 0 */
				sp->state[2] = 'N';
			else
				sp->state[2] = ' ';

		}

		if (flags & (PSSCAN_ARGV0|PSSCAN_ARGVN)) {
			free(sp->argv0);
			sp->argv0 = NULL;
			strcpy(filename_tail, "/cmdline");
			n = read_to_buf(filename, buf);
			if (n <= 0)
				break;
			if (flags & PSSCAN_ARGVN) {
				sp->argv_len = n;
				sp->argv0 = malloc(n + 1);
				memcpy(sp->argv0, buf, n + 1);
				/* sp->argv0[n] = '\0'; - buf has it */
			} else {
				sp->argv_len = 0;
				sp->argv0 = strdup(buf);
			}
		}
		break;
	}
	return sp;
}

pid_t* find_pid_by_name(const char *procName)
{
	pid_t* pidList;
	int i = 0;
	procps_status_t* p = NULL;

	pidList = xzalloc(sizeof(*pidList));
	while ((p = procps_scan(p, PSSCAN_PID|PSSCAN_COMM|PSSCAN_ARGVN))) {
		if (comm_match(p, procName)
				/* or we require argv0 to match (essential for matching reexeced /proc/self/exe)*/
				|| (p->argv0 && strcmp(bb_basename(p->argv0), procName) == 0)
				/* TOOD: we can also try /proc/NUM/exe link, do we want that? */
				) {
			if (p->state[0] != 'Z')
			{
				pidList = xrealloc_vector(pidList, 2, i);
				pidList[i++] = p->pid;
			}
		}
	}

	pidList[i] = 0;
	return pidList;
}

int pids(char *appname)
{
	pid_t *pidList;
	pid_t *pl;
	int count = 0;

	pidList = find_pid_by_name(appname);
	for (pl = pidList; *pl; pl++) {
		count++;
	}
	free(pidList);

	if (count)
		return 1;
	else
		return 0;
}
/*end pids()*/

#include <sys/sysinfo.h>
long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

char *getrouterconfig()
{
    //2016.8.16 tina add{
    char *result = NULL;
    result = (char *)malloc(sizeof(char)*64);
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(char)*64);
    //}end tina

    if (access("/tmp/asus_router.conf",0) == 0)
    {
        memset(lan_ip_addr, '\0', sizeof(lan_ip_addr));
        int fd, len, i=0;
        //2016.8.16 tina modify{
        //char ch, tmp[256], name[256], content[256] ,result[64];
        char ch, tmp[256], name[256], content[256];
        //}end tina
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        //memset(result, 0, sizeof(result));//2016.8.16 tina modify

        if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
        }
        else
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "BASE_PATH"))
                    {
                        sprintf(result, "%s", content);

                    }
                    else if(!strcmp(name, "LAN_IP"))
                    {
                        sprintf(lan_ip_addr, "%s", content);
                    }
                    else if(!strcmp(name, "PRODUCTID"))
                    {
                        sprintf(router_name, "%s", content);

                    }
                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            return result;
        }
    }
    else
        //2016.8.16 tina modify
    {
        //return "none";
        sprintf(result,"%s","none");
        return result;
    }
    //end tina
}


char *getdmconfig()
{    
    //2016.8.16 tina add{
    char *result = NULL;
    result = (char *)malloc(sizeof(char)*64);
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(char)*64);
    //}end tina
    if ((access("/tmp/APPS/DM2/Config/dm2_general.conf",0) == 0) || (access("/opt/etc/dm2_general.conf",0) == 0))
    {
        memset(nv_enable_time, '\0', sizeof(nv_enable_time));
        memset(nv_data, '\0', sizeof(nv_data));
        memset(nv_time1, '\0', sizeof(nv_time1));
        memset(nv_time2, '\0', sizeof(nv_time2));
        memset(Download_dir_path, '\0', sizeof(Download_dir_path));
        int fd, len, i=0;
        //2016.8.16 tina modify{
        //char ch, tmp[256], name[256], content[256] ,result[64];
        char ch, tmp[256], name[256], content[256];
        //}end tina
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        //memset(result, 0, sizeof(result)); //2016.8.16 tina modify
        sprintf(result, "1");
        if((fd = open("/tmp/APPS/DM2/Config/dm2_general.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
            fd = open("/opt/etc/dm2_general.conf", O_RDONLY | O_NONBLOCK);
        }
        if(fd>0)
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "Enable_time"))
                    {
                        sprintf(nv_enable_time, "%s", content);
                    }
                    else  if(!strcmp(name, "Day"))
                    {
                        sprintf(nv_data, "%s", content);
                    }
                    else  if(!strcmp(name, "Download_dir"))
                    {
                        sprintf(Download_dir_path, "%s", content);
                    }
                    else  if(!strcmp(name, "MAX_ON_HEAVY"))
                    {
                        MAX_ON_HEAVY = atoi(content);
                    }
                    else  if(!strcmp(name, "MAX_QUEUES"))
                    {
                        MAX_QUEUES = atoi(content);
                    }
                    else  if(!strcmp(name, "MAX_ON_ED2K"))
                    {
                        MAX_ON_ED2K = atoi(content);
                    }
                    else  if(!strcmp(name, "dm_radio_time_x"))
                    {
                        sprintf(nv_time1, "%s", content);
                    }
                    else  if(!strcmp(name, "dm_radio_time2_x"))
                    {
                        sprintf(nv_time2, "%s", content);
                    }

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            return result;
        }
    }
    else
        //2016.8.16 tina modify
    {
        //return "none";
        sprintf(result,"%s","none");
        return result;
    }
    //end tina
}

void init_path()
{
    memset(Base_dir, '\0', sizeof(Base_dir));
    memset(Share_dir, '\0', sizeof(Share_dir));
    //memset(sem_path, '\0', sizeof(sem_path));
    memset(In_dir, '\0', sizeof(In_dir));
    memset(Seeds_dir, '\0', sizeof(Seeds_dir));
    memset(Log_dir, '\0', sizeof(Log_dir));
    memset(Sem_dir, '\0', sizeof(Sem_dir));
    memset(jqs_dir, '\0', sizeof(jqs_dir));
    memset(jqs_file, '\0', sizeof(jqs_file));
#ifdef DM_I686
    memset(jqs_file_bak, '\0', sizeof(jqs_file_bak));
#endif

    /*char pool_tmp[128];
	char share_tmp[128];
	strcpy(pool_tmp,nvram_safe_get(NVVAL_POOL));
	strcpy(share_tmp,nvram_safe_get(NVVAL_SHARE));
	sprintf(Share_dir, "/shares/%s/%s/%s", pool_tmp, share_tmp, DOWNLOAD_FOLDER);
	//sprintf(Share_dir, "/shares/%s/%s/%s", nvram_safe_get(NVVAL_POOL), nvram_safe_get(NVVAL_SHARE), DOWNLOAD_FOLDER);
	*/

    //sprintf(Base_dir, "%s", getbasepath()); //20120821 magic modify for new config
    //2016.8.16 tina modify
    //sprintf(Base_dir, "%s", getrouterconfig());
    char *routerconfig = getrouterconfig();
    if(routerconfig != NULL)
    {
        sprintf(Base_dir, "%s", routerconfig);
        free(routerconfig);
    }
    //end tina
    sprintf(Share_dir, "%s/Download2", Base_dir);;
    //sprintf(Share_dir, "%s", "/tmp/harddisk/part0/snarf/");
    //printf("init_path share_path:%s\n",Share_dir);
    sprintf(In_dir, "%s/InComplete", Share_dir);
    sprintf(Seeds_dir, "%s/Seeds", Share_dir);
    sprintf(Log_dir, "%s/.logs", Share_dir);
    //strcpy(sem_path,"/shares/dmathined/Download");
    sprintf(Sem_dir, "%s/.sems", Share_dir);
    sprintf(jqs_dir, "%s/.sems/sem.jqs", Share_dir);
    sprintf(jqs_file, "%s/dm.jqs", Log_dir);  //2013.11.20 magic for test
#ifdef DM_I686
    sprintf(jqs_file_bak, "%s/dm.jqs.bak", Log_dir);
#endif
    //sprintf(jqs_file, "%s/dm.jqs", "/tmp/APPS/DM2/.logs");    //2013.11.20 magic for test
}

int Sem_open(Sem_t *sem, const char *pathname, int oflag, ... )
{

    int     i, flags, save_errno;
    char    c;
    mode_t  mode;
    va_list ap;
    unsigned int    value = 0;

    if (oflag & O_CREAT) {
        va_start(ap, oflag);            // init ap to final named argument
        mode = va_arg(ap, mode_t);
        value = va_arg(ap, unsigned int);
        va_end(ap);

        if (mkfifo(pathname, mode) < 0) {
            if (errno == EEXIST && (oflag & O_EXCL) == 0)
                oflag &= ~O_CREAT;      // already exists, OK
            else{
                //perror("sem mkfifo :");

                return -1;
            }
        }
    }

    // INDENT-OFF
    if(sem == NULL)
    {

        return -1;
    }


    memset(sem, '\0', sizeof(Sem_t));
    sem->sem_fd[0] = sem->sem_fd[1] = -1;

    if ( (sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0)
        goto error;
    if ( (sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0)
        goto error;
    // INDENT-ON


    if ( (flags = fcntl(sem->sem_fd[0], F_GETFL, 0)) < 0)
        goto error;
    flags &= ~O_NONBLOCK;
    if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0)
        goto error;

    if (oflag & O_CREAT) {          // initialize semaphore
        for (i = 0; i < value; i++)
            if (write(sem->sem_fd[1], &c, 1) != 1)
                goto error;
    }

    sem->sem_magic = SEM_MAGIC;

    return 0;

    error:

    save_errno = errno;
    if (oflag & O_CREAT)
        unlink(pathname);               // if we created FIFO
    close(sem->sem_fd[0]);          // ignore error
    close(sem->sem_fd[1]);          // ignore error
    memset(sem, '\0',sizeof(Sem_t));
    errno = save_errno;
    //perror("sem error :");

    return -1;
}


int Close_sem(char* logid)
{
    int  i;

    for(i=0; i<MAX_NSEM; i++)
        //./if(Sem[i].logid == logid)
        if(strcmp(Sem[i].logid, logid)==0)
            break;

    if(i == MAX_NSEM){
        return -1;
    }

    if(Sem_close(&Sem[i]) < 0)
        return -1;

    return 0;
}

int Sem_wait(Sem_t *sem)
{

    char    c;

    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    if (read(sem->sem_fd[0], &c, 1) == 1)
        return(0);

    return(-1);
}  

int Sem_post(Sem_t *sem)
{

    char    c;

    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    if (write(sem->sem_fd[1], &c, 1) == 1)
        return(0);
    return(-1);
}       

int Sem_close(Sem_t *sem)
{
    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    sem->sem_magic = 0;             // in case caller tries to use it later

    if (close(sem->sem_fd[0]) == -1 || close(sem->sem_fd[1]) == -1)
    {
        memset(sem, '\0', sizeof(Sem_t));
        return(-1);
    }

    memset(sem, '\0', sizeof(Sem_t));

    return(0);
}


int Jqs_create(){
    int fd, flag = O_CREAT|O_EXCL;

    memset(jqs_file, '\0', 256);
    sprintf(jqs_file, "%s%s/dm.jqs", Base_dir,"/Download2/.logs"); //2013.11.20 magic for test
    //sprintf(jqs_file, "%s/dm.jqs", "/tmp/APPS/DM2/.logs"); //2013.11.20 magic for test
    //unlink(jqs_file);
    if((fd = open(jqs_file, flag)) <= 0){
        if(errno == EEXIST)
            return -2;
        return -1;
    }

    close(fd);
    return 0;
}

char *getlogid(const char *logname)
{
    char *ptr;

    if((ptr = strstr(logname, "snarf_")) != NULL){
        //ptr += 6;
    }
    else if((ptr = strstr(logname, "transm_")) != NULL){
        //ptr += 7;
    }
    else if((ptr = strstr(logname, "nzb_")) != NULL){
        //ptr += 4;
    }
    else if((ptr = strstr(logname, "ed2k_")) != NULL){
        //ptr += 4;
    }
    else{
        return "none";
    }

    //return (atoi(ptr));
    return ptr;
}

int isOnProc(uint8_t status){

    return ((status == S_INITIAL)||(status == S_PROCESSING)||(status == S_PAUSED)||(status == S_COMPLETED)||(status == S_DISKFULL)||(status == S_NEEDACCOUNT)||(status == S_CONNECTFAIL)||(status == S_SSLFAIL)||(status == S_ACCOUNTFAIL)||(status == S_RECVFAIL)||(status == S_SEEDING)||(status == S_HASH)) ? (1) : (0);
}

int read_log(Log_struc *slog, char *id)
{
    Sem_t  sem;
    int  use_sem = 1;
    int  fd, flags, n;
    char  logname[MAX_NAMELEN];
    char  semname[MAX_NAMELEN];
    int is_bt;
    is_bt = (strlen(id)>5) ? 1 : 0;           // 2009.03 SZ-Alicia modified for HTTP/FTP's pid
    // test

    memset(logname, '\0',sizeof(logname));
    memset(semname, '\0',sizeof(semname));
    sprintf(logname, "%s/%s", Log_dir, id);
    sprintf(semname, "%s/sem.%s", Sem_dir, id);
    flags = O_RDONLY;
    /*if(Sem_open(&sem, semname, flags) == -1)
	{
		use_sem = 0;
	}

	if(use_sem)
		Sem_wait(&sem);*/

    // read each log to slog
    if((fd = open(logname, O_RDONLY)) > 0)
    {
        if((n = read(fd, slog, sizeof(Log_struc))) == 1)
            slog->status = S_INITIAL;
        close(fd);
    }
    else
    {
        //perror("open log error");
        return -1;
    }
    if(use_sem)
        Sem_post(&sem);

    Sem_close(&sem);
    // test

    return 1;
}


int remove_torrent(char *torrent_name)
{
    char torrent_pos[MAX_NAMELEN];
    memset(torrent_pos, '\0', sizeof(torrent_pos));
    //sprintf(torrent_pos, "%s/%s", In_dir, torrent_name);
    sprintf(torrent_pos, "%s/%s", Seeds_dir, torrent_name);
    unlink(torrent_pos);
    return 0;
}

void Clear_log(char* id)
{
    char  delname[MAX_NAMELEN];

    memset(delname, '\0', sizeof(delname));
    sprintf(delname, "%s/.sems/sem.%s", Share_dir, id);
    unlink(delname);

    memset(delname, '\0', sizeof(delname));
    sprintf(delname, "%s/.logs/%s", Share_dir, id);
    unlink(delname);

    if(Close_sem(id) < 0){
        //fprintf(stderr, "close sem %s fail\n", id);
    }
}

void check_alive()
{	// check on_heavy_jobs and on_light_jobs
    DIR *log_dir;
    struct dirent *log_ptr;
    Log_struc slog;
    char pid[MAX_HASHLEN]={0};

    if((log_dir = opendir(Log_dir)) == NULL){
        return;
    }

    on_heavy_counts = 0;
    on_light_counts = 0;
    on_nntp_counts = 0;			//Allen NNTP
    on_ed2k_counts = 0;
    on_heavy_seeding_counts=0;
    on_heavy_hashing_counts=0;

    while((log_ptr = readdir(log_dir)) != NULL)
    {
        sprintf(pid, "%s" , getlogid(log_ptr->d_name));
        if((strcmp(pid,"none"))&&(strncmp(pid,"-",1)!=0))
        {
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0)
            {
                if(isOnProc(slog.status))
                {
                    if((slog.download_type == BT) && slog.progress < 1 ){
						on_heavy_counts++;
                        if(slog.status==S_HASH){
                            on_heavy_hashing_counts++;
                        }
                    }
                    else if(slog.download_type == BT) {
                        if(slog.status==S_SEEDING){
                            on_heavy_seeding_counts++;
                        }
                    }
					else if(((slog.download_type == HTTP) || \
							 (slog.download_type == FTP)) && slog.progress < 1){
                        on_light_counts++;
                    }
					else if((slog.download_type == NZB) && slog.progress < 1 ){
                        on_nntp_counts++;
                    }
					else if((slog.download_type == ED2K) && slog.progress < 1 ){
                        on_ed2k_counts++;
                    }
                }
                if(((strstr(pid, "transm_")) != NULL) && slog.status==0)
                {
                    on_heavy_counts++;
                }
                if(((strstr(pid, "snarf_")) != NULL) && slog.status==0)
                {
                    on_light_counts++;
                }
                if(((strstr(pid, "nzb_")) != NULL) && slog.status==0)
                {
                    on_nntp_counts++;
                }
                if(((strstr(pid, "ed2k_")) != NULL) && slog.status==0)
                {
                    on_ed2k_counts++;
                }
            }
            else
            {
                Clear_log(pid);
            }

        }
    }
    closedir(log_dir);

}

char *right(char *dst,char *src, int n)
{
    char *p=src;
    char *q =dst;
    int len =strlen(src);
    if(n>len) n=len;
    p+=(len-n);
    while(*(q++) = *(p++));
    return dst;
}


int chk_on_process(char *download_name, char *infohash){ // download_name: bt is torrent's filename, http is the filename in InComplete
    DIR *log_dir;
    struct dirent *log_ptr;
    Log_struc slog;
    //./int pid;
    char pid[MAX_HASHLEN]={0};

    //printf("chk on process\n");	// tmp test
    if((log_dir = opendir(Log_dir)) == NULL)
    {
        return 1;
    }

    while((log_ptr = readdir(log_dir)) != NULL){
        sprintf(pid, "%s" , getlogid(log_ptr->d_name));
        if((strcmp(pid,"none"))&&(strncmp(pid,"-",1)!=0)){
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0){
                if(isOnProc(slog.status)){
                    if(slog.download_type == BT){
                        //if((!download_name) || (!slog.filename) || (!infohash) || (!slog.store_dst))
                        if((!download_name) || (!slog.filename) || (!slog.store_dst))
                        {
                            continue;
                        }
			

                        if(strstr(download_name,slog.id)!=NULL){
                            closedir(log_dir);
                            return 1;
                        }else{
                        }

                        if(strncmp(download_name, slog.filename, strlen(download_name)) == 0){
                            closedir(log_dir);
                            return 1;
                        }
                        if(infohash){
                            if(strncmp(infohash, slog.id, strlen(infohash)) == 0){
                                remove_torrent(download_name);	//Allen 20090826
                                closedir(log_dir);
                                return 1;
                            }
                        }
                    }else if((slog.download_type == HTTP) || (slog.download_type == FTP)){
                        if((!download_name) || (!slog.filename))
                            continue;
                        if(strncmp(download_name, slog.filename, strlen(download_name)) == 0){
                            // test
                            closedir(log_dir);
                            return 1;
                        }
                    }else if(slog.download_type == NZB){
                        if((!download_name) || (!slog.fullname))
                            continue;
                        if(strncmp(download_name, slog.fullname, strlen(download_name)) == 0){
                            // test
                            closedir(log_dir);
                            return 1;
                        }
                    }
                    else if(slog.download_type == ED2K){
                        char infohash_tmp[1024];
                        memset(infohash_tmp,0,sizeof(infohash_tmp));
                        right(infohash_tmp,download_name,34);
                        //fprintf(stderr,"\ninfohash_tmp=%s\n",infohash_tmp);
                        if(strncmp(infohash_tmp, slog.id, strlen(infohash_tmp)-2) == 0){
                            closedir(log_dir);
                            return 1;
                        }

                    }
                }
            }
        }
    }
    closedir(log_dir);
    return 0;
}


void print_log(struct Lognote *phead)
{
    struct Lognote *au = (struct Lognote*)0;
    for(au=phead; au; au=au->next){
        fprintf(stderr, "\nprint_log\n");
        fprintf(stderr,"id = %d\n", au->id);
        fprintf(stderr,"url = %s\n", au->url);
        fprintf(stderr,"type = %s\n", au->type);
    }
}

void calculate_queue(struct Lognote *phead)
{
    heavy_queue=0;
    light_queue=0;
    nntp_queue=0;
    ed2k_queue=0;
    struct Lognote *au = (struct Lognote*)0;
    for(au=phead; au; au=au->next){
        if(!strcmp(au->type,"3")){
            heavy_queue++;
        }
        if(!strcmp(au->type,"1")||!strcmp(au->type,"2")){
            light_queue++;
        }
        if(!strcmp(au->type,"4")){
            nntp_queue++;
        }
        if(!strcmp(au->type,"6")){
            ed2k_queue++;
        }
    }
}
void refresh_jqs(){
    int fd;
    int flag = O_CREAT|O_RDWR|O_TRUNC;
    struct Lognote *pau = (struct Lognote*)0;

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);

    if((fd = open(jqs_file, flag)) < 0){
        if(jqs_sem_use)
            Sem_post(&jqs_sem);
        return;
    }

    for(pau = head; pau; pau = pau->next){
        if(strcmp(pau->url,"")){
            write(fd, pau, sizeof(struct Lognote));
        }
        else{

        }
    }
    close(fd);
    if(jqs_sem_use)
        Sem_post(&jqs_sem);
}

int freelognote()
{
    struct Lognote *p1, *p2;
    p2 = head;
    while(p2)
    {
        p1=p2->next;
        free(p2);
        p2=p1;
    }
    return 1;
}

/* create a notet*/
//struct Lognote * createnote(int id, char *url, int status)
struct Lognote * createnote(int id, char *infohash,char *url, char *real_url,char *filenums, char *d_type, int status ,int times)
{
    struct Lognote *note = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(note, 0, sizeof(struct Lognote));
    note->id = id;
    strcpy(note->url, url);
    strcpy(note->infohash, infohash);
    strcpy(note->real_url, real_url);
    strcpy(note->type, d_type);
    strcpy(note->filenum, filenums);
    note->status = status;
    note->checknum = times;
    return note;
}

/* create and add a note to list*/
int addlognote(char *url,char *infohash, char *real_url, char *filenums, struct Lognote *phead ,char *d_type)
{
    int i = 1;
    int j = 1;
    //    int isInsert = 0;
    struct Lognote *p1, *p2;
    p2 = phead;
    p1 = phead->next;

    while(p1 != NULL)
    {
        //if(p1->id != i)
        //{
        //    break;
        //}

        //i++;
	j = p1->id;
	i = ++j;
        p2 = p1;
        p1 = p1->next;
    }

    if(!strncmp(url,"magnet:",7)){
        strcpy(real_url, url);
    }

    struct Lognote *note = createnote(i, infohash,url, real_url, filenums, d_type, S_NOTBEGIN , 1);

    p2->next = note;
    note->next = p1;

    //refresh_jqs(p2);
    refresh_jqs();
    //print_log(head);
    //if(note){
    //	free(note);
    //}
    return 1;
}

/* add -p by gauss */
void dellognote(int id, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(p1->id == id)
        {
            p2->next = p1->next;
            free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    //refresh_jqs(p2);
    refresh_jqs();
    //print_log(p2);
}

/* insert a note to list*/
void insertnote(struct Lognote *note, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p2 = phead;
    p1 = phead->next;

    while(p1 != NULL)
    {
        if((note->id < p1->id) && (note->id > p2->id))
        {
            break;
        }
        p2 = p1;
        p1 = p1->next;
    }

    p2->next = note;
    note->next = p1;
}

void initlognote(struct Lognote *phead)
{

    int fd_r;
    struct Lognote au;
    jqs_sem_use = 1;
    /* if(Sem_open(&jqs_sem, jqs_dir, 0) < 0)
	{
		jqs_sem_use = 0;
	}

	if(jqs_sem_use)
                Sem_wait(&jqs_sem);*/
    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {

        return;
    }

    memset(&au, 0, sizeof(struct Lognote));
    quenum=0;
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {
        if(strcmp(au.url,"")){
            struct Lognote *note = createnote(au.id, au.infohash,au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
            insertnote(note, phead);
            if(!strcmp(au.type,"5"))
            {
                quenum++;
            }
            memset(&au, 0, sizeof(struct Lognote));
        }
    }
    close(fd_r);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);
}

void init_tmp_dst(char *dst, char *src, int len){
    int i;

    for(i=len-1; i>=0; --i){
        if(src[i] == '/')
            break;
    }
    strncpy(dst, src+(i+1), len-(i+1));
}

int DM_ADD(char* cmd, char* url ,char *infohash, char* real_url, char *filenums, char* d_type)
{	    
    int sfd;
    int result = UNKNOW;
    char command[2048];
    struct sockaddr_in btaddr;
    char chk_tmp[strlen(real_url)+1];

    memset(command,0,sizeof(command));
    sprintf(command,"%s%s",cmd,real_url);
    memset(chk_tmp, 0, sizeof(chk_tmp));

    memset(&btaddr, 0, sizeof(btaddr));
	if(total_heavy_counts + total_light_counts + total_nntp_counts < MAX_QUEUES){
        btaddr.sin_family = AF_INET;
		if(!strcmp(d_type, "3")){
			if((on_heavy_counts + on_ed2k_counts< MAX_ON_HEAVY) \
			&& (on_heavy_hashing_counts<1) ) {
				init_tmp_dst(chk_tmp, real_url, strlen(real_url));
				if(chk_on_process(chk_tmp, NULL) > 0){
					return BT_EXIST;
				}else{
					btaddr.sin_port = htons(BT_PORT);
				}
			}
			else{
				addlognote(url,infohash,real_url,filenums,head,d_type);
				return HEAVY_FULL;
			}
		}
	else if(!strcmp(d_type, "1") || !strcmp(d_type, "2")){
            if(on_light_counts < MAX_ON_LIGHT){
                if(!strcmp(d_type, "2")){
                    init_tmp_dst(chk_tmp, real_url, strlen(real_url));
                    if(chk_on_process(chk_tmp, NULL) > 0)
                        return BT_EXIST;
                } else if(!strcmp(d_type, "1")){
                    init_tmp_dst(chk_tmp, real_url, strlen(real_url));
                    if(chk_on_process(chk_tmp, NULL) > 0)
                        return BT_EXIST;
                }

                btaddr.sin_port = htons(SNARF_PORT);
            }
            else{
                addlognote(url,infohash,real_url,filenums,head,d_type);
                return LIGHT_FULL;
            }
	}
	else if(!strcmp(d_type, "4")){
            if(on_nntp_counts < MAX_ON_NNTP){
                init_tmp_dst(chk_tmp, real_url, strlen(real_url));
                if(chk_on_process(chk_tmp, NULL) > 0){
                    return BT_EXIST;
                }
                else{
                    btaddr.sin_port = htons(NZB_PORT);
                }
            }
            else{
                addlognote(url,infohash,real_url,filenums,head,d_type);
                return NNTP_FULL;
            }
	} else if(!strcmp(d_type, "6")) {
		if((on_heavy_counts + on_ed2k_counts< MAX_ON_ED2K) \
				&& (on_heavy_hashing_counts == 0) ){
			if(chk_on_process(url, NULL) > 0){
				return BT_EXIST;
			} else {
				char command1[1024];
				memset(command1,'\0',sizeof(command1));
				sprintf(command1,"/opt/bin/dm2_amulecmd -h %s -P admin -c \" add %s \"", \
						"127.0.0.1",real_url);
				system(command1);
				return ACK_SUCESS;
			}
		}
		else{
			addlognote(url,infohash,real_url,filenums,head,d_type);
			return ED2K_FULL;
		}
	}
        inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
	result =  ACK_SUCESS;

        if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            //printf("ACK_FAIL1");
            result = ACK_FAIL;
        }

        if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
            //printf("ACK_FAIL2");
            result =  ACK_FAIL;
        }

        if(write(sfd, command, strlen(command)) != strlen(command))
        {
            //printf("ACK_FAIL3");
            result =  ACK_FAIL;
        }

        close(sfd);
        sleep(1);
	return result;
    }
    else{
	return TOTAL_FULL;
    }

}

/* insert a note to list
NOTE: delete the task from the file before task was really removed
*/
int insertnote2(struct Lognote *note)
{
    int ctrl_result=0;

    if(note->status==S_NOTBEGIN){

	if(!strcmp(note->type, "1") || !strcmp(note->type, "2")){
            dellognote(note->id,head);
            ctrl_result=DM_ADD("add@0@",note->url,note->infohash,note->real_url,note->filenum,note->type);
	}
	if(!strcmp(note->type, "6")){
            dellognote(note->id,head);
            ctrl_result=DM_ADD("add@0@",note->url,note->infohash,note->real_url,note->filenum,note->type);
	}
	else if(!strcmp(note->type, "4")){
            dellognote(note->id,head);
            char nzbfile_com[512];
            memset(nzbfile_com,0,sizeof(nzbfile_com));
            char *p;
            p=strstr(note->real_url,"://");
            if(p == NULL){
                sprintf(nzbfile_com,"add@1@%s/Download2/Seeds/",Base_dir);//20120821 magic for new config
            }
            else{
                sprintf(nzbfile_com,"%s","add@0@");
            }
            ctrl_result=DM_ADD(nzbfile_com,note->url,note->infohash,note->real_url,note->filenum,note->type);
	}
	else if(!strcmp(note->type, "3")){
            dellognote(note->id,head);
            if(!strncmp(note->url,"magnet:",7)){
                //ctrl_result=DM_ADD("add@0@",note->url,note->type);
                ctrl_result=DM_ADD("add@0@",note->url,note->infohash,note->real_url,note->filenum,note->type);
            }
            else{
                char btfile_com[512];
                memset(btfile_com,0,sizeof(btfile_com));
                char *p;
                p=strstr(note->real_url,"://");
                if(p == NULL){
                    if(!strncmp(note->filenum,"nonum",5)){
                        //sprintf(btfile_com,"add@1@%s/Download2/Seeds/",getbasepath());
                        sprintf(btfile_com,"add@1@All@%s/Download2/Seeds/",Base_dir);//20120821 magic for new config
                    }
                    else{
                        sprintf(btfile_com,"add@1@%s@%s/Download2/Seeds/",note->filenum,Base_dir);//20120821 magic for new config
                    }
                }
                else{
                    sprintf(btfile_com,"%s","add@0@");
                }
                ctrl_result=DM_ADD(btfile_com,note->url,note->infohash,note->real_url,note->filenum,note->type);
            }
	}
    }
    return ctrl_result;
}
/*
	read the task from jqs_file and add it to task list.

  */
void initlognote2()
{
    int fd_r;
    struct Lognote au;
    jqs_sem_use = 1;

    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {
        return;
    }

    memset(&au, 0, sizeof(struct Lognote));
    check_alive();

    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {
        if(strcmp(au.url,"")){
            if(!strcmp(au.type,"3"))
            {
                if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
                    break;
                }
                if(on_heavy_counts+on_ed2k_counts<MAX_ON_HEAVY){
                    struct Lognote *note = createnote(au.id, au.infohash,au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
                    insertnote2(note);
                    if(note){
                        free(note);
                    }
                    break;
                }
            }
            else if(!strcmp(au.type,"1")||!strcmp(au.type,"2"))
            {
                if (access("/tmp/APPS/DM2/Status/snarf_stop",0) == 0){
                    break;
                }
                if(on_light_counts<MAX_ON_LIGHT){
                    struct Lognote *note = createnote(au.id, au.infohash, au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
                    insertnote2(note);
                    if(note){
                        free(note);
                    }
                    break;
                }
            }
            else if(!strcmp(au.type,"4"))
            {
                if (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0){
                    break;
                }
                if(on_nntp_counts<MAX_ON_NNTP){
                    struct Lognote *note = createnote(au.id, au.infohash, au.url, au.real_url,au.filenum, au.type, au.status ,au.checknum);
                    insertnote2(note);
                    if(note){
                        free(note);
                    }
                    break;
                }
            }
            else if(!strcmp(au.type,"6"))
            {
                if (access("/tmp/APPS/DM2/Status/ed2k_stop",0) == 0){
                    break;
                }
                if(on_ed2k_counts+on_heavy_counts<MAX_ON_ED2K){
                    struct Lognote *note = createnote(au.id, au.infohash,au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
                    insertnote2(note);
                    if(note){
                        free(note);
                    }
                    break;
                }
            }
        }
        memset(&au, 0, sizeof(struct Lognote));

    }
    close(fd_r);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);
}

int check_time_loop(){
    time_t timter;
    timter = time((time_t*)0);
    struct tm *t_tm;
    t_tm=localtime(&timter);

    if(tmp_enable==0)
    {
        allow_download = 1;
        return 1;
    }

    if(tmp_day<7){
        if(tmp_day!=t_tm->tm_wday){
            allow_download = 0;
            return 0;
        }
    }
    if(tmp_day==8){
        if(t_tm->tm_wday==6 && t_tm->tm_wday==0)
        {
            allow_download= 0;
            return 0;
        }
    }
    if(tmp_day==9){
        if(0<t_tm->tm_wday && t_tm->tm_wday<6)
        {
            allow_download = 0 ;
            return 0;
        }
    }

    if (tmp_shour > t_tm->tm_hour || tmp_ehour < t_tm->tm_hour){
        allow_download = 0;
        return 0;
    }
    else if(tmp_shour < t_tm->tm_hour && tmp_ehour > t_tm->tm_hour){
        allow_download = 1;
        return 1;
    }
    else if(tmp_shour == t_tm->tm_hour){
        if(tmp_smin > t_tm->tm_min)
        {
            allow_download = 0;
            return 0;
        }
        else if(tmp_ehour == t_tm->tm_hour){
            if(tmp_emin >= t_tm->tm_min){
                allow_download = 1;
                return 1;
            }
            else{
                allow_download = 0;
                return 0;
            }
        }
        else if(tmp_ehour > t_tm->tm_hour){
            allow_download = 1;
            return 1;
        }
        else{
            allow_download = 0;
            return 0;
        }

    }
    else if(tmp_ehour == t_tm->tm_hour){
        if(tmp_emin >= t_tm->tm_min){
            allow_download = 1;
            return 1;
        }
        else{
            allow_download = 0;
            return 0;
        }

    }
    else{
        allow_download = 0;
        return 0;
    }
}

char *getlogid2(const char *logname)
{
    char *ptr;

    if((ptr = strstr(logname, "snarf_")) != NULL){
        ptr += 6;
    }
    else if((ptr = strstr(logname, "transm_")) != NULL){
        ptr += 7;
    }
    else if((ptr = strstr(logname, "nzb_")) != NULL){
        ptr += 4;
    }
    else if((ptr = strstr(logname, "ed2k_")) != NULL){
        ptr += 5;
    }
    //else if((ptr = strstr(logname, "error_")) != NULL){
    //    ptr += 6;
    //}
    else{
        return "none";
    }

    return ptr;
}

int DM_CTRL(char* cmd, char* task_id , char* d_type)
{	    
    int sfd;
    int result = UNKNOW;
    char command[100];
    struct sockaddr_in btaddr;

    char id[10];
    memset(id,'\0',sizeof(id));

    if(strcmp(task_id, "all") == 0)
        sprintf(id,"%s",task_id);
    else
        sprintf(id,"%s",getlogid2(task_id));

    memset(command,'\0',sizeof(command));
    sprintf(command,"%s%s",cmd,id);

    memset(&btaddr, '\0', sizeof(btaddr));
    btaddr.sin_family = AF_INET;
    if(!strcmp(d_type, "BT")){
        btaddr.sin_port = htons(BT_PORT);
    }
    else if(!strcmp(d_type, "HTTP") || !strcmp(d_type, "FTP")){
        btaddr.sin_port = htons(SNARF_PORT);
    }
    else if(!strcmp(d_type, "NZB")){
        btaddr.sin_port = htons(NZB_PORT);
    }
    inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
    result =  ACK_SUCESS;

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        result = ACK_FAIL;
    }

    if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
        result =  ACK_FAIL;
    }

    if(write(sfd, command, strlen(command)) != strlen(command))
    {
        result =  ACK_FAIL;
    }

    close(sfd);
    sleep(1);
    return result;

}

void ctrl_download(){
    //fprintf(stderr,"\nallow_download_tmp=%d\n",allow_download_tmp);
    //fprintf(stderr,"\nallow_download=%d\n",allow_download);
    if(allow_download_tmp != allow_download){
        char command[128];
        if(allow_download==1){
            DM_CTRL("all_start@","all","BT");
            DM_CTRL("all_start@","all","NZB");
            DM_CTRL("all_start@","all","HTTP");
            memset(command,0,sizeof(command));
#ifdef DM_I686
            sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume all\"","127.0.0.1");
#else
            sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume all\"",lan_ip_addr);
#endif
            system(command);
        }else{
            DM_CTRL("all_paused@","all","BT");
            DM_CTRL("all_paused@","all","NZB");
            DM_CTRL("all_paused@","all","HTTP");
            memset(command,0,sizeof(command));
#ifdef DM_I686
            sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"pause all\"","127.0.0.1");
#else
            sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"pause all\"",lan_ip_addr);
#endif
            system(command);
        }
	allow_download_tmp = allow_download;
    }

}

int check_download_time()
{
    if (access("/opt/etc/dm2_general.conf",0) == 0)
    {

        int fd, len, i=0;
        char ch, tmp[256], name[256], content[256];
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));

        if((fd = open("/opt/etc/dm2_general.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
        }
        else
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "Enable_time"))
                    {
                        tmp_enable = atoi(content);
                    }
                    else  if(!strcmp(name, "Start_hour"))
                    {
                        tmp_shour = atoi(content);
                    }
                    else  if(!strcmp(name, "Start_minute"))
                    {
                        tmp_smin = atoi(content);
                    }
                    else  if(!strcmp(name, "End_hour"))
                    {
                        tmp_ehour = atoi(content);
                    }
                    else  if(!strcmp(name, "End_minute"))
                    {
                        tmp_emin = atoi(content);
                    }
                    else  if(!strcmp(name, "Day"))
                    {
                        tmp_day = atoi(content);
                    }

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);

            time_t timter;
            timter = time((time_t*)0);
            struct tm *t_tm;
            t_tm=localtime(&timter);

            if(tmp_enable==0)
            {
                allow_download_tmp = 1;
                return 1;
            }

            if(tmp_day<7){
                if(tmp_day!=t_tm->tm_wday){
                    allow_download_tmp = 0;
                    return 0;
                }
            }
            if(tmp_day==8){
                if(t_tm->tm_wday==6 && t_tm->tm_wday==0)
                {
                    allow_download_tmp = 0;
                    return 0;
                }
            }
            if(tmp_day==9){
                if(0<t_tm->tm_wday && t_tm->tm_wday<6)
                {
                    allow_download_tmp = 0;
                    return 0;
                }
            }

            if (tmp_shour > t_tm->tm_hour || tmp_ehour < t_tm->tm_hour){
                allow_download_tmp = 0;
                return 0;
            }
            else if(tmp_shour < t_tm->tm_hour && tmp_ehour > t_tm->tm_hour){
                allow_download_tmp = 1;
                return 1;
            }
            else if(tmp_shour == t_tm->tm_hour){
                if(tmp_smin > t_tm->tm_min)
                {
                    allow_download_tmp = 0;
                    return 0;
                }
                else if(tmp_ehour == t_tm->tm_hour){
                    if(tmp_emin >= t_tm->tm_min){
                        allow_download_tmp = 1;
                        return 1;
                    }
                    else{
                        allow_download_tmp = 0;
                        return 0;
                    }
                }
                else if(tmp_ehour > t_tm->tm_hour){
                    allow_download_tmp = 1;
                    return 1;

                }
                else{
                    allow_download_tmp = 0;
                    return 0;
                }

            }
            else if(tmp_ehour == t_tm->tm_hour){
                if(tmp_emin >= t_tm->tm_min){
                    allow_download_tmp = 1;
                    return 1;
                }
                else{
                    allow_download_tmp = 0;
                    return 0;
                }

            }
            else{
                allow_download_tmp = 0;
                return 0;
            }
        }
    }
    else{
	allow_download_tmp = 0;
        return 0;
    }
}

#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 59) // 86399
static int in_sched(int now_mins, int now_dow, int sched_begin, int sched_end, int sched_begin2, int sched_end2, int sched_dow)
{
    //cprintf("%s: now_mins=%d sched_begin=%d sched_end=%d sched_begin2=%d sched_end2=%d now_dow=%d sched_dow=%d\n", __FUNCTION__, now_mins, sched_begin, sched_end, sched_begin2, sched_end2, now_dow, sched_dow);
    int restore_dow = now_dow; // orig now day of week

    // wday: 0
    if((now_dow & 0x40) != 0){
        // under Sunday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (now_mins <= sched_end2) && (sched_begin2 < sched_end2))
            return 1;

        // under Sunday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (sched_begin2 >= sched_end2))
            return 1;

        // under Saturday's sched time
        now_dow >>= 6; // Saturday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end2) && (sched_begin2 >= sched_end2))
            return 1;

        // reset now_dow, avoid to check now_day = 0000001 (Sat)
        now_dow = restore_dow;
    }

    // wday: 1
    if((now_dow & 0x20) != 0){
        // under Monday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (now_mins <= sched_end) && (sched_begin < sched_end))
            return 1;

        // under Monday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (sched_begin >= sched_end))
            return 1;

        // under Sunday's sched time
        now_dow <<= 1; // Sunday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end2) && (sched_begin2 >= sched_end2))
            return 1;
    }

    // wday: 2-5
    if((now_dow & 0x1e) != 0){
        // under today's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (now_mins <= sched_end) && (sched_begin < sched_end))
            return 1;

        // under today's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (sched_begin >= sched_end))
            return 1;

        // under yesterday's sched time
        now_dow <<= 1; // yesterday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end) && (sched_begin >= sched_end))
            return 1;
    }

    // wday: 6
    if((now_dow & 0x01) != 0){
        // under Saturday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (now_mins <= sched_end2) && (sched_begin2 < sched_end2))
            return 1;

        // under Saturday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (sched_begin2 >= sched_end2))
            return 1;

        // under Friday's sched time
        now_dow <<= 1; // Friday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end) && (sched_begin >= sched_end))
            return 1;
    }

    return 0;
}

int timecheck_item(char *activeDate, char *activeTime, char *activeTime2)
{

    if(!strcmp(nv_enable_time,"0")){
        allow_download =  1;
        return 1;
    }
    //fprintf(stderr,"\ntimecheck_item!\n");
    int current, active, activeTimeStart, activeTimeEnd;
    int activeTimeStart2, activeTimeEnd2;
    int now_dow, sched_dow=0;
    time_t now;
    struct tm *tm;
    int i;

    //setenv("TZ", nvram_safe_get("time_zone_x"), 1);

    time(&now);
    tm = localtime(&now);
    current = tm->tm_hour * 60 + tm->tm_min;
    active = 0;

    // weekdays time
    activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
    activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

    // weekend time
    activeTimeStart2 = ((activeTime2[0]-'0')*10 + (activeTime2[1]-'0'))*60 + (activeTime2[2]-'0')*10 + (activeTime2[3]-'0');
    activeTimeEnd2 = ((activeTime2[4]-'0')*10 + (activeTime2[5]-'0'))*60 + (activeTime2[6]-'0')*10 + (activeTime2[7]-'0');

    // now day of week
    now_dow = 1<< (6-tm->tm_wday);

    // schedule day of week
    sched_dow = 0;
    for(i=0;i<=6;i++){
        sched_dow += (activeDate[i]-'0') << (6-i);
    }

    active = in_sched(current, now_dow, activeTimeStart, activeTimeEnd, activeTimeStart2, activeTimeEnd2, sched_dow);

    //fprintf(stderr,"[lighttpd] active: %d\n", active);
    allow_download =  active;
    //fprintf(stderr,"[lighttpd] allow_download: %d\n", allow_download);
    return active;
}

//void http_login_cache(unsigned int ip){
void http_login_cache(connection *con){
    //fprintf(stderr,"\nhttp_login_cache\n");
    //struct in_addr temp_ip_addr;
    //char *temp_ip_str;

    //login_ip_tmp = ip;
    login_ip_tmp = (unsigned int)(con->dst_addr.ipv4.sin_addr.s_addr);
    //temp_ip_addr.s_addr = login_ip_tmp;
    //temp_ip_str = inet_ntoa(temp_ip_addr);
}

void http_login(unsigned int ip){
    //fprintf(stderr,"\n@@@http_login\n");
    struct in_addr login_ip_addr;
    char *login_ip_str;

    //if(http_port != SERVER_PORT || ip == 0x100007f)
    //	return;

    login_ip = ip;
    last_login_ip = 0;

    login_ip_addr.s_addr = login_ip;
    login_ip_str = inet_ntoa(login_ip_addr);

    login_timestamp = time((time_t *)0);

    /*if(strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "result_of_get_changed_status_QIS.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp"))
		login_timestamp = time((time_t *)0);

	char login_ipstr[32], login_timestampstr[32];
	
	memset(login_ipstr, 0, 32);
	sprintf(login_ipstr, "%u", login_ip);
	nvram_set("login_ip", login_ipstr);
	
	if(strcmp(url, "result_of_get_changed_status.asp")
			&& strcmp(url, "result_of_get_changed_status_QIS.asp")
			&& strcmp(url, "WPS_info.asp")
			&& strcmp(url, "WAN_info.asp")){
		memset(login_timestampstr, 0, 32);
		sprintf(login_timestampstr, "%lu", login_timestamp);
		nvram_set("login_timestamp", login_timestampstr);
	}*/
}

// -1: can not login, 0: not loginer, 1: can login, 2: loginer.
int http_login_check(void){
    //if(http_port != SERVER_PORT || login_ip_tmp == 0x100007f)
    //return 1;
    //	return -1;	// 2008.01 James.

    //http_login_timeout(login_ip_tmp);	// 2008.07 James.
    //fprintf(stderr,"\nhttp_login_check\n");
    //fprintf(stderr,"\n******login_ip_tmp=%d\n",login_ip_tmp);
    //fprintf(stderr,"\n******login_ip=%d\n",login_ip);
    if(login_ip == 0){
        return 1;
    }
    else if(login_ip == login_ip_tmp){
        return 2;
    }

    return 0;
}

void http_login_timeout(unsigned int ip)
{
    //time_t now;
    //fprintf(stderr,"\nhttp_login_timeout\n");
    time(&now);

    // 2007.10 James. for really logout. {
    //if (login_ip!=ip && (unsigned long)(now-login_timestamp) > 60) //one minitues
    //fprintf(stderr,"\nlogin_timestamp=%lu\n",login_timestamp);
    if((login_ip != 0 && login_ip != ip) && (unsigned long)(now-login_timestamp) > 60) //one minitues
        // 2007.10 James }
    {
        //fprintf(stderr,"\n run http_logout\n");
        http_logout(login_ip);
    }
}

void http_logout(unsigned int ip){
    //fprintf(stderr,"\nhttp_logout\n");
    if(ip == login_ip){
        last_login_ip = login_ip;
        login_ip = 0;
        login_ip_tmp = 0;
        login_timestamp = 0;

        //fprintf(stderr,"\n@@@login_ip_tmp=%d\n",login_ip_tmp);
        //fprintf(stderr,"\n@@@login_ip=%d\n",login_ip);

        //nvram_set("login_ip", "");
        //nvram_set("login_timestamp", "");

        // 2008.03 James. {
        /*if(change_passwd == 1){
			change_passwd = 0;
			reget_passwd = 1;
		}*/
        // 2008.03 James. }
    }
}

int
        swap_check()
{
    FILE *fp;
    char temp[80];
    char *p;

    if (fp=fopen("/proc/swaps", "r"))
    {
        while (fgets(temp,80,fp)!=NULL)
        {
            if (p=strstr(temp, "/.swap"))
            {
                return 1;
                break;
            }
        }
        fclose(fp);
    }
    return 0;
}

void run_onswap(){
    FILE *fp_disk;
    struct sysinfo info;
    char tmpstr[64];
    char tmpstr2[64];
    char *buf=NULL;
    int buflen=0;
    int i;

    memset(tmpstr,0,sizeof(tmpstr));
    sprintf(tmpstr, "%s/swap", Base_dir);
    unlink(tmpstr);
    memset(tmpstr,0,sizeof(tmpstr));
    sprintf(tmpstr, "%s/.swap", Base_dir);
    unlink(tmpstr);

    memset(&info,0,sizeof(struct sysinfo));
    sysinfo(&info);

    if ((info.freeram + info.bufferram) >= 1024*1024*2)
        buflen = 1024*1024*2;
    else if ((info.freeram + info.bufferram) >= 1024*1024*1.6)
        buflen = 1024*1024*1.6;
    else if ((info.freeram + info.bufferram) >= 1024*1024*1)
        buflen = 1024*1024*1;
    else if ((info.freeram + info.bufferram) >= 1024*1024*0.8)
        buflen = 1024*1024*0.8;
    else
        buflen = 1024*1024*0.5;
    buf = malloc(buflen);
    for(i=0;i<buflen;i++)
        buf[i]='\n';

    fp_disk=fopen(tmpstr, "a");
    if(fp_disk!=NULL)
    {
        for(i=0;i<1024*1024*32/buflen;i++)
            fprintf(fp_disk, "%s", buf);
        free(buf);
        fclose(fp_disk);

        memset(tmpstr2,0,sizeof(tmpstr2));
        sprintf(tmpstr2, "mkswap %s", tmpstr);
        system(tmpstr2);

        //memset(tmpstr2,0,sizeof(tmpstr2));
        //sprintf(tmpstr2, "swapon %s", tmpstr);
        //system(tmpstr2);
        swapon(tmpstr,0);

        //fprintf(stderr,"\n#########USB storage 32MB swap file is added@@@@@@@@\n");
        //logmessage("USB storage", "32MB swap file is added");
        //nvram_set("swap_on", "1");
    }

}

void run_offswap(){
    char tmpstr[64];
    memset(tmpstr,0,sizeof(tmpstr));
    sprintf(tmpstr, "%s/.swap", Base_dir);
    swapoff(tmpstr);
    sleep(1);
    if (swap_check()==1)
    {
        //fprintf(stderr,"\n#########USB storage, swapoff unsuccessfully\n");
        //logmessage("USB storage", "swapoff unsuccessfully");
        //nvram_set("swapoff_failed", "1");
        unlink(tmpstr);
        //nvram_set("reboot", "1");
    }
    else
    {
        //fprintf(stderr,"\n#########USB storage, swapoff successfully\n");
        //logmessage("USB storage", "swapoff successfully");
        //nvram_set("swap_on", "0");
        //nvram_set("swapoff_failed", "0");
        //nvram_set("apps_dms_usb_port_x2", "-1");
        unlink(tmpstr);
    }

}

int detect_process(char * process_name)  
{
    FILE *ptr;
    char buff[512];
    char ps[128];
    sprintf(ps,"ps | grep -c %s",process_name);
    strcpy(buff,"ABNORMAL");
    if((ptr=popen(ps, "r")) != NULL)
    {
        while (fgets(buff, 512, ptr) != NULL)
        {
            if(atoi(buff)>=2)   //>= for mipsbig > for arm
            {
                pclose(ptr);
                //fprintf(stderr,"\n#########dm2_transmission-daemon is running\n");
                return 1;
            }
        }
    }
    if(strcmp(buff,"ABNORMAL")==0)  /*ps command error*/
    {
        //fprintf(stderr,"\n#########ps error\n");
        return 0;
    }
    pclose(ptr);
    //fprintf(stderr,"\n#########dm2_transmission-daemon not running\n");
    return 0;
}
/* Transfer Char to ASCII */
int char_to_ascii_safe(const char *output, const char *input, int outsize)
{
    char *src = (char *)input;
    char *dst = (char *)output;
    char *end = (char *)output + outsize - 1;
    char *escape = "[]"; // shouldn't be more?

    if (src == NULL || dst == NULL || outsize <= 0)
        return 0;

    for ( ; *src && dst < end; src++) {
        if ((*src >='0' && *src <='9') ||
            (*src >='A' && *src <='Z') ||
            (*src >='a' && *src <='z')) {
            *dst++ = *src;
        } else if (strchr(escape, *src)) {
            if (dst + 2 > end)
                break;
            *dst++ = '\\'; //2012.07.31 magic
            *dst++ = *src;
        } else {
            if (dst + 3 > end)
                break;
            dst += sprintf(dst, "%%%.02X", *src);
        }
    }
    if (dst <= end)
        *dst = '\0';

    return dst - output;
}

void char_to_ascii(const char *output, const char *input)
{
    int outlen = strlen(input)*3 + 1;
    char_to_ascii_safe(output, input, outlen);
}

/* Transfer ASCII to Char */
int ascii_to_char_safe(const char *output, const char *input, int outsize){
    char *src = (char *)input;
    char *dst = (char *)output;
    char *end = (char *)output+outsize-1;
    char char_array[3];
    unsigned int char_code;

    if(src == NULL || dst == NULL || outsize <= 0)
        return 0;

    for(; *src && dst < end; ++src, ++dst){
        if((*src >= '0' && *src <= '9')
            || (*src >= 'A' && *src <= 'Z')
            || (*src >= 'a' && *src <= 'z')
            ){
            *dst = *src;
        }
        else if(*src == '\\'){
            ++src;
            if(!(*src))
                break;

            *dst = *src;
        }
        else{
            ++src;
            if(!(*src))
                break;
            memset(char_array, 0, 3);
            strncpy(char_array, src, 2);
            ++src;

            char_code = strtol(char_array, NULL, 16);

            *dst = (char)char_code;
        }
    }

    if(dst <= end)
        *dst = '\0';

    return (dst-output);
}

void ascii_to_char(const char *output, const char *input){
    int outlen = strlen(input)+1;
    ascii_to_char_safe(output, input, outlen);
}

const char *find_word(const char *s_buffer, const char *word)
{
    const char *p, *q;
    int n;

    n = strlen(word);
    p = s_buffer;
    while ((p = strstr(p, word)) != NULL) {
        if ((p == s_buffer) || (*(p - 1) == ' ') || (*(p - 1) == ',')) {
            q = p + n;
            if ((*q == ' ') || (*q == ',') || (*q == 0)) {
                return p;
            }
        }
        ++p;
    }
    return NULL;
}

/*
static void add_word(char *buffer, const char *word, int max)
{
	if ((*buffer != 0) && (buffer[strlen(buffer) - 1] != ' '))
		strlcat(buffer, " ", max);
	strlcat(buffer, word, max);
}
*/

int remove_word(char *s_buffer, const char *word)
{
    char *p;
    char *q;

    if ((p = strstr(s_buffer, word)) == NULL) return 0;
    q = p;
    p += strlen(word);
    while (*p == ' ') ++p;
    while ((q > s_buffer) && (*(q - 1) == ' ')) --q;
    if (*q != 0) *q++ = ' ';
    strcpy(q, p);

    return 1;
}

/*int decode_url(char *passwd)
{

    //printf("start decode url \n");

    int len ;
    int i,k;
    char temp_passwd[256];

    memset( temp_passwd,0,sizeof(temp_passwd) );
    memset( http_pw_check_temp,0,sizeof(http_pw_check_temp) );

    len = strlen(passwd);

    for( i = 0 , k= 0 ; i < len ; i++ ,k++)
    {
	if( passwd[i] == '"')
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = '"';
            k++;
        }
       else if( passwd[i] == "'")
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = "'";
            k++;
        }
       else if( passwd[i] == '\\')
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = '\\';
            k++;
        }
        temp_passwd[k] = passwd[i];
    }

    //int size = strlen(temp_url);
    temp_passwd[k+1] = '\0';

    //fprintf(stderr,"temp url is %s \n",temp_url);


    //strcpy(passwd,temp_passwd);
    sprintf(http_pw_check_temp,"%s",temp_passwd);

}*/
#ifdef DM_MIPSBIG
int decode_url_org(char *passwd)
{

    //printf("start decode url \n");

    int len ;
    int i,k;
    char temp_passwd[256];

    memset( temp_passwd,0,sizeof(temp_passwd) );
    memset( http_pw_check_temp,0,sizeof(http_pw_check_temp) );

    len = strlen(passwd);

    for( i = 0 , k= 0 ; i < len ; i++ ,k++)
    {
	if( passwd[i] == '"')
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = '"';
            k++;
        }
        else if( passwd[i] == "'")
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = "'";
            k++;
        }
        else if( passwd[i] == '\\')
        {
            temp_passwd[k] = '\\';
            temp_passwd[k+1] = '\\';
            k++;
        }
        temp_passwd[k] = passwd[i];
    }

    //int size = strlen(temp_url);
    temp_passwd[k+1] = '\0';

    //fprintf(stderr,"temp url is %s \n",temp_url);


    //strcpy(passwd,temp_passwd);
    sprintf(http_pw_check_temp,"%s",temp_passwd);
    return 1;
}
#endif
void CvtHex1(IN HASH Bin, OUT HASHHEX Hex) {
    unsigned short i;

    for (i = 0; i < HASHLEN; i++) {
        Hex[i*2] = int2hex((Bin[i] >> 4) & 0xf);
        Hex[i*2+1] = int2hex(Bin[i] & 0xf);
    }
    Hex[HASHHEXLEN] = '\0';
}

int decode_url(char *passwd,int tag)
{

    //printf("start decode url \n");

    memset( http_pw_check_temp,0,sizeof(http_pw_check_temp) );
    if(tag>0){
        MD5_CTX Md5Ctx3;
        HASH HA3;
        char temp_passwd[256];
        memset( temp_passwd,0,sizeof(temp_passwd) );

        MD5_Init(&Md5Ctx3);
        MD5_Update(&Md5Ctx3, (unsigned char *)passwd, strlen(passwd));
        MD5_Final(HA3, &Md5Ctx3);

        CvtHex1(HA3, temp_passwd);
        sprintf(http_pw_check_temp,"%s",temp_passwd);
    }
    else{
        sprintf(http_pw_check_temp,"%s",passwd);
    }

    return 1;
}

#ifdef DM_MIPSBIG
int check_mips_type(){
    int mips_type = 0;
    if(access("/userfs/bin/tcapi",0) == 0){
        mips_type = 1;
    } else {
        mips_type = 0;
    }
    return mips_type;
}
#endif

void start_amule(){
	// ensure aMule can be started after it crashed
	if((access(aMule_Lock, F_OK) ==0) && \
		(pids("dm2_amuled") == 0)) {
		unlink(aMule_Lock);
	}

	//use taskset to lock dm2_amuled in cpu 3 to avoid thread crash
	if ((access("/usr/bin/taskset", F_OK) == 0)&& \
	    (strcmp(router_name, "RT-AC85U") == 0)) {
		if (access("/bin/nice", F_OK) == 0) {
			system(CPU3Taskset niceCMD aMuleCMD);
		} else {
			system(CPU3Taskset aMuleCMD);
		}
	} else if (access("/bin/nice", F_OK) == 0) {
		system(niceCMD aMuleCMD);
	} else {
		system(aMuleCMD);
	}

	//to wait amule start
	sleep(8);
}
