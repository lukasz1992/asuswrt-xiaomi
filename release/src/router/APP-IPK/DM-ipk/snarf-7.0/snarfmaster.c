#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include "logs.h"
#include "ex.h"

#define MYPORT 3490 /* user port */
#define BACKLOG 10 /* max listen num*/
#define MAXDATASIZE 1024 /* max rev num */
#define CONFIG_PATH "/opt/etc/dm2_snarf.conf"
#define CONFIG_NEW_PATH "/opt/etc/dm2_general.conf"
#define CONFIG_ROUTER_PATH "/tmp/asus_router.conf"
#define SEM 0

int socket_port;
char logpath[256];
char downloadpath[256];
char sempath[256];
char basepath[256];
char ex_downloadpath[256];
char completepath[256];
int semUse;


int sem_init(char *path,int logid){

       //printf("enter sem_int\n");

        char semName[256];

        int  flag_sem = O_CREAT|O_RDWR;
        int  init_value = 1;

        Sem_t sem;

        memset(semName,0,sizeof(semName));
        sprintf(semName,"%s/sem.snarf_%d",path,logid);



         semUse = 1;

        if (Sem_open(&sem, semName, flag_sem, FILE_MODE, init_value) == (-1)){
                semUse = 0;
                perror("open sem failed");
                return -1;
                }

        return 0;
}

struct Lognote{
    int id;
    char url[512];
    int status;
    //int pid;// add by gauss
    //Sem_t sem;
    struct Lognote *next;
};

struct Lognote *head;

int cmd_parser(char* str_cmd, int pid, struct Lognote *phead);

/* create a notet*/
struct Lognote * createnote(int id, char *url, int status)
{
    struct Lognote *note = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(note, 0, sizeof(struct Lognote *));
    note->id = id;
    strcpy(note->url, url);
    note->status = status;
    //note->pid = pid;
    return note;
}

/* run the not completed snarf */
void initsnarf(struct Lognote *phead)
{
    struct Lognote* p1 = phead->next;
    while(p1 != NULL)
    {
        if(p1->status == S_PROCESSING)
        {
            // process
            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "./dm2_snarf %d %s %s \"%s\" \"%s\" &", p1->id, logpath, downloadpath,completepath, p1->url);
            if (!fork()) {
                system(tmp);
                exit(0);
            }
        }
        p1 = p1->next;
    }
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

void initlognote(char *path, struct Lognote *phead)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path);
      while (NULL != (ent=readdir(pDir)))
    {
         //if (ent->d_reclen==24 || ent->d_reclen==20)
        //{
           // if (ent->d_type==0 || ent->d_type==8)
            //{
                char *p = NULL ;
                p = strstr(ent->d_name,"snarf");
                    if(p)
                       {
                            //printf("file:%s\n", ent->d_name);
                            Log_struc log_ts;
                            memset(&log_ts, 0, sizeof(Log_struc));
                            int fd1;
                            char tmp_name[256];
                            memset(tmp_name, 0, sizeof(tmp_name));
                            sprintf(tmp_name, "%s/%s", logpath, ent->d_name);
                            //     printf("tmp_name=%s\n", tmp_name);

                            if((fd1 = open(tmp_name, O_RDONLY | O_NONBLOCK)) < 0)
                            {
                                printf("\nread log error!\n");
                            }
                            else
                            {
                            int len = read(fd1, &log_ts, sizeof(Log_struc));
                            close(fd1);

                            int id = atoi(ent->d_name+6);

                            struct Lognote *note = createnote(id, log_ts.fullname, log_ts.status);
                            insertnote(note, phead);
                           }
                      }

                //printf("####   after insertnot()\n");

            //}

        //}
    }
      if(pDir)
        closedir(pDir);
}


/* create and add a note to list*/
int addlognote(char *url, struct Lognote *phead)
{
    int i = 1;
    //    int isInsert = 0;
    struct Lognote *p1, *p2;
    p2 = phead;
    p1 = phead->next;

    while(p1 != NULL)
    {
        if(p1->id != i)
        {
            break;
        }

        i++;
        p2 = p1;
        p1 = p1->next;
    }

    struct Lognote *note = createnote(i, url, S_PROCESSING);

    p2->next = note;
    note->next = p1;


    return note->id;
}

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
}

struct Lognote* getlognote(int id, struct Lognote *phead)
{
    struct Lognote* p1 = phead->next;
    while(p1 != NULL)
    {
        if(p1->id == id)
        {           
            return p1;
        }
        p1 = p1->next;
    }

    return NULL;
}

//void freelognote(int signo)
void freelognote()
{
    //printf("free res\n");
    struct Lognote *p1, *p2;
    p1 = head->next;
    p2 = head;

    while(p1 != NULL)
    {
        free(p2);

        p2 = p1;
        p1 = p1->next;
    }
    free(p2);
    free(head);
}
void print_log(struct Lognote *phead)
{
    struct Lognote* p1 = phead->next;
    while(p1 != NULL)
    {
        printf("id = %d\n", p1->id);
        printf("url = %s\n", p1->url);
        printf("status = %d\n", p1->status);
        p1 = p1->next;
    }
}



void InitSnarfCfg(char *path)
{
    int fd, len, i=0,name_len;
    name_len=strlen(path);
    //printf("name lenth is %d\n",name_len);
    char ch, tmp[256], name[256], content[256];
    memset(tmp, 0, sizeof(tmp));
    memset(name, 0, sizeof(name));
    memset(content, 0, sizeof(content));
    char s[] = "PORT=3490\nLOG_PATH=/Download2/.logs\nSEM_PATH=/Download2/.sems\nDOWNLOAD_PATH=/Download2/InComplete";
    if(access(path,0) != 0)
    {
        fprintf(stderr,"\nconfig is not exists\n");
        FILE *fp;
        fp = fopen("/opt/etc/dm2_snarf.conf","w+");
        if(fp != NULL)
        {
            fprintf(fp,"%s",s);
        }
        fclose(fp);
    }

    if((fd = open(path, O_RDONLY | O_NONBLOCK)) < 0)
    {
        printf("\nread log error!\n");
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

                    if(strcmp(name, "PORT") == 0)
                    {
                        socket_port = atoi(content);
                    }
                    else if(strcmp(name, "LOG_PATH") == 0)
                    {
                        strcpy(logpath, content);

                    }
                    else if(strcmp(name, "SEM_PATH") == 0)
                    {
                        strcpy(sempath, content);

                    }
                    else if(strcmp(name, "BASE_PATH") == 0)
                    {
                        if(strlen(basepath) == 0)
                            strcpy(basepath, content);
                    }
                    else if(strcmp(name, "EX_DOWNLOAD_PATH") == 0)
                    {
                        strcpy(ex_downloadpath, content);

                    }
                    else if(strcmp(name, "DOWNLOAD_PATH") == 0  )
                    {
                        strcpy(downloadpath, content);

                    }
                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
      }
    if(strstr(path,"dm2_snarf.conf") != NULL)
    {
        if(socket_port == 0)
            socket_port = 3490;
        if(strlen(logpath) == 0)
            strcpy(logpath,"/Download2/.logs");
        if(strlen(sempath) == 0)
            strcpy(sempath,"/Download2/.sems");
        if(strlen(downloadpath) == 0)
            strcpy(downloadpath,"/Download2/InComplete");
        FILE *fp;
        fp = fopen("/opt/etc/dm2_snarf.conf","w+");
        if(fp != NULL)
        {
            fprintf(fp,"%s",s);
        }
        fclose(fp);
    }

}

//add by gauss
int CompareCfg(char *path,char *new_path)
{


        char temp[256];

        InitSnarfCfg(path);
        InitSnarfCfg(new_path);

        memset(temp,0,sizeof(temp));
        sprintf(temp,"%s%s",basepath,logpath);
        strcpy(logpath,temp);
        DIR *pDir;
        if( ( pDir = opendir(logpath) )== NULL)
        {
            memset(temp, 0, sizeof(temp));
            sprintf(temp, "mkdir  -p %s", logpath);
            system(temp);
        }
        else
            closedir(pDir);
        //printf("lopath is %s \n",logpath);

        memset(temp,0,sizeof(temp));
        sprintf(temp,"%s%s",basepath,sempath);
        strcpy(sempath,temp);
        if( ( pDir = opendir(sempath) ) == NULL)
        {
            memset(temp, 0, sizeof(temp));
            sprintf(temp, "mkdir  -p %s", sempath);
            system(temp);
        }
        else
            closedir(pDir);
        //printf("sempath is %s \n",sempath);

        memset(temp,0,sizeof(temp));
        sprintf(temp,"%s%s",basepath,downloadpath);
         strcpy(downloadpath,temp);
        if( (pDir = opendir(downloadpath) ) == NULL)
        {
            memset(temp, 0, sizeof(temp));
            sprintf(temp, "mkdir  -p %s", downloadpath);
            system(temp);
        }
        else
            closedir(pDir);
        //printf("downloadpath is %s \n",downloadpath);

        memset(temp,0,sizeof(temp));
        //sprintf(temp,"%s/%s",basepath,ex_downloadpath);
         //strcpy(completepath,temp);
         strcpy(completepath,ex_downloadpath);
        /*if( (pDir = opendir(completepath) )== NULL)
        {
            memset(temp, 0, sizeof(temp));
            sprintf(temp, "mkdir  -p %s", completepath);
            system(temp);
        }
        else
            closedir(pDir);*/ //20130206 magic del
        //printf("completepath is %s \n",completepath);
}

int decode_url(char *url)
{

    //printf("start decode url \n");

    int len ;
    int i,k;
    char temp_url[512];

    memset( temp_url,0,sizeof(temp_url) );

    len = strlen(url);

    for( i = 0 , k= 0 ; i < len ; i++ ,k++)
    {
        if( url[i] == '&')
        {
            temp_url[k] = '\\';
            temp_url[k+1] = '&';
            k++;
        }
        else if( url[i] == '(')
        {
            temp_url[k] = '\\';
            temp_url[k+1] = '(';
            k++;
        }
       else if( url[i] == ')')
        {
            temp_url[k] = '\\';
            temp_url[k+1] = ')';
            k++;
        }
        temp_url[k] = url[i];
    }

    //int size = strlen(temp_url);
    temp_url[k+1] = '\0';

    //fprintf(stderr,"temp url is %s \n",temp_url);


    strcpy(url,temp_url);

}

int cmd_parser(char* str_cmd, int pid, struct Lognote *phead)
{
    //printf("socet command is %s \n",str_cmd);

    char cmd_name[64];
    char cmd_param[512];
    char *ch;
    struct Lognote *p1, *p2;

    memset(cmd_name, 0, sizeof(cmd_name));
    memset(cmd_param, 0, sizeof(cmd_param));

    ch = str_cmd;
    int i = 0;
    while(*ch != '@')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, str_cmd, i);
    //strcpy(cmd_name,str_cmd);

    //printf("cmd name is %s \n",cmd_name);

    if(strcmp(cmd_name, "add") == 0)
    {
        ch++; // pass @ ch
        i++;
        if(*ch == '0') // url
        {
            ch += 2;
            i += 2; // pass @ ch
            memset(cmd_param, 0, sizeof(cmd_param));
            strcpy(cmd_param, str_cmd+i);

            //printf("cmd_param is %s \n",cmd_param);

            int logid = addlognote(cmd_param, phead);
            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));

            //Sem_init(logpath,logid);
            struct Lognote* p1 = getlognote(logid, phead);
#if SEM
            sem_init(sempath,logid);
#endif

            // escape '&'
            //if( (strchr(p1->url,'&') != NULL) || (strchr(p1->url,'(') != NULL) ||(strchr(p1->url,')') != NULL)){
                //decode_url(p1->url);
		//}


            //fprintf(stderr,"\ndecode url is %s \n",p1->url);

            sprintf(tmp, "/opt/bin/dm2_snarf %d %s %s \"%s\" \"%s\" &", p1->id, logpath, downloadpath,completepath, p1->url);
	    //sprintf(tmp, "/opt/bin/dm2_snarf %d %s %s \"%s\" %s &", p1->id, logpath, downloadpath,completepath, p1->url);

            //fprintf(stderr,"\n snarf is %s \n",tmp);

            if (!fork()) {
                system(tmp);
                exit(0);
            }


            //print_log(phead);
        }
        else if(*ch == '1') // torrent path
        {
        }
        else if(*ch == '2') // torrent data
        {
        }
    }
    else if(strcmp(cmd_name, "pause") == 0)
    {

        ch++; // pass @ ch
        i++;
        memset(cmd_param, 0, sizeof(cmd_param));
        strcpy(cmd_param, str_cmd+i);
        int id = atoi(cmd_param);
        struct Lognote* p1 = getlognote(id, phead);
        if(p1 != NULL)
        {
            char log_name[200];
#if SEM
             char sem_name[256];
             Sem_t sem;


             memset(sem_name,0,sizeof(sem_name));
             sprintf(sem_name,"%s/sem.snarf_%d",sempath,id);
#endif

            Log_struc log_ts;
            memset(&log_ts, 0, sizeof(Log_struc));


            memset(log_name, 0, sizeof(log_name));

            sprintf(log_name, "%s/snarf_%d", logpath, id);

#if SEM

           if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                   printf("sem open error\n");
                   semUse = 0;
                   }

            if(semUse)
                Sem_wait(&sem);
#endif

            int fd1;
            if((fd1 = open(log_name, O_RDONLY | O_NONBLOCK)) < 0)
            {
                printf("\nread log error!\n");
                //return;
            }
            else
            {
                int len = read(fd1, &log_ts, sizeof(Log_struc));
                close(fd1);

                if(len > 0)
                {

                        int killpid =  atoi(log_ts.id );

                        if(log_ts.status == S_PROCESSING)
                        {
                                if(killpid > 1)
                                   kill(killpid,SIGKILL);

                                        int fd2;

                                        //if(semUse)
                                               //Sem_wait(&sem);

                                        if((fd2 = open(log_name, O_RDWR|O_CREAT|O_TRUNC, 0777)) < 0)
                                        {
                                            printf("\nread log error!\n");
                                        }
                                        else
                                        {
                                           //printf("\nwrite log !\n");
                                            log_ts.status = S_PAUSED;
                                            if( log_ts.download_type == HTTP )
                                               log_ts.ifile.iHttp.rate = 0 ;
                                            else
                                                log_ts.ifile.iFtp.rate = 0;
                                            int len1 = write(fd2, &log_ts, LOG_SIZE);

                                            close(fd2);
                                        }

                                       //if(semUse)
                                       //{
                                           //Sem_post(&sem);
                                           //Sem_close(&sem);
                                           //Sem_unlink(sem_name);
                                       //}
                                        p1->status = S_PAUSED;
                             }
                    }

            }
#if SEM
            if(semUse)
            {
                Sem_post(&sem);
                //Sem_close(&sem);
            }
#endif
        }
    }
    else if(strcmp(cmd_name, "all_paused") == 0) //add by gauss
    {

            struct dirent* ent = NULL;
            DIR *pDir;
            pDir=opendir(logpath);
            //int id[256];
            if(pDir != NULL )
            {
                //printf("obtain all ids\n");
                while (NULL != (ent=readdir(pDir)))

                {
                      char *p = NULL ;
                      p = strstr(ent->d_name,"snarf");
                          if(p)
                         {

                              int id;
                              id = atoi(ent->d_name+6);

                              struct Lognote* p1 = NULL;
                              p1 = getlognote(id, phead);

                              char log_name[256];
#if SEM
                              char sem_name[256];
                              Sem_t sem;

                              memset(sem_name, 0, sizeof(sem_name));
                              sprintf(sem_name, "%s/sem.snarf_%d", sempath, id);
#endif



                              //memset(tmp, 0, sizeof(tmp));
                              memset(log_name, 0, sizeof(log_name));

                              Log_struc log_ts;
                              memset(&log_ts, 0, sizeof(Log_struc));



                              sprintf(log_name, "%s/snarf_%d", logpath, id);

                              //strcpy(log_name,tmp);
#if SEM
                              if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                                      printf("sem fiel %s open error\n",sem_name);
                                      semUse = 0;
                                      }
                              else{
                                  //printf("sem fiel %s open ok\n",sem_name);
                              }

                              if(semUse)
                                  Sem_wait(&sem);
#endif

                                  int fd1;
                                  if((fd1 = open(log_name, O_RDONLY | O_NONBLOCK)) < 0)
                                  //if((fd1 = open(tmp, O_RDONLY)) < 0)
                                 {
                                     printf("\nread log error!\n");
                                 }
                                 else
                                  {
                                     int len = read(fd1, &log_ts, sizeof(Log_struc));
                                     close(fd1);

                                   if(len > 0)
                                     {

                                             if(log_ts.status == S_PROCESSING)
                                             {
                                                 int pid =  atoi(log_ts.id );

                                                 if(pid > 1)
                                                 {
                                                         kill(pid,SIGKILL);
                                                         //if(semUse)
                                                             //Sem_wait(&sem);
                                                                 int fd2;

                                                                 if((fd2 = open(log_name, O_RDWR|O_CREAT|O_TRUNC, 0777)) < 0)
                                                                 {
                                                                     printf("\nread log error!\n");
                                                                 }
                                                                 else
                                                                 {
                                                                     log_ts.status = S_PAUSED;

                                                                     if( log_ts.download_type == HTTP )
                                                                        log_ts.ifile.iHttp.rate = 0 ;
                                                                     else
                                                                         log_ts.ifile.iFtp.rate = 0;

                                                                      write(fd2, &log_ts, LOG_SIZE);
                                                                      close(fd2);
                                                                 }

                                                                 //if(semUse)
                                                                 //{
                                                                     //Sem_post(&sem);
                                                                     //Sem_close(&sem);
                                                                     //Sem_unlink(sem_name);
                                                                 //}

                                                         p1->status = S_PAUSED;


                                                   }
                                             }
                                         }
                               }
#if SEM
                             if(semUse)
                             {
                                 Sem_post(&sem);
                                 //Sem_close(&sem);
                             }
#endif
                         }

                }
                closedir(pDir);

            }
            else {
                printf("can't open log dir\n");
            }

    }
    else if(strcmp(cmd_name, "start") == 0)
    {
        ch++; // pass @ ch
        i++;
        memset(cmd_param, 0, sizeof(cmd_param));
        strcpy(cmd_param, str_cmd+i);
        int id = atoi(cmd_param);
        struct Lognote* p1 = getlognote(id, phead);

        char log_name[256];
#if SEM

        char sem_name[256];
        Sem_t sem;
        memset(sem_name,0,sizeof(sem_name));
        sprintf(sem_name,"%s/sem.snarf_%d",sempath,id);
#endif

        Log_struc log_ts;
        int fd1;

        memset(&log_ts, 0, sizeof(Log_struc));
        memset(log_name, 0, sizeof(log_name));


        sprintf(log_name, "%s/snarf_%d", logpath, id);

#if SEM
       if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
               printf("sem open error\n");
               semUse = 0;
               }

        if(semUse)
            Sem_wait(&sem);
#endif

        if((fd1 = open(log_name, O_RDONLY | O_NONBLOCK)) < 0)
        {
            printf("\nread log error!\n");
        }
        else
        {
            int len = read(fd1, &log_ts, sizeof(Log_struc));
            close(fd1);

            if(len > 0)
            {
                if(p1 != NULL &&  (log_ts.status == S_PAUSED || log_ts.status == S_DISKFULL ) )
                {
                    //printf("@@@@@@@@@ disk full is resume @@@@@@@@@@\n");

                    char tmp[1024];
                    memset(tmp, 0, sizeof(tmp));
                    sprintf(tmp, "./dm2_snarf %d %s %s \"%s\" \"%s\" &", p1->id, logpath, downloadpath,completepath, p1->url);

                    if (!(pid = fork())) {
                        system(tmp);
                        exit(0);
                    }
                    p1->status = S_PROCESSING;
                }
            }
        }
#if SEM
        if(semUse)
        {
            Sem_post(&sem);
        }
#endif

        //if()

    }
    else if(strcmp(cmd_name, "all_start") == 0) //add by gauss
    {
        struct dirent* ent = NULL;
        DIR *pDir;
        pDir=opendir(logpath);
        int id[256];

        if(pDir != NULL )
        {
            //printf("obtain all ids\n");
            while (NULL != (ent=readdir(pDir)))

            {
                char *p = NULL ;
                p = strstr(ent->d_name,"snarf");

                if(p)
                {
                    int id;
                    id = atoi(ent->d_name+6);

                    struct Lognote* p1 = NULL;
                    p1 = getlognote(id, phead);

                    char log_name[256];

#if SEM
                    char sem_name[256];
                    Sem_t sem;
                    memset(sem_name,0,sizeof(sem_name));
                    sprintf(sem_name,"%s/sem.snarf_%d",sempath,id);
#endif

                    Log_struc log_ts;
                    int fd1;

                    memset(&log_ts, 0, sizeof(Log_struc));
                    memset(log_name, 0, sizeof(log_name));


                    sprintf(log_name, "%s/snarf_%d", logpath, id);

#if SEM
                   if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                           printf("sem open error\n");
                           semUse = 0;
                           }

                    if(semUse)
                        Sem_wait(&sem);
#endif

                    if((fd1 = open(log_name, O_RDONLY | O_NONBLOCK)) < 0)
                    {
                        printf("\nread log error!\n");
                    }
                    else
                    {
                        int len = read(fd1, &log_ts, sizeof(Log_struc));
                        close(fd1);

                        if(len > 0)
                        {
                            if(p1 != NULL &&  (log_ts.status == S_PAUSED || log_ts.status == S_DISKFULL ) )
                            {
                                char tmp[1024];
                                memset(tmp, 0, sizeof(tmp));
                                sprintf(tmp, "./dm2_snarf %d %s %s \"%s\" \"%s\" &", p1->id, logpath, downloadpath,completepath, p1->url);

                                if (!(pid = fork())) {
                                    system(tmp);
                                    exit(0);
                                }
                                p1->status = S_PROCESSING;
                            }
                        }
                    }
#if SEM
                    if(semUse)
                    {
                        Sem_post(&sem);
                    }
#endif
                }
            }
            closedir(pDir);
        }

    }
    else if(strcmp(cmd_name, "clean_completed") == 0) //add by gauss
    {

        struct dirent* ent = NULL;
        DIR *pDir;
        pDir=opendir(logpath);
        //int id[256];

        if(pDir != NULL )
        {
            //printf("obtain all ids\n");
            while (NULL != (ent=readdir(pDir)))

            {

                      char *p = NULL ;
                      p = strstr(ent->d_name,"snarf");

                      if(p)
                      {
                          int id;
                          id = atoi(ent->d_name+6);

                          char log_name[200];
#if SEM
                          char sem_name[200];
                          Sem_t sem;
                          memset(sem_name, 0, sizeof(sem_name));
                          sprintf(sem_name, "%s/sem.snarf_%d", sempath, id);
#endif

                          memset(log_name, 0, sizeof(log_name));

                          Log_struc log_ts;
                          memset(&log_ts, 0, sizeof(Log_struc));


                          sprintf(log_name, "%s/snarf_%d", logpath, id);


                          struct Lognote* p1 = getlognote(id, phead);

#if SEM
                          if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                                  printf("sem open error\n");
                                  semUse = 0;
                                  }


                        if(semUse)
                            Sem_wait(&sem);
#endif

                          int fd1 = open(log_name, O_RDONLY | O_NONBLOCK);
                          if(fd1  < 0)
                         {
                             printf("\nread log error!\n");
                         }
                         else
                          {
                             int len = read(fd1, &log_ts, sizeof(Log_struc));
                             close(fd1);

                             if( len > 0)
                             {

                                     if(log_ts.status == S_COMPLETED)
                                     //if(p1->status == S_COMPLETED)
                                     {
                                         //unlink(log_name);

                                         if( remove(log_name) == -1)
                                         {
                                             perror("log file can't del \n");
                                             printf("%s cant not revmoe \n",log_name);
                                         }

                                         //unlink(sem_name);


                                     }
                               }
                         }

#if SEM
                         if(semUse)
                         {
                             if(log_ts.status == S_COMPLETED)
                                 remove(sem_name);
                             else
                             {
                                 Sem_post(&sem);
                                 Sem_close(&sem);
                             }
                         }
#endif
                     }
            }
            closedir(pDir);
        }

    }
    else if(strcmp(cmd_name, "cancel") == 0)
    {
        ch++; // pass @ ch
        i++;
        memset(cmd_param, 0, sizeof(cmd_param));
        strcpy(cmd_param, str_cmd+i);

        //printf("command is %s \n",cmd_param);

        int id = atoi(cmd_param);
        struct Lognote* p1 = getlognote(id, phead);
        if(p1 != NULL)
        {
            char log_name[256],sem_name[256],file_name[256];

#if SEM
            Sem_t sem;
            memset(sem_name, 0, sizeof(sem_name));
            sprintf(sem_name, "%s/sem.snarf_%d", sempath, id);
#endif

            // delete log and not completed file

            Log_struc log_ts;
            memset(&log_ts, 0, sizeof(Log_struc));

            memset(log_name, 0, sizeof(log_name));

            memset(file_name, 0, sizeof(file_name));

            sprintf(log_name, "%s/snarf_%d", logpath, id);

#if SEM
            if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                    printf("sem open error\n");
                    semUse = 0;
                    }


            if(semUse)
                Sem_wait(&sem);
#endif

            int fd1 = open(log_name, O_RDONLY | O_NONBLOCK);

            if(fd1 < 0)
            {
                printf("\nread log error!\n");
            }
            else
            {
                int len = read(fd1, &log_ts, sizeof(Log_struc));
                close(fd1);

                if(len > 0)
                {
                    //printf("status code is %d \n",log_ts.status);
                    if(log_ts.status == S_PROCESSING)
                    //if(p1->status != S_COMPLETED )
                    {
                       //if(log_ts.status != S_DEAD_OF_ERROR && log_ts.status != S_DISKFULL )
                        //{
                            int killpid = atoi(log_ts.id);
                            if(killpid > 1 )
                                kill(killpid,SIGKILL);
                        //}

                        sprintf(file_name, "%s/%s", downloadpath,log_ts.filename);
                        //unlink(file_name);
                        remove(file_name);
                        // rm not completed file
                    }

                    //unlink(log_name);
                    remove(log_name);

                    dellognote(id, phead);

                    //print_log(phead);
                }
        }
#if SEM
            if(semUse)
            {
                //Sem_post(&sem);
                //Sem_close(&sem);
                Sem_unlink(sem_name);
            }
#endif

            //print_log(phead);

            //printf("file status=%d\n", log_ts.status);
        }

    }
    else if(strcmp(cmd_name, "setpath") == 0)
    {
        ch++; // pass @ ch
        i++;
        memset(cmd_param, 0, sizeof(cmd_param));
    //    strcpy(cmd_param, str_cmd+i);
     //   printf("path: %s\n", cmd_param);
        memset(downloadpath, 0, sizeof(downloadpath));
        strcpy(downloadpath, str_cmd+i);
    }

    /*
    else if(strcmp(cmd_name, "send") == 0)
    {
        //printf("########enter send #########\n");
        memset(cmd_param, 0, sizeof(cmd_param));

        int logid,pid;
        ch++;
        i++;
        logid = atoi(str_cmd+i);

        //printf("logid is %d \n",logid);

        while(*ch != '@')
        {
            ch++;
            i++;
        }

        ch++;
        i++; // pass second '@'

        pid = atoi(str_cmd+i);

        //printf("pid is %d\n",pid);

        struct Lognote* p1 = getlognote(logid, phead);
        p1->pid = pid;

    }
    else if(strcmp(cmd_name, "status") == 0)
    {
        //printf("########enter status #########\n");
        memset(cmd_param, 0, sizeof(cmd_param));

        int logid;
        ch++;
        i++;
        logid = atoi(str_cmd+i);

        struct Lognote* p1 = getlognote(logid, phead);
        p1->status = S_COMPLETED;

    }
    */

    return 1;
}

void send_kill()
{

   //printf("recevie SIGINT \n");

   int have_snarf_progress = 0;

    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(logpath);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))

        {

                  char *p = NULL ;
                  p = strstr(ent->d_name,"snarf");

                  if(p)
                  {
                      //printf("p is %s\n",p);
                      have_snarf_progress = 1;
                      int id;
                      id = atoi(ent->d_name+6);

                      char log_name[200],sem_name[200];

#if SEM
                      Sem_t sem;
                      memset(sem_name, 0, sizeof(sem_name));
                      sprintf(sem_name, "%s/sem.snarf_%d", sempath, id);
#endif

                      memset(log_name, 0, sizeof(log_name));


                      Log_struc log_ts;
                      memset(&log_ts, 0, sizeof(Log_struc));


                      sprintf(log_name, "%s/snarf_%d", logpath, id);


                      //struct Lognote* p1 = getlognote(id, phead);
#if SEM

                      if(Sem_open(&sem, sem_name, O_RDONLY) == -1){
                              printf("sem open error\n");
                              semUse = 0;
                              }

                    if(semUse)
                        Sem_wait(&sem);
#endif

                      int fd1 = open(log_name, O_RDONLY | O_NONBLOCK);
                      if(fd1  < 0)
                     {
                         printf("\nread log error!\n");
                     }
                     else
                      {
                         int len = read(fd1, &log_ts, sizeof(Log_struc));
                         close(fd1);

                         if( len > 0 && log_ts.status == S_PROCESSING)
                         {
                             int pid = atoi(log_ts.id);
                             if(pid > 1)
                               kill(pid,SIGUSR1);
                          }
                     }

#if SEM
                     if(semUse)
                     {
                             Sem_post(&sem);
                             Sem_close(&sem);
                     }
#endif
                 }
        }
        closedir(pDir);
    }

    //printf("have_snarf_progress is %d \n",have_snarf_progress);
    //if(have_snarf_progress != 1)
    //{
        freelognote();
        exit(0);
    //}
}

int write_debug(char *msg)
{
   FILE *fp;
   char *log_name = "/tmp/snarf_log";
   if(access(log_name,0)== 0)
    fp = fopen(log_name,"a");
   else
    fp = fopen(log_name,"w");

   fprintf(fp,"%s",msg);
   fclose(fp);
}

int CheckMountPathExist()
{
    char msg[256]={0};
    FILE *fp;
    char mount_path[1024] = {0};
    fp = fopen("/tmp/asus_router.conf","r");
    if(fp)
    {
        char generalbuf[128];
        while(!feof(fp))
        {
            fscanf(fp,"%[^\n]%*c",generalbuf);
            if(strncmp(generalbuf,"BASE_PATH=",9) == 0)
            {
                strncpy(mount_path,generalbuf+10,1024);
                //sprintf(msg,"mount_path=%s\n",mount_path);
                if(access(mount_path,0) == 0)
                {
                    fclose(fp);  
                    return 1;
                 }
            }
        }
     fclose(fp);
    }
   

    return 0;
}

main()
{
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd*/
    int numbytes;
    char buf[MAXDATASIZE];

    struct Lognote *p;
    int pid;
    int yes = 1;

    //signal(SIGTERM, freelognote);
    //signal(SIGQUIT, freelognote);
    //signal(SIGALRM, freelognote);

    memset(basepath,0,sizeof(basepath));

    if(!CheckMountPathExist())
    {
        printf("mount path is not exist\n");
        return -1;
    }

    signal(SIGUSR1,send_kill);
    signal(SIGCHLD,SIG_IGN);

    //add by gauss
    InitSnarfCfg(CONFIG_ROUTER_PATH);
    //printf("basepath is %s\n",basepath);
    CompareCfg(CONFIG_PATH,CONFIG_NEW_PATH);
    //printf("basepath is %s\n",basepath);

    //InitSnarfCfg(CONFIG_PATH);

    //printf("basepath is %s")

    head = (struct Lognote *)malloc(sizeof(struct Lognote));

    memset(head, 0, sizeof(struct Lognote));

    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    int sin_size;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(MYPORT); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), sizeof(my_addr.sin_zero)); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct
                                                         sockaddr))== -1) {
        perror("bind");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    initlognote(logpath, head);
   // print_log(head);
    initsnarf(head);

    while(1) { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                             &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        //printf("server: got connection from %s\n", \
               //inet_ntoa(their_addr.sin_addr));

        memset(buf, 0, sizeof(buf));

        if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        cmd_parser(buf, pid, head);


        memset(buf, 0, sizeof(buf));
        strcpy(buf, "rev cmd");
        if (send(new_fd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
        }

        close(new_fd);
        //while(waitpid(-1,NULL,WNOHANG) > 0);
    }

    // free resource


}


