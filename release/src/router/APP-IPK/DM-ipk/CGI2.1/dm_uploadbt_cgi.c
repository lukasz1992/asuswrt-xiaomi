#include "dm_uploadbt_cgi.h"
#include "dm_hook.h"
#include "dm_func.h"
#include "dm_btdecode.h"
typedef enum{
    // UP_SHOW,
    UP_OK,
    UP_FAIL,
    UP_NONE,
}UP_STATUS;

#include <ctype.h>
//inline char* strlwr( char* str )
char* strlwr( char* str ) //2016.8.19 tina modify
{
char* orig = str;
// process the string
for (;*str!='\0';str++ )
*str=tolower(*str);
return orig;
}

extern Filess *filess_head;
char *ffname;
char *changefilename(char* filename);
char infohash_u[MAX_NAMELEN];//leo added
char torrent_pos_u[MAX_NAMELEN];// leo added
int end_bt(char *filename)
{
    char  *ptr;

    /*if(((ptr=strstr(filename, ".torrent"))!=NULL) && (*(ptr+8)=='\0')){
//fprintf(stderr,"\nptr=%s\n",ptr);
        return 1;
    } else{
//fprintf(stderr,"\nptr1=%s\n",ptr);
        return 0;
    }*/
    char bt_type[10] ;
    memset(bt_type,0,sizeof(bt_type));
    strcpy(bt_type,filename+strlen(filename)-8);
    strlwr(bt_type);
    //fprintf(stderr,"\nbt_type=%s\n",bt_type);
    if(strcmp(bt_type,".torrent") == 0) {
        return 1;
    }
    else{
        return 0;
    }
}

int end_nzb(char *filename)
{
    char  *ptr;

    /*if(((ptr=strstr(filename, ".nzb"))!=NULL) && (*(ptr+4)=='\0')){
        return 1;
    } else{
        return 0;
    }*/
    char nzb_type[10] ;
    memset(nzb_type,0,sizeof(nzb_type));
    strcpy(nzb_type,filename+strlen(filename)-4);
    strlwr(nzb_type);
    //fprintf(stderr,"\nnzb_type=%s\n",nzb_type);
    if(strcmp(nzb_type,".nzb") == 0) {
        return 1;
    }
    else{
        return 0;
    }
}


char *lowercase(char* hash)
{
    int i=0;
    while(hash[i]!='\0')
    {
        if(hash[i]>=65&&hash[i]<=90)
        {
            hash[i]+=32;

        }
        i++;
    }
    return hash;
}

int DM_ADD(char* cmd, char* d_type,char* fname,struct Lognote *qhead)
{
    int sfd;
    int result = UNKNOW;
    char command[2048];
    struct sockaddr_in btaddr;


    memset(command,'\0',sizeof(command));
    sprintf(command,"%s",cmd);

    check_alive();
    if(total_counts + on_heavy_counts + on_light_counts + on_nntp_counts + on_ed2k_counts< MAX_QUEUES){
        memset(&btaddr, '\0', sizeof(btaddr));
        btaddr.sin_family = AF_INET;
        result =  ACK_SUCESS;
        if(!strcmp(d_type, "3")){

            //if(on_heavy_counts < MAX_ON_HEAVY){ //20121204 magic new rules
            if(on_heavy_counts + on_ed2k_counts< MAX_ON_HEAVY){
                //sprintf(torrent_pos, "%s/Download2/InComplete/%s", getbasepath(),fname);
                //sprintf(torrent_pos, "%s/Download2/Seeds/%s", getbasepath(),fname);
               // sprintf(torrent_pos, "%s/Download2/Seeds/%s", Base_dir,fname);
               // memset(infohash, '\0', sizeof(infohash));
               // get_file_infohash(torrent_pos, infohash);
               // lowercase(infohash);

                memset(filename,'\0',sizeof(filename));
                get_file_name(torrent_pos_u, filename);

                int64_t total_len=0;
                get_file_length(torrent_pos_u, &total_len);
                //fprintf(stderr,"\nmulti_file=%d\n",multi_file);

                if(multi_file == 1)
                {
                    get_files_length_path(torrent_pos_u);
                }
                if(chk_on_process(fname, infohash_u) > 0){
                    return BT_EXIST;
                }
                else if(checklognote(fname,qhead)>0){
                    return BT_EXIST;
                }
                else if(on_hash_counts>0)
                {
                    return NOW_HASHING;//2012.06.27 eric added
                }
                else{
                    //int64_t max_len=0;
                    int64_t total_len=0;
                    get_file_length(torrent_pos_u, &total_len);
                    //fprintf(stderr,"check_disk_space: %lld - total_len:%lld = %lld \n",check_disk_space(Base_dir),total_len,check_disk_space(Base_dir)-total_len);

                    if(total_len >= (check_disk_space(Base_dir) - (int64_t)50*1024*1024)){	//rtorrent will close when less then 50Mb
                        //gdc_err_job("File size is larger then the space left on the partition", pid_rtn, torrent);
                        return DISK_FULL;
                    }
                    else{
                        //if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
                        FILE *cgi_fp;
                        cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
                        fclose(cgi_fp);
                        return BT_STOP;
                        //}else{
                        //	btaddr.sin_port = htons(BT_PORT);
                        //}
                    }
                    
                }
            }
            else{
                return HEAVY_FULL;
               }
        }
        else if(!strcmp(d_type, "4")){
            if(on_nntp_counts < MAX_ON_NNTP){
                if(chk_on_process(fname, NULL) > 0){
                    return BT_EXIST;
                }
                else if(checklognote(fname,qhead)>0){
                    return BT_EXIST;
                }
                else{
                    //if (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0){
                    FILE *cgi_fp;
                    cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
                    fclose(cgi_fp);
                    return NZB_STOP;
                    //}else{
                    //	btaddr.sin_port = htons(NZB_PORT);
                    //}
                }
            }
            else{
                return NNTP_FULL;
            }
        }
        inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);

        if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            result = ACK_FAIL;
        }

        if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
            result = ACK_FAIL;
        }

        if(write(sfd, command, strlen(command)) != strlen(command))
        {
            result = ACK_FAIL;
        }

        close(sfd);

        Filess *p;
        while(filess_head != NULL)
        {
            p = filess_head;
            filess_head = filess_head->next;
            free(p);
        }

        sleep(1);
        return result;
    }
    else{
        return TOTAL_FULL;
    }
}


void print_log(struct Lognote *phead)
{
    struct Lognote *au = (struct Lognote*)0;
    for(au=phead; au; au=au->next){
        fprintf(stderr, "\nprint_log\n");
        fprintf(stderr,"id = %d\n", au->id);
        fprintf(stderr,"url = %s\n", au->url);
fprintf(stderr,"hash = %s\n", au->infohash);
        fprintf(stderr,"type = %s\n", au->type);
        fprintf(stderr,"filenum = %s\n", au->filenum);
    }
}

/* create a notet*/
//struct Lognote * createnote(int id, char *url, int status)
struct Lognote * createnote(int id, char *infohash,char *url, char *real_url,char *filenums,char *d_type, int status ,int times)
{
    struct Lognote *note = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(note, 0, sizeof(struct Lognote *));
    note->id = id;

    strcpy(note->url, url);
    strcpy(note->real_url, real_url);
    strcpy(note->filenum, filenums);
    strcpy(note->type, d_type);
    strcpy(note->infohash, infohash);
    note->status = status;
    note->checknum = times;

    return note;
}

/* insert a note to list*/
void insertnote(struct Lognote *note, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
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
    total_counts=0;
    //fprintf(stderr,"jqs_dir=%s \n",jqs_dir);
    int fd_r;
    struct Lognote au;
    jqs_sem_use = 1;
    if(Sem_open(&jqs_sem, jqs_dir, 0) < 0)
    {
        jqs_sem_use = 0;
    }

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);
   // fprintf(stderr,"@@@@@@@@@ intilognote jqs_file=%s \n",jqs_file);
    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {
        //fprintf(stderr," initlognote open jqs_file error \n");
        return;
    }

    memset(&au, 0, sizeof(struct Lognote));
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {
        //fprintf(stderr,"initlog \n");
        struct Lognote *note = createnote(au.id,au.infohash,au.url,au.real_url,au.filenum, au.type, au.status ,au.checknum);
        insertnote(note, phead);
        if(strcmp(note->url,"")){
            total_counts++;
        }
        memset(&au, 0x00, sizeof(struct Lognote));
    }
    close(fd_r);

    //print_log(head);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);
}

void refresh_jqs(struct Lognote *phead){
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

    for(pau = phead; pau; pau = pau->next){

        if(strcmp(pau->url,"")){
                /*if(strcmp(pau->type,"4"))
		{
		    strcpy(pau->infohash, infohash_u);
                }*/
            write(fd, pau, sizeof(struct Lognote));
             //write(fd, pau, sizeof(infohash_u ));

        }
        else{
             //fprintf(stderr,"not write\n");
        }
    }
    close(fd);
    if(jqs_sem_use)
        Sem_post(&jqs_sem);

}

/* create and add a note to list*/
int addlognote(char *url,char *infohash,char *real_url, char *filenums, struct Lognote *phead ,char *d_type)
{
    //print_log(phead);
    int i = 1;
    int j = 1;
    //    int isInsert = 0;
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
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
    struct Lognote *note = createnote(i,infohash, url, real_url,filenums, d_type, S_NOTBEGIN, 1);

    p2->next = note;
    note->next = p1;

    //fprintf(stderr, "note->url=%s\n",note->url);
    //(stderr, "note->type=%s\n",note->type);

    //head -> next = (struct Lognote*)0;
    //memcpy(&head,&note,sizeof(note));
    system("killall dm2_detect&");//neal add
    system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected"); //neal add
    refresh_jqs(head);



    return note->id;
}

int checklognote(char *url, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;


    int result=0;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
            result=1;
            break;
        }
        if(strcmp(infohash_u,"")!=0){
                if(strncmp(infohash_u, p1->infohash, strlen(infohash_u)) == 0){

                    remove_torrent(url, BT_EXIST);	//Allen 20090826
                    //closedir(log_dir);
                    return 1;
                }
        }
        p2 = p1;
        p1 = p1->next;

    }

    return result;
}

/* add -p by gauss */
void dellognote(int id, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
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
    system("killall dm2_detect&");//neal add
    system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected"); //neal add
    refresh_jqs(head);
}

void updatelognote_byurl(char *url, struct Lognote *phead, char *D_type, int status)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
            //p2->next = p1->next;
            sprintf(p1->type,"%s",D_type);
            p1->status = status;
            //free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    system("killall dm2_detect&");//neal add
    system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected"); //neal add
    refresh_jqs(head);
}

char *changefilename(char* filename)
{
    while(strrchr(filename,'\\') != NULL)
    {
        filename = strrchr(filename,'\\')+1;
    }
    //fprintf(stderr,"tmp=%s\n",filename);
    return filename;

}

#define debug(x...) fprintf(stderr,x)
int main(int argc,char* argv[]){
    init_path();
    //struct Lognote *p;
    head = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(head, 0, sizeof(struct Lognote));
    initlognote(head);

    printf("ContentType:text/html\r\n");
    printf("Cache-Control:private,max-age=0;\r\n");
    printf("\r\n");

    char *datajs;

    datajs = getenv("QUERY_STRING");
    init_cgi(datajs);	// there wasn't '?' in the head.

    char *filenums;
    filenums = websGetVar(datajs, "download_type", "");
    //char *ffname;    // leo changed to whole
    ffname = websGetVar(datajs, "filename", "");
    char *D_type_tmp;
    D_type_tmp = websGetVar(datajs, "D_type", "");

    UP_STATUS up_s;
    up_s=UP_OK;
    char *p;
    int ctrl_result;
    char D_type[10],cmd[512], fname[1024], updir[256];
    memset(fname,'\0',sizeof(fname));
    memset(D_type, '\0', sizeof(D_type));
    memset(cmd, '\0', sizeof(cmd));
    memset(updir, '\0', sizeof(updir));

    sprintf(updir, "%s/Download2/Seeds/", Base_dir);

    if(getenv("CONTENT_TYPE")){
        if(!strncmp(getenv("CONTENT_TYPE"),"multipart/form-data",19)){
            char boundary[64]="--";
            p=strstr(getenv("CONTENT_TYPE"),"boundary=");
            strcpy(boundary+2,p+9);
            int len=atoi(getenv("CONTENT_LENGTH"));
            //read down to last file field
            char str[512];

            int pos=0;
            while(1){
                if(fgets(str,512,stdin)){
                    pos+=strlen(str);
                    if(strstr(str,"filename=")){
                        //get filename
                        p=strstr(str,"filename=");
                        strcpy(fname,p+9);
                        if(!strcmp(fname,"\"\"\r\n")){
                            up_s=UP_NONE;
                            goto done;
                        }
                        int len = strlen(fname);

                        char temp_fname[1024];
                        memset(temp_fname, '\0', sizeof(temp_fname));
                        strncpy(temp_fname,fname+1,len-4);
                        sprintf(fname,"%s",temp_fname);
                        *p='\0';

                        break;
                    }
                }
            }

            if(end_bt(fname)){
                sprintf(D_type,"3");

            }
            else if(end_nzb(fname)){
                sprintf(D_type,"4");
            }
            else{
                sprintf(D_type,"5");
            }

            fgets(str,512,stdin);
            pos+=strlen(str);
            fgets(str,512,stdin);
            pos+=strlen(str);

            int fSize=len-pos-strlen(boundary)-6;
            //int bSize=4096;//delete by eric
            //open file for writing
            char fpath[256]="";
            strcat(fpath,updir);

            strcat(fpath,changefilename(fname));
            FILE *f=fopen(fpath,"wb");
            if(!f){debug("open file failure");goto fail;}
            //chunk read
            //int chunk=fSize/bSize;////delete by eric
            //int lbSize=fSize%bSize;
            char *buf=malloc(fSize);
            memset(buf, 0, sizeof(fSize));
            int i = 0;
            if(fread(buf,fSize,1,stdin))
            {
                if(*buf == 13){buf++;i++;}
                if(*buf == 10){buf++;i++;}
                //fprintf(stderr,"\nbuf=%s\n",buf);
                if(!fwrite(buf,fSize-i,1,f))
                    goto fail;
            }
            else
            {
                goto fail;
            }
            buf = buf - i;
            free(buf);
            if(fflush(f)){debug("error flush file");goto fail;}
            fclose(f);
            up_s=UP_OK;
            goto done;
        }
    }
    goto done;
fail:
    up_s=UP_FAIL;
done:
    printf("<script>");
    //int checktime = check_download_time();
    //20120821 magic modify{
    //getbasepath();
    //2016.8.22 tina modify{
    //getdmconfig();
    char *dmconfig2 = getdmconfig();
    if(dmconfig2 != NULL)
        free(dmconfig2);
    //}end tina

    // 20160505 leo added for optimization code  {

    if(strcmp(fname,"")!=0)
    {

        memset(infohash_u,'\0',sizeof(infohash_u));
        memset(torrent_pos_u,'\0',sizeof(torrent_pos_u));
        sprintf(torrent_pos_u, "%s/Download2/Seeds/%s", Base_dir,changefilename(fname));
        get_file_infohash(torrent_pos_u,infohash_u);
        lowercase(infohash_u);
    }
    else
    {
        memset(infohash_u,'\0',sizeof(infohash_u));
        memset(torrent_pos_u,'\0',sizeof(torrent_pos_u));
        sprintf(torrent_pos_u, "%s/Download2/Seeds/%s", Base_dir,changefilename(ffname));
        get_file_infohash(torrent_pos_u,infohash_u);
        lowercase(infohash_u);
    }


    // 20160505 leo added for optimization code  }
    //20120821 magic modify}
    int checktime=timecheck_item(nv_data,nv_time1,nv_time2); //2012.07.10 magic added for cross-night
    if(up_s==UP_OK){
        if(checktime==1){

            if(*filenums && filenums)
            {
                Filess *p;
                while(filess_head != NULL)
                {
                    p = filess_head;
                    filess_head = filess_head->next;
                    free(p);
                }
                //sleep(1);

                //20151113 leo added for utility  {


                if (chk_on_process(changefilename(ffname),infohash_u)>0)
                {

                    ctrl_result=BT_EXIST;
                }
                else  if(checklognote(changefilename(ffname),head)>0)
                {
                    ctrl_result=BT_EXIST;
                }

               //memset(infohash_u,'\0',sizeof(infohash_u));
                //20151113 leo added for utility  }

               if (ctrl_result!=BT_EXIST)
                {
    addlognote(changefilename(ffname),infohash_u,changefilename(ffname),filenums,head,"3");
                    printf("parent.response_dm_add(\"ACK_SUCESS\");</script>");
                    fflush(stdout);
                    return 1;
                }

                //printf("successful</script>");

                //ctrl_result = ACK_SUCESS;
            }
            else
            {
                sprintf(cmd,"%s%s%s","add@1@",updir,changefilename(fname));

                ctrl_result = DM_ADD(cmd,D_type,changefilename(fname),head);

            }
            switch(ctrl_result)
            {
            case ACK_SUCESS:
                //fprintf(stderr,"\nBT_AUCESS\n");
                if(multi_file == 1)
                {
                    Filess *pp;
                    pp = filess_head;
                    printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, ",filename,fname);
                    if(pp->length < 1024){
                        printf("#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                    }
                    else{
                        printf("#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                    }
                    while(pp)
                    {
                        pp = pp->next;
                        if(pp)
                        {
                            if(pp->length < 1024){
                                printf(", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                            }
                            else{
                                printf(", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                            }
                        }
                    }
                    printf("\");");
                    Filess *p;
                    while(filess_head != NULL)
                    {
                        p = filess_head;
                        filess_head = filess_head->next;
                        free(p);
                    }
                }
                else
                {
                    printf("parent.response_dm_add(\"ACK_SUCESS\");");
                }
                break;
            case ACK_FAIL:
				printf("parent.hideLoading();");
                printf("parent.response_dm_add(\"ACK_FAIL\");");
                break;
            case BT_EXIST:
				printf("parent.hideLoading();");
                printf("parent.response_dm_add(\"BT_EXIST\");");
                break;
            case NOW_HASHING://2012.06.27 eric added
            case HEAVY_FULL:
                memset(filename,'\0',sizeof(filename));
                get_file_name(torrent_pos_u, filename);
                //int64_t max_len=0;
                int64_t total_len=0;
                get_file_length(torrent_pos_u,&total_len);
                if(multi_file == 1)
                {
                     get_files_length_path(torrent_pos_u);
                }

                if(total_len >= (check_disk_space(Base_dir) - (int64_t)50*1024*1024)){	//rtorrent will close when less then 50Mb
					printf("parent.hideLoading();");
                    printf("parent.response_dm_add(\"DISK_FULL\");");
                }
                else if(chk_on_process(changefilename(fname), infohash_u) > 0){
					printf("parent.hideLoading();");
                    printf("parent.response_dm_add(\"BT_EXIST\");");
                }
                else if(checklognote(changefilename(fname),head)>0){
                    printf("parent.hideLoading();");
                    printf("parent.response_dm_add(\"BT_EXIST\");");
                }
                else{
                    if(multi_file == 1)
                    {
                        Filess *pp;
                        pp = filess_head;
                         //char data[1560]="\0";
						printf("parent.hideLoading();");
                        printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, ",filename,fname);
                        if(pp->length < 1024){
                            printf("#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                        }
                        else{
                            printf("#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                        }
                        while(pp)
                        {
                            pp = pp->next;
                            if(pp)
                            {
                                //char data1[200]="\0";
                                if(pp->length < 1024){
                                    //sprintf(data1,", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                                    printf(", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                                }
                                else{
                                    //sprintf(data1,", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                                    printf(", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                                }
                                //strcat(data,data1);
                            }
                        }
                        //fprintf(stderr,"\n filename=%s\n fname=%s\n \ndata=%s \n",filename,fname,data);
                        //printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, %s\");",filename,fname,data);
                        printf("\");");
                        Filess *p;
                        while(filess_head != NULL)
                        {
                            p = filess_head;
                            filess_head = filess_head->next;
                            free(p);
                        }
                    }
                    else{
                        addlognote(changefilename(fname),infohash_u,changefilename(fname),"nonum",head,D_type);
						printf("parent.hideLoading();");
                        printf("parent.response_dm_add(\"ACK_SUCESS\");");
                        }
                }
                //printf("parent.response_dm_add(\"HEAVY_FULL\");");
                break;
            case BT_STOP:
                //fprintf(stderr,"\nBT_STOP\n");
                unlink("/tmp/APPS/DM2/Status/cgi_running");
                if(multi_file == 1)
                {
                    Filess *pp;
                    pp = filess_head;
					printf("parent.hideLoading();");
                    printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, ",filename,fname);
                    if(pp->length < 1024){
                        //sprintf(data,"#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                        printf("#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                    }
                    else{
                        //sprintf(data,"#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                        printf("#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                    }
                    while(pp)
                    {
                        pp = pp->next;
                        if(pp)
                        {
                            //char data1[200]="\0";
                            if(pp->length < 1024){
                                //sprintf(data1,", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                                printf(", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
                            }
                            else{
                                //sprintf(data1,", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                                printf(", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
                            }
                            //strcat(data,data1);
                        }
                    }
                    //fprintf(stderr,"\n filename=%s\n fname=%s\n \ndata=%s \n",filename,fname,data);
                    //printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, %s\");",filename,fname,data);
                    printf("\");");
                    Filess *p;
                    while(filess_head != NULL)
                    {
                        p = filess_head;
                        filess_head = filess_head->next;
                        free(p);
                    }
                }
                else
                {

                    addlognote(changefilename(fname),infohash_u,changefilename(fname),"nonum",head,D_type);
		    printf("parent.hideLoading();");
                    printf("parent.response_dm_add(\"ACK_SUCESS\");");
                }
                //addlognote(changefilename(fname),changefilename(fname),"nonum",head,D_type);
                //		printf("parent.response_dm_add(\"ACK_SUCESS\");");
                break;
            case NZB_STOP:
                addlognote(changefilename(fname),"nohash",changefilename(fname),"nonum",head,D_type);
                unlink("/tmp/APPS/DM2/Status/cgi_running");
                printf("parent.response_dm_add(\"ACK_SUCESS\");");
                break;
            case LIGHT_FULL:
                printf("parent.response_dm_add(\"LIGHT_FULL\");");
                break;
            case NNTP_FULL:
                if(chk_on_process(changefilename(fname), NULL) > 0){
                    printf("parent.response_dm_add(\"BT_EXIST\");");
                }
                else if(checklognote(changefilename(fname),head)>0){
                    printf("parent.response_dm_add(\"BT_EXIST\");");
                }
                else{
                    addlognote(changefilename(fname),"nohash",changefilename(fname),"nonum",head,D_type);
                }
                //printf("parent.response_dm_add(\"NNTP_FULL\");");
                break;
            case TOTAL_FULL:
		printf("parent.hideLoading();");
                printf("parent.response_dm_add(\"TOTAL_FULL\");");
                break;
            case DISK_FULL:
		printf("parent.hideLoading();");
                printf("parent.response_dm_add(\"DISK_FULL\");");
                break;
            default:
		printf("parent.hideLoading();");
                printf("parent.response_dm_add(\"UNKNOW\");");
                break;
            }
        }
        else{
if(!strcmp(D_type_tmp,"3")){
				sprintf(D_type,"3");
		}
		if(!strcmp(D_type,"3")){
			    if(*filenums && filenums)
			    {
Filess *p;
				while(filess_head != NULL)
				{
				    p = filess_head;
				    filess_head = filess_head->next;
				    free(p);
				}
				//sleep(1);
                                addlognote(changefilename(ffname),infohash_u,changefilename(ffname),filenums,head,"3");
				printf("parent.response_dm_add(\"ACK_SUCESS\");</script>");
				//printf("successful</script>");
				fflush(stdout);
				return 1;
				//ctrl_result = ACK_SUCESS;
			    }
			    else
			    {


		        memset(filename,'\0',sizeof(filename));
        get_file_name(torrent_pos_u, filename);

		        int64_t total_len=0;
        get_file_length(torrent_pos_u, &total_len);
		        //fprintf(stderr,"\nmulti_file=%d\n",multi_file);

		        if(multi_file == 1)
		        {
            get_files_length_path(torrent_pos_u);
		        }

        if(chk_on_process(changefilename(fname), infohash_u) > 0){
				printf("parent.hideLoading();");
		           	printf("parent.response_dm_add(\"BT_EXIST\");");
		        }
			else if(checklognote(changefilename(fname),head)>0){
				printf("parent.hideLoading();");
				printf("parent.response_dm_add(\"BT_EXIST\");");
			}
			else{
				    if(multi_file == 1)
				    {
					Filess *pp;
					pp = filess_head;

					//char data[10240]="\0";
                                        printf("parent.hideLoading();");
					printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, ",filename,fname);
					if(pp->length < 1024){
						//sprintf(data,"#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
						printf("#%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
					}
					else{ 
						//sprintf(data,"#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
						printf("#%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
					}
					while(pp)
					{
					    //fprintf(stderr,"\n*******id=%d\n",pp->id);
					    //fprintf(stderr,"length=%ld\n",pp->length);
					    //fprintf(stderr,"name=%s\n",pp->name);
					    //fprintf(stderr,"path=%s\n",pp->path);
					    //printf("parent.response_dm_add(\"ACK_SUCESS=%d#%s#%ld#%s\");",pp->id,pp->name,pp->length,pp->path);
					    pp = pp->next;
					    if(pp)
					    {
						//char data1[200]="\0";
						if(pp->length < 1024){
						        //sprintf(data1,", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
						        printf(", #%d#%s#%lldKB#%s",pp->id,pp->name,pp->length/1024,pp->path);
						}
						else{
						        //sprintf(data1,", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
						        printf(", #%d#%s#%lldMB#%s",pp->id,pp->name,pp->length/1024/1024,pp->path);
						}
						//strcat(data,data1);
					    }
					}
					//fprintf(stderr,"\n filename=%s\n fname=%s\n \ndata=%s \n",filename,fname,data);
					//printf("parent.response_dm_add(\"BT_ACK_SUCESS=%s, #%s, %s\");",filename,fname,data);
					printf("\");");
						Filess *p;
						while(filess_head != NULL)
						{
						    p = filess_head;
						    filess_head = filess_head->next;
						    free(p);
						}
				    }
				else{
                                        addlognote(changefilename(fname),infohash_u,changefilename(fname),"nonum",head,D_type);
					printf("parent.hideLoading();");
					printf("parent.response_dm_add(\"ACK_SUCESS\");");
				}
			}
			}
		}
		else if(!strcmp(D_type,"4")){
		      	if(chk_on_process(changefilename(fname), NULL) > 0){
		            printf("parent.response_dm_add(\"BT_EXIST\");");
		        }
		        else if(checklognote(changefilename(fname),head)>0){
				printf("parent.response_dm_add(\"BT_EXIST\");");
			}
			else{
                                addlognote(changefilename(fname),"nohash",changefilename(fname),"nonum",head,D_type);
				printf("parent.response_dm_add(\"ACK_SUCESS\");");
			}
		
		}
	}
    }
    else if(up_s==UP_FAIL){
	printf("parent.hideLoading();");
        printf("parent.response_dm_add(\"ACK_FAIL\");");
    }
    else if(up_s==UP_NONE){
	printf("parent.hideLoading();");
        printf("parent.response_dm_add(\"UNKNOW\");");
    }
    printf("</script>");
    fflush(stdout);
    return 1;
}
