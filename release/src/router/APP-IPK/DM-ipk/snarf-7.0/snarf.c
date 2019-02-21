/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */
/* This program and all accompanying files are copyright 1998 Zachary
 * Beane <xach@xach.com>. They come with NO WARRANTY; see the file COPYING
 * for details.
 * 
 * This program is licensed to you under the terms of the GNU General
 * Public License. You should have recieved a copy of it with this
 * program; if you did not, you can view the terms at http://www.gnu.org/.
 *
 */



#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "options.h"
#include "llist.h"
#include "url.h"
#include "http.h"
#include "ftp.h"
#include "gopher.h"
#include "util.h"
#include "logs.h"

#define LATEST_URL "http://www.xach.com/snarf/snarf-latest.tar.gz"

#ifdef PROGRESS_DEFAULT_OFF
#define PROGRESS_SETTING "off"
#else
#define PROGRESS_SETTING "on"
#endif
#define MAXDATASIZE 500

//extern char share_dir[MAX_NAMELEN];
//extern int log_use;
int log_use;
char share_dir[MAX_NAMELEN];
int is_kill = 0;
int reading_sock = 0 ;

//int create_initlog = 0;
//int create_errorlog = 0;
//int create_nomarllog = 0;

int resolve_hostname = 0;
int socket_connect = 0;
int socket_write = 0 ;
int socket_read = 0 ;

char basic_path[256];
char real_name[256];




UrlResource *rsrc	= NULL;

static void

usage(int verbose)
{
        if (!verbose) {
                fprintf(stderr, "Use `snarf --help' for help\n");
                exit(1);
        }

        printf("This is snarf, version %s\n", VERSION);
        printf("usage: snarf [OPTIONS] URL [OUTFILE] ...\n");
        printf("Options:\n"
               "    -a     Force active FTP (default is passive)\n"
               "    -v     Verbose; print anything the server sends\n" 
               "    -q     Don't print progress bars (compiled default is %s)\n"
               "    -p     Force printing of progress bars (overrides -q)\n"
               "    -r     Resume downloading a partially transferred file\n"
               "    -n     Ignore '-r' and transfer file in its entirety\n"
               "    -m     Spoof MSIE user-agent string\n"
               "    -z     Spoof Navigator user-agent string\n"
               "\n"
               "Lowercase option letters only affect the URLs that "
               "immediately follow them.\n"
               "If you give an option in caps, it will be the "
               "default option for all URLs\nthat follow it.\n"
               "\n"
               "If you specify the outfile as '-', the file will be "
               "printed to standard\noutput as it downloads.\n"
               "\n"
               "You can have as many URLs and outfiles as you want\n"
               "\n"
               "You can specify a username and password for ftp or "
               "http authentication. The\nformat is:\n"
               "\n"
               "    ftp://username:password@host/\n"
               "\n"
               "If you don't specify a password, you will be prompted "
                "for one.\n"
               "\n"
               "snarf checks the SNARF_PROXY, FTP_PROXY, GOPHER_PROXY, "
               "HTTP_PROXY, and PROXY\nenvironment variables.\n"
               "\n"
               "snarf is free software and has NO WARRANTY. See the file "
               "COPYING for details.\n", PROGRESS_SETTING);

        exit(1);
}
                
void set_kill()
{
    is_kill = 1;
    //printf("set is_kill to 1 \n");

    //if( (create_init != 1) || (create_init == 1 && create_errorlog !=1 ) )
        //exit(0);

    if( resolve_hostname || socket_connect || socket_write || socket_read || reading_sock )
    {
        url_resource_destroy(rsrc);
        exit(0);
    }
}

//20121220 magic added{
struct disk_info *initial_disk_data(struct disk_info **disk)
{
    struct disk_info *follow_disk;

    if(disk == NULL)
        return NULL;

    *disk = (struct disk_info *)malloc(sizeof(struct disk_info));
    if(*disk == NULL)
        return NULL;

    follow_disk = *disk;

    follow_disk->diskname = NULL;
    follow_disk->diskpartition = NULL;
    follow_disk->mountpath = NULL;
    follow_disk->serialnum = NULL;
    follow_disk->product = NULL;
    follow_disk->vendor = NULL;
    follow_disk->disktype = NULL;
    follow_disk->next = NULL;
    follow_disk->port = (unsigned int)0;
    follow_disk->partitionport = (unsigned int )0;

    return follow_disk;
}

void free_disk_struc(struct disk_info **disk)
{
    struct disk_info *follow_disk, *old_disk;

    if(disk == NULL)
        return;

    follow_disk = *disk;
    while(follow_disk != NULL)
    {
        if(follow_disk->diskname != NULL)
            free(follow_disk->diskname);
        if(follow_disk->diskpartition != NULL)
            free(follow_disk->diskpartition);
        if(follow_disk->mountpath != NULL)
            free(follow_disk->mountpath);
        if(follow_disk->serialnum != NULL)
            free(follow_disk->serialnum);
        if(follow_disk->product != NULL)
            free(follow_disk->product);
        if(follow_disk->vendor != NULL)
            free(follow_disk->vendor);
		  if(follow_disk->disktype != NULL)
            free(follow_disk->disktype);
        old_disk = follow_disk;
        follow_disk = follow_disk->next;
        free(old_disk);
    }
}

int init_diskinfo_struct()
{
    int len = 0;
    FILE *fp;
    if(access("/tmp/usbinfo",0)==0)
    {
        fp =fopen("/tmp/usbinfo","r");
        if(fp)
        {
            fseek(fp,0,SEEK_END);
            len = ftell(fp);
            fseek(fp,0,SEEK_SET);
        }
    }
    else
        return -1;

    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    fread(buf,1,len,fp);
    fclose(fp);

    if(initial_disk_data(&follow_disk_tmp) == NULL){
        return -1;
    }
    if(initial_disk_data(&follow_disk_info_start) == NULL){
        return -1;
    }

    follow_disk_info = follow_disk_info_start;
    //get diskname and mountpath
    char a[1024];
    char *p,*q;
    fp = fopen("/proc/mounts","r");
    if(fp)
    {
       while(!feof(fp))
        {
           memset(a,'\0',sizeof(a));
           fscanf(fp,"%[^\n]%*c",a);
           if((strlen(a) != 0)&&((p=strstr(a,"/dev/sd")) != NULL))
           {
               singledisk++;
               if(singledisk != 1){
                   initial_disk_data(&follow_disk_tmp);
               }
               p = p + 5;
               follow_disk_tmp->diskname=(char *)malloc(5);
               memset(follow_disk_tmp->diskname,'\0',5);
               strncpy(follow_disk_tmp->diskname,p,4);

               p = p + 3;
               follow_disk_tmp->partitionport=atoi(p);
               if((q=strstr(p,"/tmp")) != NULL)
               {
                   if((p=strstr(q," ")) != NULL)
                   {
                       follow_disk_tmp->mountpath=(char *)malloc(strlen(q)-strlen(p)+1);
                       memset(follow_disk_tmp->mountpath,'\0',strlen(q)-strlen(p)+1);
                       strncpy(follow_disk_tmp->mountpath,q,strlen(q)-strlen(p));
                   }
                   p++;//eric added for disktype
                   if((q=strstr(p," ")) != NULL)
                   {
                       follow_disk_tmp->disktype=(char *)malloc(strlen(p)-strlen(q)+1);
                       memset(follow_disk_tmp->disktype,'\0',strlen(p)-strlen(q)+1);
                       strncpy(follow_disk_tmp->disktype,p,strlen(p)-strlen(q));
                   }//eric added for disktype
               }
               char diskname_tmp[4];
               memset(diskname_tmp,'\0',sizeof(diskname_tmp));
               strncpy(diskname_tmp,follow_disk_tmp->diskname,3);
               if((p=strstr(buf,diskname_tmp)) != NULL)
               {
                   p = p - 6;
                   follow_disk_tmp->port=atoi(p);
                   follow_disk_tmp->diskpartition=(char *)malloc(4);
                   memset(follow_disk_tmp->diskpartition,'\0',4);
                   strncpy(follow_disk_tmp->diskpartition,diskname_tmp,3);
                   q=strstr(p,"_serial");
                   q = q + 8;
                   p=strstr(q,"_pid");
                   follow_disk_tmp->serialnum=(char *)malloc(strlen(q)-strlen(p)-4);
                   memset(follow_disk_tmp->serialnum,'\0',strlen(q)-strlen(p)-4);
                   strncpy(follow_disk_tmp->serialnum,q,strlen(q)-strlen(p)-5);
                   p = p + 5;
                   q=strstr(p,"_vid");
                   follow_disk_tmp->product=(char *)malloc(strlen(p)-strlen(q)-4);
                   memset(follow_disk_tmp->product,'\0',5);
                   strncpy(follow_disk_tmp->product,p,strlen(p)-strlen(q)-5);
                   q = q + 5;
                   follow_disk_tmp->vendor=(char *)malloc(5);
                   memset(follow_disk_tmp->vendor,'\0',5);
                   strncpy(follow_disk_tmp->vendor,q,4);
               }

               follow_disk_info->next =  follow_disk_tmp;
               follow_disk_info = follow_disk_tmp;
           }
       }
    }
    fclose(fp);
        return 0;
}
//20121220 magic added}

int
main(int argc, char *argv[])
{
    signal(SIGUSR1,set_kill);
    List *arglist		= NULL;
    //UrlResource *rsrc	= NULL;
    Url *u			= NULL;
    int retval		= 0;
    int i;
    int logid = 0;
    char logpath[256];
    char downloadpath[256];
    char completepath[256];

    arglist = list_new();
    rsrc 	= url_resource_new();
    //20121220 magic added{
    if(init_diskinfo_struct()==-1)
        fprintf(stderr,"\ninit_diskinfo_struct failed\n");
    FILE *fp;
    memset(generalbuf,'\0',sizeof(generalbuf));
    memset(serialtmp,'\0',sizeof(serialtmp));
    memset(producttmp,'\0',sizeof(producttmp));
    memset(vondertmp,'\0',sizeof(vondertmp));
    fp = fopen("/opt/etc/dm2_general.conf","r");
    if(fp)
    {
        while(!feof(fp))
        {
            fscanf(fp,"%[^\n]%*c",generalbuf);
            if(strncmp(generalbuf,"serial=",7) == 0)
            {
                strncpy(serialtmp,generalbuf+7, sizeof(serialtmp));
            }
            else if(strncmp(generalbuf,"vonder=",7) == 0)
            {
                strncpy(vondertmp,generalbuf+7, sizeof(vondertmp));
            }
            else if(strncmp(generalbuf,"product=",8) == 0)
            {
                strncpy(producttmp,generalbuf+8, sizeof(producttmp));
            }
            else if(strncmp(generalbuf,"partition=",10) == 0)
            {
                partitiontmp = atoi(generalbuf+10);
            }
        }
    }
    fclose(fp);
    //20121220 magic added}

    if(argc < 2) {
        fprintf(stderr, "snarf: not enough arguments\n");
        usage(0); /* usage() exits */
    }

    if (strcmp(argv[1], "--version") == 0) {
        printf("snarf %s\n", VERSION);
        exit(0);
    }

    if (strcmp(argv[1], "--help") == 0) {
        usage(1);
    }

    for(i = 1; --argc; i++) {
        if( strcmp(argv[i], "LATEST") == 0 ) {
            u = url_new();
            if( !url_init(u, LATEST_URL) ) {
                report(ERR, "`%s' is not a valid URL");
                continue;
            }

            rsrc->url = u;
            continue;
        }

        // first get log id
        if(logid == 0)
        {
            logid = atoi(argv[i]);

            i++;
            --argc;

            memset(logpath, 0, sizeof(logpath));
            memcpy(logpath, argv[i], sizeof(logpath));

            memset(basic_path, 0, sizeof(basic_path));
            strncpy(basic_path,logpath,strlen(logpath)-6);

            i++;
            --argc;
            memset(downloadpath, 0, sizeof(downloadpath));
            //memcpy(downloadpath, argv[i], sizeof(downloadpath));
            strcpy(downloadpath,argv[i]);
            //printf("download path: %s\n", downloadpath);
            //fprintf(stderr,"download path: %s\n", downloadpath);

            i++;
            --argc;
            memset(completepath, '\0', sizeof(completepath));
            //memcpy(downloadpath, argv[i], sizeof(downloadpath));
            strncpy(completepath, argv[i], sizeof(completepath));

            continue;
        }

        /* options */
        if( argv[i][0] == '-' && argv[i][1] ) {
            if( rsrc->url ) {
                list_append(arglist, rsrc);
                rsrc = url_resource_new();
            }

            rsrc->options = set_options(rsrc->options, argv[i]);
            continue;
        }

        /* everything else */
        if( !rsrc->url ) {
            u = url_new();
            if( !url_init(u, argv[i]) ) {
                report(ERR, "bad url `%s'", argv[i]);
                continue;
            }
            rsrc->url = u;
        } else if( is_probably_an_url(argv[i]) ) {
            list_append(arglist, rsrc);
            rsrc = url_resource_new();

            u = url_new();
            if( !url_init(u, argv[i]) ) {
                report(ERR, "bad url `%s'", argv[i]);
                continue;
            }
            rsrc->url = u;
        } else {
            if( rsrc->outfile )
                report(WARN, "ignoring `%s' for outfile",
                       argv[i]);
            else
                rsrc->outfile = strdup(argv[i]);
        }
    }

    if( rsrc->url ) {
        list_append(arglist, rsrc);
    }

    if (arglist->data == NULL) {
        fprintf(stderr, "snarf: not enough arguments\n");
        usage(0);
    }

    // init log
    //create init log before send request

    //fprintf(stderr,"create init log start\n");

    Log_struc log_s;
    char log_name[256];
    memset(&log_s, '\0', sizeof(log_s));
    memset(log_name,0,sizeof(log_name));
    sprintf(log_name,"%s/snarf_%d",logpath,logid);

    //log_s.id = getpid();
    sprintf(log_s.id,"%d",getpid());
    //log_s.filename = rsrc->url->file;


    get_real_download_filename(rsrc->url->file,real_name);
    strcpy(log_s.filename,real_name);
    if(rsrc->url->service_type == SERVICE_HTTP)
    {
        log_s.download_type = HTTP;
        log_s.ifile.iHttp.rate = 0;
    }
    else
    {
        log_s.download_type = FTP;
        log_s.ifile.iFtp.rate = 0;
    }
    log_s.status = S_PROCESSING;

    int fd;
    if((fd = open(log_name,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
    {
        perror("Open log error\n");
        //fprintf(stderr,"open %s error\n",log_name);
    }
    else
    {
        //fprintf(stderr,"start write init log\n");
        //create_initlog = 1;
        write(fd, &log_s, LOG_SIZE);
        close(fd);

        if(is_kill == 1)
        {
            url_resource_destroy(rsrc);
            //create_initlog = 1;
            exit(0);
        }
    }


    memset(share_dir , '\0', sizeof(share_dir));
    sprintf(share_dir, "%s", ".");
    log_use = 1;
    init_log(logpath,completepath,logid);

    /* walk through the arglist and output and do that thing you
           do so well */

    while (arglist != NULL) {
        rsrc = arglist->data;
        arglist = arglist->next;
        u = rsrc->url;

        if( !rsrc->outfile && u->file )
        {
            int tlen = strlen(downloadpath)+strlen("/")+strlen(real_name)+1;
            char *tmp = (char*)malloc(tlen);
            memset(tmp, 0 , tlen);
            sprintf(tmp, "%s/%s", downloadpath, real_name);
            rsrc->outfile = strdup(tmp);
            free(tmp);
        }

        if( rsrc->outfile ) {
            rsrc->outfile_offset = get_file_size(rsrc->outfile);

            if (rsrc->outfile_offset &&
                    !(rsrc->options & OPT_NORESUME)) {
                fprintf(stderr, "setting resume on "
                        "existing file `%s' at %ld bytes\n",
                        rsrc->outfile, rsrc->outfile_offset);
                rsrc->options |= OPT_RESUME;
            }
        }


        if (u->username && !u->password) {
            char *prompt = NULL;

            prompt = strconcat("Password for ",
                               u->username, "@",
                               u->host, ": ", NULL);

            u->password = strdup(getpass(prompt));
            free(prompt);
        }

        i = transfer(rsrc);
        if (i == 0)
            retval++;
        if(i == -2)    //download no complete
        {
            //logtime = 0;
            arglist = list_new();
            list_append(arglist, rsrc);
            //if()
        }
        if(i == -3)
        {
            char basepath[128] = {0};
            strncpy(basepath,logpath,strlen(logpath)-6);
            handle_complete(rsrc,log_name,completepath,basepath);
            url_resource_destroy(rsrc);
        }
        else
            url_resource_destroy(rsrc);
    }

    free_disk_struc(&follow_disk_info_start); //20121220 magic added
    return retval;
}