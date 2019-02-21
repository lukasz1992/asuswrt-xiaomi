/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include "util.h"

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/statvfs.h>

#ifdef USE_SOCKS5
#define SOCKS
#include <socks.h>
#endif

#if (defined(HAVE_SYS_IOCTL_H) && defined(GUESS_WINSIZE))
/* Needed for SunOS */
#ifndef BSD_COMP
#define BSD_COMP
#endif
#include <sys/ioctl.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "url.h"
#include "options.h"
// ex
#include "logs.h"
#include "ex.h"
//~ex

//#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
//#define FILE1_MODE (S_IFIFO | 0666)

/*
#define csprintf(fmt, args...) do{\
        FILE *cp = fopen("/dev/console", "w");\
        if(cp) {\
                fprintf(cp, fmt, ## args);\
                fclose(cp);\
        }\
}while(0)
*/
#define SEM 0


char output_buf[BUFSIZ];

char mnt_type[MAX_SHORTLEN];
int64_t Available_space;

struct sockaddr_in  servaddr;
extern char share_dir[MAX_NAMELEN];
char logName[MAX_NAMELEN];
char semName[MAX_NAMELEN];
//int  log_use, semUse;
extern int log_use;
int semUse;
char complete_path[MAX_NAMELEN];
char info_dest_path[MAX_NAMELEN];
char basepath[MAX_NAMELEN];
int LogID;
int logtime = 0;
extern int is_kill;
extern int reading_sock;
extern int create_errorlog;

extern int resolve_hostname;
extern int socket_connect;
extern char real_name[256];

Log_struc  log_s;
int above_4GB = 0;

#ifndef HAVE_STRERROR

extern int sys_nerr;
extern char *sys_errlist[];

const char *
        strerror(int index)
{
    if( (index > 0) && (index <= sys_nerr) ) {
        return sys_errlist[index];
    } else if(index==0) {
        return "No error";
    } else {
        return "Unknown error";
    }
}

#endif /* !HAVE_STRERROR */



void
        repchar(FILE *fp, char ch, int count)
{
    while(count--)
        fputc(ch, fp);
}


int
        guess_winsize()
{
#if (defined(HAVE_SYS_IOCTL_H) && defined(GUESS_WINSIZE))
    int width;
    struct winsize ws;

    if( ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == -1 ||
        ws.ws_col == 0 )
        width = 79;
    else
        width = ws.ws_col - 1;

    return width;
#else
    return 79;
#endif /* defined(HAVE_SYS_IOCTL_H) && defined(GUESS_WINSIZE) */
}


double
        double_time(void)
{
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.00);
#else
    return time();
#endif
}

int  cut_path(char *path , int n)
{

    //int i;
    memset(info_dest_path,'\0',sizeof(info_dest_path));

    /*
    char *new_path = NULL;
    new_path = path;

    for(i= 0;i< n ;i++)
    {
        new_path = strchr(new_path,'/');
        new_path++;
    }
    */

    //strcpy(info_dest_path,new_path);
    strcpy(info_dest_path,path);

    //return new_path;
}

char *
        string_lowercase(char *string)
{
    char *start = string;

    if( string == NULL )
        return NULL;

    while( *string ) {
        *string = tolower(*string);
        string++;
    }

    return start;
}


char *
        get_proxy(const char *firstchoice)
{
    char *proxy;
    char *help;

    if( (proxy = getenv(firstchoice)) )
        return proxy;

    help = safe_strdup(firstchoice);
    string_lowercase(help);
    proxy = getenv(help);
    safe_free(help);
    if( proxy )
        return proxy;

    if( (proxy = getenv("SNARF_PROXY")) )
        return proxy;

    if( (proxy = getenv("PROXY")) )
        return proxy;

    return NULL;
}

int64_t bg_10_pow(int pow){

    int64_t num = 1;
    int i;

    for(i=0; i<pow; ++i)
        num*=10;

    return num;
}

int64_t convert_bg_str(char *str){

    int64_t bg_val = 0;
    int len = strlen(str);
    char bg_str[len];
    int i, pow, num;

    memset(bg_str, '\0', len);
    strncpy(bg_str, str, len);

    for(i=0; i<len; ++i){
        pow = len - i - 1;
        num = bg_str[i] - 48;
        bg_val += (num * bg_10_pow(pow));
    }

    return bg_val;
}

void strip_str(char *str, char *tmps){

    int i, j;

    for(i=0, j=0; i<strlen(str); ++i){
        if(*(str+i) == ' ')
            continue;

        tmps[j++] = *(str+i);
    }
}

int64_t ato64_t(char *str){

    char *n_critical_size = "2147483647";
    char tmps[32];

    memset(tmps, '\0', sizeof(tmps));
    strip_str(str, tmps);

    if(strlen(tmps) < 10)
        return atoi(tmps);
    else if(strlen(tmps) == 10){
        if(strcmp(tmps, n_critical_size) <= 0)
            return atoi(tmps);
        else
            return convert_bg_str(tmps);
    } else
        return convert_bg_str(tmps);

    return -1;
}

void 
        init_log(const char *path, char *completepath,int id){

    LogID = id;

    memset(logName, '\0', sizeof(logName));
    memset(semName, '\0', sizeof(semName));
    memset(&log_s, '\0', sizeof(log_s));


    //add by gauss
    memset(complete_path, '\0', sizeof(complete_path));
    strcpy(complete_path,completepath);

    //strcpy(info_dest_path,cut_path(completepath,4));
    cut_path(complete_path,4);

    //printf("\n #### complete path is %s\n",complete_path);

    //char basepath[256];
    memset(basepath,0,sizeof(basepath));

    strncpy(basepath,path,strlen(path)-6);

    sprintf(logName, "%s/snarf_%d", path, id);
    sprintf(semName,"%s/.sems/sem.snarf_%d",basepath,id);
}

void small_sleep(float nsec){
    struct timeval  tval;

    tval.tv_sec = (int)(nsec*1000000) / 1000000;
    tval.tv_usec = (int)(nsec*1000000) % 1000000;

    select(0, NULL, NULL, NULL, &tval);
}

int
        log_init(UrlResource *rsrc)
{
    //printf("pid: %d\n", getpid());
    sprintf(log_s.id, "%d", getpid());
    if(rsrc->url->service_type == SERVICE_HTTP)
        log_s.download_type = HTTP;
    else if(rsrc->url->service_type == SERVICE_FTP)
        log_s.download_type = FTP;

    time(&log_s.begin_t);

    strncpy(log_s.fullname, rsrc->url->full_url, min(strlen(rsrc->url->full_url), sizeof(log_s.fullname)) );
    //strncpy(log_s.filename, rsrc->outfile, min(strlen(rsrc->outfile), sizeof(log_s.filename)) );
    strcpy(log_s.filename,real_name);
    log_s.filesize = (int)(rsrc->outfile_size / 1024);
    //fprintf(stderr, "[snarf] log_init: log_s.fullname:[%s], log_s.filename[%s] \n",log_s.fullname,log_s.filename);
    // log_s.filesize = 1000;

    //2013.09.16 gauss add{
    if((rsrc->outfile_size >= SIZE_4G) ){
        FILE *fp;
        char disk_type[8] = {0};
        fp = fopen("/tmp/asus_router.conf","r");
        if(fp)
        {
            while(!feof(fp))
            {
                fscanf(fp,"%[^\n]%*c",generalbuf);
                if(strncmp(generalbuf,"DEVICE_TYPE=",11) == 0)
                {
                    strcpy(disk_type,generalbuf+12);
                    if(!strncmp(disk_type,"vfat",4)||!strncmp(disk_type,"tfat",4))
                    {
                      fclose(fp);
                      err_log(rsrc, "Not Support 4GB",log_s.download_type);

                      return -1;
                    }
                    else
                        above_4GB = 1;
                    break;
                }
            }
        }
        fclose(fp);
    }
    //2013.09.16 gauss add{
      struct disk_info *disk_tmp; //2012.12.20 magic add{
      disk_tmp = follow_disk_info_start; //2012.12.20 magic add}

    switch(log_s.download_type){
    case HTTP:
        if(rsrc->proxy)
            strncpy(log_s.ifile.iHttp.proxy, rsrc->proxy, min(strlen(rsrc->proxy), MAX_SHORTNAMELEN));
                
                //2012.12.20 magic add{
                log_s.ifile.iHttp.partitionnum = 0;
                while(disk_tmp != NULL)
                {
                    if(disk_tmp->diskname != NULL)
                    {
                        if((strncmp(serialtmp,disk_tmp->serialnum,strlen(disk_tmp->serialnum)) == 0)&&(strncmp(producttmp,disk_tmp->product,strlen(disk_tmp->product)) == 0)&&(strncmp(vondertmp,disk_tmp->vendor,strlen(disk_tmp->vendor)) == 0)&&(partitiontmp==disk_tmp->partitionport))
                        {
                            break;
                        }
                        else
                            disk_tmp=disk_tmp->next;
                    }
                    else
                        disk_tmp=disk_tmp->next;
                }

                if((disk_tmp!=NULL)&&(disk_tmp->partitionport != 0))
                {
                    sprintf(log_s.ifile.iHttp.serial, "%s",disk_tmp->serialnum);
                    sprintf(log_s.ifile.iHttp.pid, "%s",disk_tmp->product);
                    sprintf(log_s.ifile.iHttp.vid, "%s",disk_tmp->vendor);
                    log_s.ifile.iHttp.partitionnum = disk_tmp->partitionport;
                }
                else
                {
                    memset(log_s.ifile.iHttp.serial,0,sizeof(log_s.ifile.iHttp.serial));
                    memset(log_s.ifile.iHttp.pid,0,sizeof(log_s.ifile.iHttp.pid));
                    memset(log_s.ifile.iHttp.vid,0,sizeof(log_s.ifile.iHttp.vid));
                }
                 //2012.12.20 magic add}

        break;
    case FTP:
        if(rsrc->proxy)
            strncpy(log_s.ifile.iFtp.proxy, rsrc->proxy, min(strlen(rsrc->proxy), MAX_SHORTNAMELEN));
        if(rsrc->url->username)
            strncpy(log_s.ifile.iFtp.user_name, rsrc->url->username, min(strlen(rsrc->url->username), MAX_SHORTLEN));
        if(rsrc->url->password)
            strncpy(log_s.ifile.iFtp.password, rsrc->url->password, min(strlen(rsrc->url->password), MAX_SHORTLEN));
        if(rsrc->url->host)
            //strncpy(log_s.ifile.iFtp.user_name, rsrc->url->username, min(strlen(rsrc->url->username), MAX_SHORTNAMELEN));
            strncpy(log_s.ifile.iFtp.host, rsrc->url->host, min(strlen(rsrc->url->host), MAX_SHORTNAMELEN));
        if(rsrc->url->port)
            log_s.ifile.iFtp.port = rsrc->url->port;

                //2012.12.20 magic add{
                log_s.ifile.iFtp.partitionnum = 0;
                while(disk_tmp != NULL)
                {
                    if(disk_tmp->diskname != NULL)
                    {
                        if((strncmp(serialtmp,disk_tmp->serialnum,strlen(disk_tmp->serialnum)) == 0)&&(strncmp(producttmp,disk_tmp->product,strlen(disk_tmp->product)) == 0)&&(strncmp(vondertmp,disk_tmp->vendor,strlen(disk_tmp->vendor)) == 0)&&(partitiontmp==disk_tmp->partitionport))
                        {
                            break;
                        }
                        else
                            disk_tmp=disk_tmp->next;
                    }
                    else
                        disk_tmp=disk_tmp->next;
                }

                if((disk_tmp!=NULL)&&(disk_tmp->partitionport != 0))
                {
                    sprintf(log_s.ifile.iFtp.serial, "%s",disk_tmp->serialnum);
                    sprintf(log_s.ifile.iFtp.pid, "%s",disk_tmp->product);
                    sprintf(log_s.ifile.iFtp.vid, "%s",disk_tmp->vendor);
                    log_s.ifile.iFtp.partitionnum = disk_tmp->partitionport;
                }
                else
                {
                    memset(log_s.ifile.iFtp.serial,0,sizeof(log_s.ifile.iFtp.serial));
                    memset(log_s.ifile.iFtp.pid,0,sizeof(log_s.ifile.iFtp.pid));
                    memset(log_s.ifile.iFtp.vid,0,sizeof(log_s.ifile.iFtp.vid));
                }
         //2012.12.20 magic add}
        break;
    }
    //fprintf(stderr, "snarf: logInit: logName is %s, semName is %s\n", logName, semName);
    small_sleep(0.6);


#if SEM
     semUse = 1;
     //int  flag_sem = O_CREAT|O_RDWR;
          if(Sem_open(&sem, semName, 0) == (-1)){  // try open
                semUse = 0;
                fprintf(stderr, "snarf error: log_init: Cannot open sem\n");

        }
#endif


    //makefifo(semName);
   return 0;

}

void final_log(char *logName,char *complete_path,UrlResource *rsrc)
{
    int  fd;
    Log_struc log_s;
    memset(&log_s, 0, sizeof(Log_struc));

    if((fd = open(logName, O_RDONLY | O_NONBLOCK)) < 0)
    {
        fprintf(stderr,"read log error!\n");
        return;
    }
    else
    {
        int len = read(fd, &log_s, sizeof(Log_struc));
        close(fd);
    }


    log_s.status = S_COMPLETED;
    log_s.progress = 1.00;
    if(log_s.filesize == 0)
        log_s.filesize = rsrc->outfile_offset/1024;

            char *realdldir;
            realdldir = my_nstrchr('/',complete_path,4);
 //fprintf(stderr,"\ncomplete_path=%s\n",complete_path);
    if(log_s.download_type == HTTP )
    {
       log_s.ifile.iHttp.rate = 0 ;
       strcpy(log_s.ifile.iHttp.Time_Left,"00:00:00");

           if ( !access(complete_path,0) ){
                 sprintf(log_s.ifile.iHttp.Destination, "%s",complete_path);
                 //fprintf(stderr,"\nlog_s.ifile.iHttp.Destination=%s\n",log_s.ifile.iHttp.Destination);
            }
            else{
                    sprintf(log_s.ifile.iHttp.Destination, "%s%s","not_found",realdldir);
                    //fprintf(stderr,"\nlog_s.ifile.iHttp.Destination=%s\n",log_s.ifile.iHttp.Destination);
                }
   }
    else
    {
        log_s.ifile.iFtp.rate = 0;
        strcpy(log_s.ifile.iFtp.Time_Left,"00:00:00");
           if ( !access(complete_path,0) ){
                 sprintf(log_s.ifile.iFtp.Destination, "%s",complete_path);
            }
            else{
                    sprintf(log_s.ifile.iFtp.Destination, "%s%s","not_found",realdldir);
                }
    }

 

    //log_s.ifile.iHttp.rate = 0;

    if((log_use == 1) && (strlen(share_dir) > 0)){

#if SEM
        if(semUse)
            Sem_wait(&sem);
#endif

        if((fd = open(logName,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
            perror("Open log error\n");
        else{
            if(write(fd, &log_s, LOG_SIZE) < 0)
                fprintf(stderr, "snarf: write log error\n");
            close(fd);
        }

#if SEM
        if(semUse)
            Sem_post(&sem);
#endif
    }

}


void gdc_query(char *err, char *url){
    int sfd;
    struct sockaddr_in gdcaddr;
    char gift_cmd[512];

    // test
    //fprintf(stderr, "snarf giftquery 1\n");
    memset(&gdcaddr, '\0', sizeof(gdcaddr));
    gdcaddr.sin_family = AF_INET;
    gdcaddr.sin_port = htons(GIFT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &gdcaddr.sin_addr);

    // test
    //fprintf(stderr, "snarf giftquery 2\n");

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("gdc socket error");
        return;
    }
    if(connect(sfd, (struct sockaddr*)&gdcaddr, sizeof(gdcaddr)) < 0){
        perror("gdc connect error");
        return;
    }

    // test
    //fprintf(stderr, "snarf giftquery 3\n");

    memset(gift_cmd, '\0', sizeof(gift_cmd));
    sprintf(gift_cmd, "exdown url(Err://%d#%s@error://%s);", getpid(), err, url);

    // test
    //fprintf(stderr, "snarf : gdc_query sendcmd: %s\n", gift_cmd);

    if(write(sfd, gift_cmd, strlen(gift_cmd)) != strlen(gift_cmd))
        perror("snarf: gdc query write");

    close(sfd);
    // test
    //fprintf(stderr, "snarf : gdc_query close fd\n");
}

void err_log(UrlResource *rsrc, char *err,int type){
    int  fd;

    err_counts++;
    memset(&log_s, '\0', sizeof(log_s));

    if( strcmp(err,"Disk space is not enough") == 0)
        log_s.status = S_DISKFULL;
    else if( strcmp(err,"Move Disk space is not enough") == 0)
    {
        log_s.status = S_MOVEDISKFULL;
        log_s.filesize = rsrc->outfile_size / 1024;
        log_s.progress = 1.0;
    }
    else if(strcmp(err,"Not Support 4GB") == 0)
    {
          log_s.status = S_TYPE_OF_ERROR;
          log_s.filesize = rsrc->outfile_size / 1024;
    }
    else if(strcmp(err,"Move 4GB Error") == 0)
    {
         log_s.status = S_MOVE4GBERROR;
         log_s.filesize = rsrc->outfile_size / 1024;
         log_s.progress = 1.0;
     }
    else
    {
        log_s.status = S_DEAD_OF_ERROR;
    }

    log_s.error = 1;
    log_s.download_type = type;

    //fprintf(stderr,"logs type is %d \n",log_s.download_type);

    // test
    //fprintf(stderr, "[snarf]: error log: %s\n", err);
    //gdc_query(err, rsrc->outfile);      // Allen  temp

    if(rsrc->outfile)
        //strncpy(log_s.filename, rsrc->outfile, min(strlen(rsrc->outfile), sizeof(log_s.filename)) );
        strcpy(log_s.filename,real_name);
    else
        strncpy(log_s.filename, "no url/file specified", 21);

    if(err)
    {
        if( strcmp(err,"Disk space is not enough") == 0)
            strcpy(log_s.fullname,rsrc->url->full_url);
        else
            strncpy(log_s.fullname, err, strlen(err));
    }
    else
        strncpy(log_s.fullname, "error", 5);

    small_sleep(0.6);

    if(log_use == 1){
#if SEM
        if(semUse)
            Sem_wait(&sem);
#endif

        if(logName){
            if((fd = open(logName,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
                perror("Open log error\n");
            else{
                if(write(fd, &log_s, LOG_SIZE) < 0)
                    fprintf(stderr, "snarf: err_log: write log error\n");
                    close(fd);

            }
        }
#if SEM
        if(semUse)
            Sem_post(&sem);
#endif
    }

    gdc_query(log_s.fullname, log_s.filename);      // err, url
}

int 
        chk_integrity(UrlResource *rsrc)
{
    /*
        struct stat64 sb;
        if(stat64(rsrc->outfile, &sb) < 0){
                if(ENOENT == errno){
                        // test
                        fprintf(stderr,"[sn:chki] error, lose file %s\n", rsrc->outfile);
                        return -1;
                } else{
                        // test
                        fprintf(stderr,"[sn:chki] error, stat file %s\n", rsrc->outfile);
                        return -1;
                }
        }
        */
    return 0;
}

//2012.12.20 magic added{
char *my_nstrchr(const char chr,char *str,int n){
    if(n<1)
    {
        printf("my_nstrchr need n>=1\n");
        return NULL;
    }

    char *p1,*p2;
    int i = 1;
    p1 = str;

    do{
        p2 = strchr(p1,chr);
        p1 = p2;
        p1++;
        i++;
    }while(p2!=NULL && i<=n);

    if(i<n)
    {
        return NULL;
    }

    if(p2 == NULL)
        return "\0";

    return p2;
}//2012.12.20 magic added}

int IsCompleteSpaceFull(char *path,UrlResource *rsrc)
{
    struct statvfs diskdata;
    long long int free_disk_space;
    if (!statvfs(path, &diskdata))
    {
        free_disk_space = (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
        printf("free disk space is %lld \n",free_disk_space);


        if( rsrc->outfile_size  >= free_disk_space - RESERVE_SPACE )
        {
            err_log(rsrc,"Move Disk space is not enough",HTTP);
            return 1;
        }
    }
    else
    {
        printf("obtain disk space is failed ,basic_path is %s \n",path);
        return -1;
    }

    return 0;
}

int handle_complete(UrlResource *rsrc,char *logName,char *complete_path,char *basepath)
{
    //test
    Log_struc log_ts;
    memset(&log_ts, 0, sizeof(Log_struc));
    int fd1;
  //  printf("\nlogname=%s!\n", logName);
    if((fd1 = open(logName, O_RDONLY | O_NONBLOCK)) < 0)
    {
        printf("\nread log error!\n");
    }
    else
    {
        int len = read(fd1, &log_ts, sizeof(Log_struc));
        close(fd1);
    }
    //test
    //move coplete file to Complete dir ##add by gauss 20110504

    char src_file[256],dest_file[256],temp[256];

    memset(src_file,    '\0', sizeof(src_file));
    memset(dest_file,   '\0', sizeof(dest_file));

    sprintf(src_file,"%s/InComplete/%s",basepath,real_name);

//20121220 magic added{
        free_disk_struc(&follow_disk_info_start);


    //fprintf(stderr,"init_diskinfo_struct start\n");
    if(init_diskinfo_struct()==-1){
        fprintf(stderr,"\ninit_diskinfo_struct failed when move file\n");
    }
    //fprintf(stderr,"init_diskinfo_struct end\n");

    char mountpath[100];
    memset(mountpath,'\0',sizeof(mountpath));
    char *ptr;
    ptr = my_nstrchr('/',complete_path,4);
        if(!ptr){
                strncpy(mountpath,complete_path,strlen(complete_path));
        }
        else{
                strncpy(mountpath,complete_path,strlen(complete_path)-strlen(ptr));
         }
    struct disk_info *disks_tmp;
    disks_tmp = follow_disk_info_start;
    while(disks_tmp!=NULL)
    {
        if(disks_tmp->mountpath!=NULL)
        {
            if((strncmp(serialtmp,disks_tmp->serialnum,strlen(disks_tmp->serialnum)) == 0)&&(strncmp(producttmp,disks_tmp->product,strlen(disks_tmp->product)) == 0)&&(strncmp(vondertmp,disks_tmp->vendor,strlen(disks_tmp->vendor)) == 0)&&(partitiontmp==disks_tmp->partitionport))
            {
                break;
            }
            else
                disks_tmp=disks_tmp->next;
        }
        else
            disks_tmp=disks_tmp->next;
    }

    char basedir[strlen(basepath)+10];
    memset(basedir,'\0',sizeof(basedir));
    strncpy(basedir,basepath,strlen(basepath));

    if(disks_tmp)
    {
        if(strncmp(mountpath,disks_tmp->mountpath,strlen(disks_tmp->mountpath)) != 0)
        {
            memset(mountpath,'\0',sizeof(mountpath));
            strncpy(mountpath,disks_tmp->mountpath,strlen(disks_tmp->mountpath));
            char realpath[200];
            memset(realpath,'\0',sizeof(realpath));
            strncpy(realpath,disks_tmp->mountpath,strlen(disks_tmp->mountpath));
            strcat(realpath,ptr);
            memset(complete_path,'\0',sizeof(char)*strlen(complete_path)); //14/10/31 add by gauss fix complete dir excep
            strncpy(complete_path,realpath,strlen(realpath));
        }
    }
    else
    {
        //memset(tor->stats.errorString,'\0',sizeof(tor->stats.errorString));
        //strcpy(tor->stats.errorString,"The diresed download folder is not exist and move file to default download folder");
        strcat(basedir,"/Complete");
        memset(complete_path,'\0',sizeof(char)*strlen(complete_path)); //14/10/31 add by gauss fix complete dir excep
        strncpy(complete_path,basedir,strlen(basedir));
    }

    //fprintf(stderr,"check disk info end\n");

    int status = IsCompleteSpaceFull(complete_path,rsrc);

    //fprintf(stderr,"check IsCompleteSpaceFull end\n");

    if(status == 1)
        return -1;

    if(disks_tmp)
    {
        if(above_4GB ==1)
        {
            if(!strncmp(disks_tmp->disktype,"vfat",4)||!strncmp(disks_tmp->disktype,"tfat",4))
            {
                err_log(rsrc, "Move 4GB Error",log_ts.download_type);
                return -1;
            }
        }
    }

    //fprintf(stderr,"check 4GB end\n");

    sprintf(dest_file,"%s/%s",complete_path,real_name);

        //printf("dest_file=%s",dest_file);
        //printf("complete_path=%s",complete_path);
//20121220 magic added}

    if(!opendir(complete_path))
    {
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "mkdir  -p %s", complete_path);
        system(temp);
    }

    int fd;
    if( fd = (open(src_file, O_RDONLY | O_NONBLOCK)) != -1 ) {
         close(fd);

        //if ( rename(src_file,dest_file) == -1 )
           // printf("remove error\n");
         memset(temp, 0, sizeof(temp));
         sprintf(temp, "mv \"%s\"  \"%s\"", src_file,dest_file);
         system(temp);
        //else
            //printf("remove succeffly\n");
    }
    //fprintf(stderr,"move complete file end\n");

    final_log(logName,complete_path,rsrc); //2012.12.20 magic
    return 1;
}

int
        dump_data(UrlResource *rsrc, int sock, FILE *out)
{
    int out_fd 		= fileno(out);
    Progress *p		= NULL;
    int bytes_read		= 0;
    ssize_t written		= 0;
    char buf[BUFSIZE];


    // ex
    if((log_use == 1) && (strlen(share_dir) > 0))
    {
        if(log_init(rsrc) == -1)  // add for _log
            return -1;
    }
    else
        fprintf(stderr, "snarf: no log!\n");
    //~ex

    /* if we already have all of it */
    if( !(rsrc->options & OPT_NORESUME) ) {
        if( rsrc->outfile_size &&
            (rsrc->outfile_offset >= rsrc->outfile_size) ) {
            report(WARN, "you already have all of `%s', skipping",
                   rsrc->outfile);
            final_log(logName,complete_path,rsrc);
            close(sock);
            return 0;
        }
    }

    int type;
    if(rsrc->url->service_type == SERVICE_FTP)
        type = FTP;
    else
        type = HTTP;

    //Allen 20100919 ++
    //fprintf(stderr, "Allen temp test SIZE_4G:%lld rsrc->outfile_size:%lld KB\n",SIZE_4G, rsrc->outfile_size);
    if((rsrc->outfile_size >= SIZE_4G) && (strcmp(mnt_type, "fat32") == 0) ){
        err_log(rsrc, "Files larger then 4GB can not be created on fat32 file system.",type);
        return 0;
    }

    if((rsrc->outfile_size/1024 >= Available_space )&&( Available_space > 0 )){
        err_log(rsrc, "File size is larger then the space left on the partition.",type);
        return 0;
    }
    //

    p = progress_new();

    progress_init(p, rsrc, rsrc->outfile_size);

    if (!(rsrc->options & OPT_NORESUME)) {
        progress_update(p, rsrc->outfile_offset);
        p->offset = rsrc->outfile_offset;
    }

    // read loop


    while( (bytes_read = read(sock, buf, BUFSIZE)) > 0)
    {
        reading_sock = 0;
        if(chk_integrity(rsrc) < 0)
        {
            // test
            fprintf(stderr, "[sn] chk integrty fail [%s]\n", rsrc->outfile);
            close(sock);
            return -100;
        }
        progress_update(p, bytes_read);
        written = write(out_fd, buf, bytes_read);

        if(is_kill == 1)
        {
            close(sock);
            close(out_fd);
            return 0;
        }

        if( written == -1 )
            //if( p->current > 10*1024*1024 )		//for test only
        {
            perror("write");
            //final_log();		//Allen 20101006
            if(is_kill != 1)
            {
                if(errno == ENOSPC)
                    err_log(rsrc,"Disk space is not enough",type);
                else
                    err_log(rsrc, "Write error ! Please check the space left on the partition or else.",type);
            }

            close(sock);

            return 0;
        }

        reading_sock = 1;

    }

    close(sock);
    progress_destroy(p);

    //fprintf(stderr,"exit loop bytes_read=%ld,current_size=%lld,total_size=%lld\n",bytes_read,p->current,rsrc->outfile_size);

    if(p->current < rsrc->outfile_size)
    {
        //fprintf(stderr,"download is incompleted\n");
        logtime = 0;
        return -2;
    }

    handle_complete(rsrc,logName,complete_path,basepath);
}


//off_t
long long int
        get_file_size(const char *file)
{
    struct stat64 file_info;

    if( !(file || *file) )
        return 0;

    if( file[0] == '-' )
        return 0;

    if( stat64(file, &file_info) == -1 )
        return 0;
    else
        return(file_info.st_size);
}



int debug_enabled = 0;

Progress *
        progress_new(void)
{
    Progress *new_progress;

    new_progress = malloc(sizeof(Progress));

    new_progress->tty	= 0;
    new_progress->length 	= 0;
    new_progress->current 	= 0;
    new_progress->offset	= 0;
    new_progress->overflow	= 0;
    new_progress->max_hashes= 0;
    new_progress->cur_hashes= 0;

    new_progress->start_time = double_time();

    return new_progress;
}


int
        //progress_init(Progress *p, UrlResource *rsrc, long int len)
        progress_init(Progress *p, UrlResource *rsrc, long long int len) //modify by gauss

{


    char *filename 	= NULL;
    int win_width	= 0;
    int total_units	= 0;

    if( !p )
        return 0;


    p->rsrc = rsrc;

    //printf("p option is %c \n");
#ifdef PROGRESS_DEFAULT_OFF
    #ifndef PROGRESS_DEFAULT_OFF
        if( rsrc->options & OPT_PROGRESS ) {
            p->tty = 1;

            printf("set p tty to 1\n");
        } else {
            if( (!isatty(2)) || (rsrc->outfile[0] == '-') ||
                (rsrc->options & OPT_QUIET) ) {

                p->tty = 0;



                //return 1;
            } else {
                p->tty = 1;
                printf("else set p tty to 1\n");
            }
        }
    #else
        if( rsrc->options & OPT_PROGRESS )
            p->tty = 1;
        else {
            p->tty = 0;
            return 1;
        }
    #endif /* PROGRESS_DEFAULT_OFF */
#endif /* PROGRESS_DEFAULT_OFF */
    win_width = guess_winsize();

    if( win_width > 30 )
        total_units = win_width - (30 + 9 + 16);
    else {
        /* the window is pathetically narrow; bail */
        total_units = 20;
    }


    p->length = len;

    /* buffering stderr: no flicker! yay! */
    setbuf(stderr, (char *)&output_buf);

    //fprintf(stderr, "%s (", rsrc->url->full_url);

    if( total_units && len ) {
        p->length = len;
        p->max_hashes = total_units;
        //fprintf(stderr, "%dK", (int )(len / 1024));
    } else {
        //fprintf(stderr, "unknown size");
    }

    //fprintf(stderr, ")\n");

    p->current = 0;

    filename = strdup(rsrc->outfile);

    if( strlen(filename) > 24 )
        filename[24] = '\0';

    //fprintf(stderr, "%-25s[", filename);

    if( p->length )
        repchar(stderr, ' ', p->max_hashes);
    else
        fputc('+', stderr);

    //fprintf(stderr, "] %7dK", (int )p->current);
    fflush(stderr);
    return 1;
}

void
        pause_log(){

    int fd;

    stopp = 0;
#if SEM
    if(semUse)
        Sem_wait(&sem);
#endif
    fd = open(logName, O_RDWR|O_CREAT|O_TRUNC, 0777);

     if(fd != -1)
    {
         log_s.status = S_PAUSED;
         write(fd, &log_s, LOG_SIZE);
         close(fd);

     }

#if SEM
     if(semUse)
         Sem_post(&sem);
#endif
     kill(getpid(), SIGSTOP);

}

int SecondToDate(int sec,char *time)
{
  int second,minute,hour;
  char csec[3],cmin[3],chour[3];
  hour = sec / 3600;
  minute = (sec - hour * 3600) / 60 ;
  second = sec - hour * 3600 - minute *60;  
 
  //char time[20];
  //memset(time,'\0',sizeof(time));

 sprintf(time,"%2d:%2d:%2d",hour,minute,second);

 int i;
 for(i=0;time[i] != '\0';i++)
        if(time[i]== ' ')
                time[i] = '0';


//return time;
 
}



void
        progress_update(Progress *	p,
                        long long int 	increment) //modify by gauss
{
    unsigned int units;
    float  percent_done  = 0;
    double  elapsed = 0;
    float  rate = 0;
    char *anim = "-\\|/";
    int  fd;
    int t_left;

    //printf("Enter progress_update() \n");

    //char log_test[256];
    //memset(log_test,0,sizeof(log_test));
    //sprintf(log_test,"%s/log_test",basepath);
#ifdef PROGRESS_DEFAULT_OFF
    if( !(p->tty) ){
        //return;
        }
#endif


    p->current += increment;

   // if (strlen(p->rsrc->outfile) > 24) {
    //    p->rsrc->outfile[24] = '\0';
   // }

    //fprintf(stderr, "\r");
   // fprintf(stderr, "%-25s [", p->rsrc->outfile);


    if( p->length ) {
        percent_done = (float )p->current / p->length;
        //double elapsed;
        //float rate;

        /*units = percent_done * p->max_hashes;
        if( units )
            repchar(stderr, '#', units);
        repchar(stderr, ' ', p->max_hashes - units);
        fprintf(stderr, "] ");
        fprintf(stderr, "%7dK", (int )(p->current / 1024));*/

        elapsed = double_time() - p->start_time;
        //printf("elapsed time is %f",elapsed);

        if (elapsed)
        {
            rate = ((p->current - p->offset) / elapsed) / 1024;


        }
        else
            rate = 0;

        /* The first few runs give extra-high values, so skip them */
        if (rate > 999999)
            rate = 0;

        //fprintf(stderr, " | %7.2fK/s", rate);
        //csprintf("%7.2fK/s", rate);

    } else {
        /* length is unknown, so just draw a little spinny thingy */
        //fprintf(stderr, "%c]", anim[p->frame++ % 4]);
        //fprintf(stderr, " %7dK", (int )(p->current / 1024));

    }

    fflush(stderr);

    // ex
    if((log_use == 1) && (share_dir != NULL) && ( (int)elapsed - logtime > 4 || logtime==0 ))
    //if((log_use == 1) && (share_dir != NULL) )
    {
        //printf("##########update log #######\n");
        log_s.status = S_PROCESSING;
        time(&log_s.now_t);

        switch(log_s.download_type)
        {
        case HTTP:
            log_s.ifile.iHttp.elapsed = elapsed;
            log_s.ifile.iHttp.rate = rate;
            log_s.progress = percent_done;

            //add info by gauss
            strcpy(log_s.ifile.iHttp.Destination,info_dest_path);
            log_s.ifile.iHttp.Created_time=log_s.now_t;
            strcpy(log_s.ifile.iHttp.URL,p->rsrc->url->full_url);
            log_s.ifile.iHttp.Start_Time=log_s.begin_t;
            if(rate > 0 )
            {
                t_left = (int)( (p->length - p->current)/1024 / rate );
                SecondToDate(t_left,log_s.ifile.iHttp.Time_Left);
            }
            else
            {
                strcpy(log_s.ifile.iHttp.Time_Left,"99:99:99");
            }


            break;
        case FTP:
            log_s.ifile.iFtp.elapsed = elapsed;
            log_s.ifile.iFtp.rate = rate;
            log_s.progress = percent_done;

            strcpy(log_s.ifile.iFtp.Destination,info_dest_path);
            log_s.ifile.iFtp.Created_time=log_s.now_t;
            strcpy(log_s.ifile.iFtp.URL,p->rsrc->url->full_url);
            log_s.ifile.iFtp.Start_Time=log_s.begin_t;
            if(rate > 0 )
            {
                t_left = (int)( (p->length - p->current)/1024 / rate );
                SecondToDate(t_left,log_s.ifile.iFtp.Time_Left);
            }
            else
            {
                strcpy(log_s.ifile.iFtp.Time_Left,"99:99:99");
            }

            break;
        }

        writing_log = 1;
#if SEM
        if(semUse)
            Sem_wait(&sem);
#endif
        if((fd = open(logName,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
        //if((fd = open(logName,O_RDWR|O_CREAT|O_TRUNC|O_SYNC, 0777))<0)
        {
            perror("Open log error\n");
        }
        else
        {
            int len_t= write(fd, &log_s, LOG_SIZE);

            if(len_t  < 0)
            {
               //fprintf(stderr, "snarf: write log error\n");
               //printf("snarf: write log error\n");
            }
            else
            {
                //printf("write size is %d \n",len_t);
            }

            close(fd);
        }
#if SEM
        if(semUse)
            Sem_post(&sem);
#endif

        if(stopp == 1){ // 2007_fix
            stopp = 0;
#if SEM
            if(semUse)
                Sem_wait(&sem);
#endif
            fd = open(logName, O_RDWR|O_CREAT|O_TRUNC, 0777);
            if(fd != -1)
            {
                log_s.status = S_PAUSED;
                write(fd, &log_s, LOG_SIZE);
                close(fd);
            }
#if SEM
            if(semUse)
                Sem_post(&sem);
#endif
            kill(getpid(), SIGSTOP);
        }
        writing_log = 0;

        logtime += 4 ;
    }
    //~ex
    return;
}


void 
        progress_destroy(Progress *p)
{
    double elapsed = 0;
    double kbytes;
#ifdef PROGRESS_DEFAULT_OFF
    if( p && (!p->tty) )
        return;
#endif

    elapsed = double_time() - p->start_time;

    //fprintf(stderr, "\n");

    if( elapsed ) {
        kbytes = ((float )(p->current - p->offset) / elapsed) / 1024.0;
        //fprintf(stderr, "%lld bytes transferred in %.2f sec (%.2fk/sec)\n",p->current - p->offset, elapsed, kbytes);
    } else {
        //fprintf(stderr, "%lld bytes transferred in less than a second\n", p->current);
    }
    fflush(stderr);

    safe_free(p);
}




/* taken from glib */
char *
        strconcat (const char *string1, ...)
{
    unsigned int   l;
    va_list args;
    char   *s;
    char   *concat;

    l = 1 + strlen (string1);
    va_start (args, string1);
    s = va_arg (args, char *);

    while( s ) {
        l += strlen (s);
        s = va_arg (args, char *);
    }
    va_end (args);
    concat = malloc(l);
    concat[0] = 0;

    strcat (concat, string1);
    va_start (args, string1);
    s = va_arg (args, char *);
    while (s) {
        strcat (concat, s);
        s = va_arg (args, char *);
    }
    va_end (args);

    return concat;
}


/* written by lauri alanko */
char *
        base64(char *bin, int len)
{
    char *buf= (char *)malloc((len+2)/3*4+1);
    int i=0, j=0;

    char BASE64_END = '=';
    char base64_table[64]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789+/";

    while( j < len - 2 ) {
        buf[i++]=base64_table[bin[j]>>2];
        buf[i++]=base64_table[((bin[j]&3)<<4)|(bin[j+1]>>4)];
        buf[i++]=base64_table[((bin[j+1]&15)<<2)|(bin[j+2]>>6)];
        buf[i++]=base64_table[bin[j+2]&63];
        j+=3;
    }

    switch ( len - j ) {
    case 1:
        buf[i++] = base64_table[bin[j]>>2];
        buf[i++] = base64_table[(bin[j]&3)<<4];
        buf[i++] = BASE64_END;
        buf[i++] = BASE64_END;
        break;
    case 2:
        buf[i++] = base64_table[bin[j] >> 2];
        buf[i++] = base64_table[((bin[j] & 3) << 4)
                                | (bin[j + 1] >> 4)];
        buf[i++] = base64_table[(bin[j + 1] & 15) << 2];
        buf[i++] = BASE64_END;
        break;
    case 0:
        break;
    }
    buf[i]='\0';
    return buf;
}


void
        report(enum report_levels lev, char *format, ...)
{
    switch( lev ) {
    case DEBUG:
        fprintf(stderr, "debug: ");
        break;
    case WARN:
        fprintf(stderr, "warning: ");

        break;
    case ERR:
        fprintf(stderr, "error: ");
        break;
    default:
        break;
    }

    if( format ) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
}


int
        tcp_connect(char *remote_host, int port)
{
    struct hostent *host;
    struct sockaddr_in sa;
    int sock_fd;


    resolve_hostname = 1;
    if((host = (struct hostent *)gethostbyname(remote_host)) == NULL) {
        herror(remote_host);
        return 0;
    }
     resolve_hostname = 0;

    /* get the socket */
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 0;
    }

    /* connect the socket, filling in the important stuff */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr, host->h_addr,host->h_length);

    socket_connect = 1 ;
    if(connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
        perror(remote_host);
        return 0;
    }
    socket_connect = 0 ;

    return sock_fd;
}


#ifndef HAVE_STRDUP
char *
        strdup(const char *s)
{
    char *new_string = NULL;

    if (s == NULL)
        return NULL;

    new_string = malloc(strlen(s) + 1);

    strcpy(new_string, s);

    return new_string;
}
#endif /* HAVE_STRDUP */


int
        transfer(UrlResource *rsrc)
{
    int i;
    switch (rsrc->url->service_type) {
    case SERVICE_HTTP:
        i = http_transfer(rsrc);
        break;
    case SERVICE_FTP:
        i = ftp_transfer(rsrc);
        break;
    case SERVICE_GOPHER:
        i = gopher_transfer(rsrc);
        break;
    default:
        report(ERR, "bad url: %s", rsrc->url->full_url);
    }

    return i;
}

int get_real_download_filename(char *original_name,char *real_name)
{
    char *p = NULL;
    if(original_name == NULL)
    {
        strcpy(real_name,"index.html");
    }
    else
    {
        p = strchr(original_name,'?');
        if( p != NULL)
        {
            int pos = strlen(original_name) - strlen(p);
            strncpy(real_name,original_name,pos);
        }
        else
            strcpy(real_name,original_name);
    }

    //fprintf(stderr,"real_name=%s\n",real_name);
    return 0;
}

static void *xmalloc_fatal(size_t size) {
    if (size==0) return NULL;
    fprintf(stderr, "Out of memory.");
    //exit(1);
    return;
}

void *xmalloc (size_t size) {
    void *ptr = malloc (size);
    if (ptr == NULL) return xmalloc_fatal(size);
    return ptr;
}

void *xcalloc (size_t nmemb, size_t size) {
    void *ptr = calloc (nmemb, size);
    if (ptr == NULL) return xmalloc_fatal(nmemb*size);
    return ptr;
}

void *xrealloc (void *ptr, size_t size) {
    void *p = realloc (ptr, size);
    if (p == NULL) return xmalloc_fatal(size);
    return p;
}

char *xstrdup (const char *s) {
    void *ptr = xmalloc(strlen(s)+1);
    strcpy (ptr, s);
    return (char*) ptr;
}

char *oauth_url_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;

        switch(in){
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': case '~': case '.': case '-':
            ns[strindex++]=in;
            break;
        default:
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
            break;
        }
        string++;
    }
    ns[strindex]=0;
    return ns;
}

char *space_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;
        if(in != ' ')
        {
            ns[strindex++]=in;
        }
        else
        {
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
        }

        string++;
    }
    ns[strindex]=0;
    return ns;
}

char *oauth_url_unescape(const char *string, size_t *olen) {
    size_t alloc, strindex=0;
    char *ns = NULL;
    unsigned char in;
    long hex;

    if (!string) return NULL;
    alloc = strlen(string)+1;
    ns = (char*) xmalloc(alloc);

    while(--alloc > 0) {
        in = *string;
        if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
            char hexstr[3]; // '%XX'
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;
            hex = strtol(hexstr, NULL, 16);
            in = (unsigned char)hex; /* hex is always < 256 */
            string+=2;
            alloc-=2;
        }
        ns[strindex++] = in;
        string++;
    }
    ns[strindex]=0;
    if(olen) *olen = strindex;
    return ns;
}
