
//2011.04.26 magic added{
#include "dm_hook.h"
#include "dm_url_parser.h"
#include "dm_http_parser.h"
#include "dm_ftp_parser.h"

char *getrouterconfig()
{
    if (access("/tmp/asus_router.conf",0) == 0)
    {
        int fd, len, i=0;
        char ch, tmp[256], name[256], content[256];
        char *result = NULL;
        result = (char *)malloc(64);
        if(NULL == result)
            return NULL;
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        memset(result, 0, sizeof(result));

        if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
            return NULL;
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
        return NULL;

}

void getdmconfig()
{
    if (access("/tmp/APPS/DM2/Config/dm2_general.conf",0) == 0)
    {
        int fd, len, i=0;
        char ch, tmp[256], name[256], content[256] ,result[64];
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        memset(result, 0, sizeof(result));
        sprintf(result, "1");

        if((fd = open("/tmp/APPS/DM2/Config/dm2_general.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
            return ;
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
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "MAX_ON_HEAVY"))
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

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            return ;
        }
    }
    else
        return ;

}

void init_path()
{
    memset(Base_dir, '\0', sizeof(Base_dir));
    memset(Share_dir, '\0', sizeof(Share_dir));
    memset(In_dir, '\0', sizeof(In_dir));
    memset(Log_dir, '\0', sizeof(Log_dir));
    memset(Sem_dir, '\0', sizeof(Sem_dir));
    memset(jqs_dir, '\0', sizeof(jqs_dir));
    memset(jqs_file, '\0', sizeof(jqs_file));

	char *routerconfig = getrouterconfig();
	if(routerconfig != NULL)
	{
        sprintf(Base_dir, "%s", routerconfig);
        free(routerconfig);
	}
    sprintf(Base_dir, "%s", getrouterconfig());
    sprintf(Share_dir, "%s/Download2", Base_dir);
    sprintf(In_dir, "%s/InComplete", Share_dir);
    sprintf(Log_dir, "%s/.logs", Share_dir);
    sprintf(Sem_dir, "%s/.sems", Share_dir);
    sprintf(jqs_dir, "%s/.sems/sem.jqs", Share_dir);
    sprintf(jqs_file, "%s/dm.jqs", Log_dir);
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

    return ((status == S_INITIAL)||(status == S_PROCESSING)||(status == S_PAUSED)||(status == S_COMPLETED)||(status == S_DISKFULL)||(status == S_NEEDACCOUNT)||(status == S_CONNECTFAIL)||(status == S_SSLFAIL)||(status == S_ACCOUNTFAIL)||(status == S_RECVFAIL) ||(status == S_HASH)) ? (1) : (0);
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
    if(Sem_open(&sem, semname, flags) == -1)
    {
        use_sem = 0;
    }

    if(use_sem)
        Sem_wait(&sem);

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
    sprintf(torrent_pos, "%s/%s", In_dir, torrent_name);
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
    }
}

void check_alive()
{	// check on_heavy_jobs and on_light_jobs
    DIR *log_dir;
    struct dirent *log_ptr;
    Log_struc slog;
    //./int pid;
    char pid[MAX_HASHLEN]={0};

    if((log_dir = opendir(Log_dir)) == NULL){
        return;
    }

    on_heavy_counts = 0;
    on_light_counts = 0;
    on_nntp_counts = 0;			//Allen NNTP
    on_ed2k_counts = 0;

    while((log_ptr = readdir(log_dir)) != NULL)
    {
        //./if((pid = getlogid(log_ptr->d_name)) > 0)
        sprintf(pid, "%s" , getlogid(log_ptr->d_name));

        if((strcmp(pid,"none"))&&(strncmp(pid,"-",1)!=0))
        {
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0)
            {
                if(isOnProc(slog.status))
                {
                    //if(slog.download_type == BT)	Allen 20090722
                    if((slog.download_type == BT) && slog.progress < 1 )
                        on_heavy_counts++;
                    else if(((slog.download_type == HTTP)||(slog.download_type == FTP)) && slog.progress < 1)
                        on_light_counts++;
                    else if((slog.download_type == NZB) && slog.progress < 1 )		//Allen NNTP
                        on_nntp_counts++;
                    else if((slog.download_type == ED2K) && slog.progress < 1 )		//Allen NNTP
                        on_ed2k_counts++;
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
    char pid[MAX_HASHLEN]={0};

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
                        if((!download_name) || (!slog.filename) || (!infohash) || (!slog.store_dst))
                        {
                            continue;
                        }
                        if(strncmp(download_name, slog.filename, strlen(download_name)) == 0){
                            // test
                            closedir(log_dir);
                            return 1;
                        }

                        if(strncmp(infohash, slog.store_dst, strlen(infohash)) == 0){
                            // test
                            remove_torrent(download_name);	//Allen 20090826
                            closedir(log_dir);
                            return 1;
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

/* create a notet*/
//struct Lognote * createnote(int id, char *url, int status)
struct Lognote * createnote(int id, char *infohash,char *url, char *real_url, char *filenums, char *d_type, int status ,int times)
{
    struct Lognote *newnote = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(newnote, 0x00, sizeof(struct Lognote *));
    newnote->id = id;
    strcpy(newnote->url, url);
    strcpy(newnote->real_url, real_url);
    strcpy(newnote->filenum, filenums);
    strcpy(newnote->infohash, infohash);
    strcpy(newnote->type, d_type);
    newnote->status = status;
    newnote->checknum = times;
    return newnote;
}

/* create and add a note to list*/
int addlognote(char *url, char *infohash, char *real_url, char *filenums, struct Lognote *phead ,char *d_type)
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

    if(!strncmp(url,"magnet:",7)){
        strcpy(real_url, url);
    }

    struct Lognote *note = createnote(i, infohash ,url, real_url,filenums, d_type, S_NOTBEGIN ,1);

    p2->next = note;
    note->next = p1;

    //refresh_jqs(p2);
    refresh_jqs();
    //print_log(head);

    return note->id;
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
    refresh_jqs();
}


int check_url(UrlResource *rsrc ,int times)
{

    Url *u = NULL;
    u   = rsrc->url;
    int parser_status;
    switch (u->service_type) {
    case SERVICE_HTTP:
        parser_status = http_transfer(rsrc,times);

        if( parser_status == P_PASS)
            return 1;
        else
        {
            if( strstr(u->full_url, "http://") ) {
                u->full_url += 7;
            }

            u->full_url = strconcat("ftp://",u->full_url,NULL);

            Url *new_url = NULL;
            new_url = url_new();
            url_init(new_url,u->full_url);

            rsrc->url = new_url;


            parser_status = ftp_transfer(rsrc,times);

            if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
                return 1;
            else
                return -1;
        }
        break;
    case SERVICE_FTP:
        parser_status = ftp_transfer(rsrc,times);

        if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
            return 1;
        else
        {
            if( strstr(u->full_url, "ftp://") ) {
                u->full_url += 6;
            }

            u->full_url = strconcat("http://",u->full_url,NULL);

            Url *new_url = NULL;
            new_url = url_new();
            url_init(new_url,u->full_url);

            rsrc->url = new_url;

            parser_status = http_transfer(rsrc,times);

            if( parser_status == P_PASS )
                return 1;
            else
                return -1;
        }
        break;
    case SERVICE_BT:
	u->status = P_PASS;
        return 1;
        //break;
    case SERVICE_NZB:
	u->status = P_PASS;
        return 1;
    case SERVICE_ED2K:
	u->status = P_PASS;
        return 1;
    default:
        break;
    }
}

Url  *url_parser(char *url ,int times)
{
    Url *src_url = NULL;
    Url *new_url = NULL;
    UrlResource *rsrc = NULL;

    rsrc = url_resource_new();
    src_url = url_new();
    new_url = url_new();
    url_init(src_url,url);

    rsrc->url = src_url;

    check_url(rsrc ,times);

    new_url = rsrc->url;

    return new_url;

}

void updatelognote_byurl(char *url, char *real_url, struct Lognote *phead,char *D_type,int status,int times)
{
    struct Lognote *p1, *p2;
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
            //p2->next = p1->next;
            sprintf(p1->real_url,"%s",real_url);
            sprintf(p1->type,"%s",D_type);
            //sprintf(p1->status,"%d",status);
            p1->status = status;
            p1->checknum = times;
            //free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    refresh_jqs(head);
}

void dellognote_byurl(char *url, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
            p2->next = p1->next;
            free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    refresh_jqs(head);
}


void initlognote3()
{
    //print_log(head);
    int fd_r;
    struct Lognote au;
    jqs_sem_use = 1;
    if(Sem_open(&jqs_sem, jqs_dir, 0) < 0)
    {
        jqs_sem_use = 0;
    }

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);

    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {
        return;
    }

    memset(&au, 0x00, sizeof(struct Lognote));
    //check_alive();
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {
        if(strcmp(au.url,"")){
            if(au.status==S_PARSER_FAIL)
            {
                Url *new_url = NULL;
                new_url = url_new();
                new_url = url_parser(au.url, au.checknum);
                char D_type[10];
                memset(D_type,'\0',sizeof(D_type));
                if(new_url->status == P_PASS )
                {
                    switch (new_url->service_type) {
                    case SERVICE_HTTP:
                        sprintf(D_type,"1");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NOTBEGIN,au.checknum);
                        break;
                    case SERVICE_FTP:
                        sprintf(D_type,"2");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NOTBEGIN,au.checknum);
                        break;
                    case SERVICE_BT:
                        sprintf(D_type,"3");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NOTBEGIN,au.checknum);
                        break;
                    case SERVICE_NZB:
                        sprintf(D_type,"4");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NOTBEGIN,au.checknum);
                        break;
                    case SERVICE_ED2K:
                        sprintf(D_type,"6");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NOTBEGIN,au.checknum);
                        break;
                    default:
                        sprintf(D_type,"5");
                        updatelognote_byurl(au.url,new_url->full_url,head,D_type,au.status,au.checknum);
                        break;

                    }
                }
                else if(new_url->status == P_NEEDACCOUNT)
                {
                    sprintf(D_type,"2");
                    updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_NEEDACCOUNT,au.checknum);
                    break;
                }
                else{
                    if(au.checknum<3){
                        au.checknum++;
                    }
                    sprintf(D_type,"5");
                    updatelognote_byurl(au.url,new_url->full_url,head,D_type,S_PARSER_FAIL,au.checknum);
                }

            }
        }
        memset(&au, 0x00, sizeof(struct Lognote));

    }
    close(fd_r);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);
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
    if(Sem_open(&jqs_sem, jqs_dir, 0) < 0)
    {
        jqs_sem_use = 0;
    }

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);

    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {

        return;
    }

    memset(&au, 0x00, sizeof(struct Lognote));
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {

        struct Lognote *note = createnote(au.id, au.infohash, au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
        insertnote(note, phead);
        memset(&au, 0x00, sizeof(struct Lognote));

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

static void safe_leave(int signo){
    freelognote();
    exit(0);
}

int main (int argc, char **argv) {
    umask(0);
    setsid();

    chdir("/");

    close(STDIN_FILENO);

    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, safe_leave);
    signal(SIGUSR2, safe_leave);

    init_path();
    getdmconfig(); //20120821 magic modify

    head = (struct Lognote *)malloc(sizeof(struct Lognote));
    on_heavy_counts = on_light_counts = total_heavy_counts = total_light_counts = 0;
    on_nntp_counts = total_nntp_counts = 0;

    time(&now);
    memset(head, 0x00, sizeof(struct Lognote));
    load_checkurl_timestamp = now;
    initlognote(head); //2011.05.26 magic added
    initlognote3();
    system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected");

    freelognote();
    sync();
    return 0;

}
