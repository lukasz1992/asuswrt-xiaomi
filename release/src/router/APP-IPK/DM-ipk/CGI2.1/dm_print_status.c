#include "dm_print_status.h"
#include "dm_func.h"
#include "dm_hook.h"

int get_ed2k_server_status(){
	int  fd = -1;
	int  fd2 = -1;
	char buf[64];
	char buf2[64];
	char *p = NULL;
	memset(buf, 0, sizeof(buf));

	fd = open("/tmp/dm2_amule_status", O_RDONLY);
	if(fd > 0) {
		if ((read(fd, buf, sizeof(buf)-1)) > 0) {
            if(strstr(buf, "disconnected") != NULL) {
                close(fd);
                return 3;
            }else if(strstr(buf, "connected") != NULL) {
				close(fd);
				return 1;
			} else if (strstr(buf, "connecting") != NULL) {
				if((p = strstr(buf, "server_ip:")) != NULL) {
					fd2 = open("/tmp/dm2_amule_timedout", O_RDONLY);
					if(fd2 > 0) {
						if ((read(fd2, buf2, sizeof(buf2)-1)) > 0) {
							if(strstr(buf2, p+10) != NULL) {
								close(fd);
								close(fd2);
								return 0;
							}
						}
						close(fd2);
					}
				}
				close(fd);
				return 2;
			}
		}
		close(fd);
	}
	return 3;
}

int queue_job(struct Lognote *au_add)
{
    struct Lognote *au_node;
    au_node = jqs_head;

    if((au_node = (struct Lognote*)malloc(sizeof(struct Lognote))) == NULL)
        return -1;

    memcpy(au_node, au_add, sizeof(struct Lognote));
    au_node->next = (struct Lognote*)0;

    if(jqs_rear){
        jqs_rear->next = au_node;
    }else{
        jqs_head = au_node;
    }

    jqs_rear = au_node;

    return 0;
}


/* create a notet*/
//struct Lognote * createnote(int id, char *url, int status)
struct Lognote * createnote(int id,char *infohash, char *url, char *real_url,char *filenums, char *d_type, int status ,int times)
{
    struct Lognote *note = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(note, 0, sizeof(struct Lognote *));
    note->id = id;
    strcpy(note->url, url);
    strcpy(note->infohash, infohash);
    strcpy(note->real_url, real_url);
    strcpy(note->filenum, filenums);
    strcpy(note->type, d_type);
    note->status = status;
    note->checknum = times;
    return note;
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

void read_queue_jobs(struct Lognote *phead)
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

    memset(&au, 0, sizeof(struct Lognote));
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {
        struct Lognote *note = createnote(au.id, au.infohash,au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
        insertnote(note, phead);
	memset(&au, 0x00, sizeof(struct Lognote));
    }
    close(fd_r);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);

}

void free_jobs_queue()
{

    struct Lognote *au, *next_au;

    for(au = jqs_head; au;)
    {
        next_au = au->next;
        free(au);
        au = next_au;
    }
}


int read_tracker(Tracker_struct *tracker_files, char *tracker, int num)
{
    Sem_t  sem;
    int  use_sem = 1;
    int  fd, flags, n;
    char  logname[MAX_NAMELEN];
    char  semname[MAX_NAMELEN];

    memset(logname, '\0',sizeof(logname));
    memset(semname, '\0',sizeof(semname));

    snprintf(logname, sizeof(logname), "%s/%s", Log_dir, tracker);
    snprintf(semname, sizeof(semname), "%s/sem.%s", Sem_dir, tracker);


    flags = O_RDONLY;
    if(Sem_open(&sem, semname, flags) == -1)
        use_sem = 0;
    if(use_sem)
        Sem_wait(&sem);

    if((fd = open(logname, O_RDONLY)) > 0)
    {

        if(n = read(fd, tracker_files, sizeof(Tracker_struct)*num)<0)
        {
            return -1;
        }



        close(fd);
    }
    else
    {
        return -1;
    }
    if(use_sem)
        Sem_post(&sem);

    Sem_close(&sem);
    return 1;

}

int read_file(File_detail *file_message, char *id, int num)
{
    Sem_t  sem;
    int  use_sem = 1;
    int  fd, flags, n;
    char  logname[MAX_NAMELEN];
    char  semname[MAX_NAMELEN];


    memset(logname, '\0',sizeof(logname));
    memset(semname, '\0',sizeof(semname));
    snprintf(logname, sizeof(logname), "%s/%s", Log_dir, id);
    snprintf(semname, sizeof(semname), "%s/sem.%s", Sem_dir, id);
    flags = O_RDONLY;
    if(Sem_open(&sem, semname, flags) == -1)
        use_sem = 0;
    if(use_sem)
        Sem_wait(&sem);

    if((fd = open(logname, O_RDONLY)) > 0)
    {
        lseek(fd, 0, SEEK_SET);
        lseek(fd, sizeof(Log_struc), SEEK_SET);


        if(n = read(fd, file_message, sizeof(File_detail)*num)<0)
        {
            return -1;
        }



        close(fd);
    }
    else
    {
        return -1;
    }
    if(use_sem)
        Sem_post(&sem);

    Sem_close(&sem);
    return 1;

}

char *reLineFeedtoSpace(char *src, size_t size) {
	int i = 0;
	for(i = 0; i < size && src[i] != '\0'; i++){
		if(src[i] == '\n')
			src[i] = ' ';
	}
	return src;
}


void print_log( Log_struc *log, char *logid, int log_index, char* cmd)
{
	int ret = 0;
    char  s_filesize[16];
    char  s_progress[16];
    char  s_status[20];
    char  s_elapsed[50];
    char  s_type[10];
    char  s_error[256];
    char  s_have[16]; // SZ-Magic 20120618 S_have
    int  Diff_t = 0, elapsed_hr = 0, elapsed_min = 0, elapsed_sec = 0;
    int  peers = 0;
    double  adr = 0.00, aur = 0.00;
    char s_adr[16], s_aur[16];
    char title[1024];
    char s_downloaddir[256];  //20121030 magic for open link

    memset(s_filesize, '\0', sizeof(s_filesize));
    memset(s_progress, '\0', sizeof(s_progress));
    memset(s_status, '\0', sizeof(s_status));
    memset(s_elapsed, '\0', sizeof(s_elapsed));
    memset(s_type, '\0', sizeof(s_type));
    memset(s_error, '\0', sizeof(s_error));
    memset(s_have, '\0', sizeof(s_have)); // SZ-Magic 20120618 S_have
    memset(s_downloaddir, '\0', sizeof(s_downloaddir)); //20121030 magic for open link

    if(!logid)
        logid = NULL;

    // filename
    memset(s_adr, '\0', sizeof(s_adr));
    memset(s_aur, '\0', sizeof(s_aur));
    memset(title, '\0', sizeof(title));

    if(!log->filename){
        strncpy(log->filename, "noName", 6);
    }
	snprintf(title, sizeof(title), "%s", log->filename);
	reLineFeedtoSpace(title, sizeof(title));
    snprintf(s_progress, sizeof(s_progress),"%.3f ", log->progress);

    logs_id[log_index].status = log->status;

	switch(log->status)
	{
	case S_INITIAL:
		snprintf(s_status, sizeof(s_status), "%s", "Initialing");
		break;
	case S_PROCESSING:
		snprintf(s_status, sizeof(s_status), "%s", "Downloading");
		if(log->download_type == ED2K) {
			ret = get_ed2k_server_status();
			if(ret == 2) {
				snprintf(s_status, sizeof(s_status), "%s", "Connecting");
            } else if(ret == 0) {
				snprintf(s_status, sizeof(s_status), "%s", "Timeout");
            } else if(ret == 3) {
                snprintf(s_status, sizeof(s_status), "%s", "Disconnected");
            }
		}
		break;
	case S_PAUSED:
		snprintf(s_status, sizeof(s_status), "%s", "Paused");
		break;
	case S_COMPLETED:
		snprintf(s_status, sizeof(s_status), "%s", "Finished");
		break;
	case S_SEEDING:
		snprintf(s_status, sizeof(s_status), "%s","Seeding");
		break;
	case S_DEAD_OF_DELETED:
		snprintf(s_status, sizeof(s_status), "%s","Stop");
		break;
	case S_DEAD_OF_ERROR:
		snprintf(s_status, sizeof(s_status), "%s","Error");
		break;
	case S_TYPE_OF_ERROR:
		snprintf(s_status, sizeof(s_status), "%s", "Vfat4G");
		break;
	case S_MOVE4GBERROR:
		snprintf(s_status, sizeof(s_status), "%s","Move4GBError");
		break;
	case S_HASH:
		snprintf(s_status, sizeof(s_status), "%s","Hashing");
		break;
	case S_PARSER_FAIL:
		snprintf(s_status, sizeof(s_status), "%s", "Error");
		break;
	case S_CONNECTFAIL:
		snprintf(s_status, sizeof(s_status), "%s", "ConnectFail");
		break;
	case S_SSLFAIL:
		snprintf(s_status, sizeof(s_status), "%s", "SSLFail");
		break;
	case S_ACCOUNTFAIL:
		snprintf(s_status, sizeof(s_status), "%s","AccountFail");
		break;
	case S_RECVFAIL:
		snprintf(s_status, sizeof(s_status), "%s","SocketRecvFail");
		break;
	case S_NOTCOMPLETED:
		snprintf(s_status, sizeof(s_status), "%s","NotCompleted");
		break;
	case S_DISKFULL:
		snprintf(s_status, sizeof(s_status), "%s", "Diskfull");
		break;
	case S_MOVEDISKFULL:
		snprintf(s_status, sizeof(s_status), "%s", "MoveDiskFull");
		break;
	default:
		snprintf(s_status, sizeof(s_status), "%s", "Initialing");
	}

    // type
    switch(log->download_type)
    {
	
    case BT:
        snprintf(s_type, sizeof(s_type), "%s", "BT");
        break;
    case FTP:
        snprintf(s_type, sizeof(s_type),  "%s","FTP");
        break;
    case HTTP:
        snprintf(s_type, sizeof(s_type),  "%s","HTTP");
        break;
    case NZB:
        snprintf(s_type, sizeof(s_type),  "%s","NZB");
        break;
    case ED2K:
        snprintf(s_type, sizeof(s_type), "%s", "ED2K");
        break;
    default:
        snprintf(s_type, sizeof(s_type),  "%s","UNKNOWN");
    }

    // pass time
    if(log->begin_t)
    {
        if(log->now_t)
        {
            Diff_t = (int)difftime(log->now_t, log->begin_t);
            elapsed_hr = Diff_t / 3600;
            Diff_t = Diff_t % 3600;
            elapsed_min = Diff_t / 60;
            Diff_t = Diff_t % 60;
            elapsed_sec = Diff_t;
            char s_hr[16];
            char s_min[3];
            char s_sec[3];
            memset(s_hr, '\0', sizeof(s_hr));
            memset(s_min, '\0', sizeof(s_min));
            memset(s_sec, '\0', sizeof(s_sec));
            if(elapsed_hr < 10)
                snprintf(s_hr, sizeof(s_hr), "0%d",elapsed_hr);
            else
                snprintf(s_hr, sizeof(s_hr), "%d",elapsed_hr);
            if(elapsed_min < 10)
                snprintf(s_min, sizeof(s_min), "0%d",elapsed_min);
            else
                snprintf(s_min, sizeof(s_min), "%d",elapsed_min);
            if(elapsed_sec < 10)
                snprintf(s_sec, sizeof(s_sec), "0%d",elapsed_sec);
            else
                snprintf(s_sec, sizeof(s_sec), "%d",elapsed_sec);
            snprintf(s_elapsed, sizeof(s_elapsed), "%s:%s:%s", s_hr, s_min, s_sec);

        }
    }

	//download speed
	switch(log->download_type)
	{
	case BT:
		adr = (double)(log->ifile.iBt.downloadSpeed);
		aur = (double)(log->ifile.iBt.uploadSpeed);
		snprintf(s_adr, sizeof(s_adr), "%.2f KBps", adr);
		snprintf(s_aur, sizeof(s_aur),"%.2f KBps", aur);
		peers = log->ifile.iBt.downloadpeers;
		break;
	case FTP:
		adr = (double)log->ifile.iFtp.rate;
		snprintf(s_adr, sizeof(s_adr), "%.2f KBps", adr);
		snprintf(s_aur, sizeof(s_aur),"0 KBps");
		break;
	case HTTP:
		adr = (double)log->ifile.iHttp.rate;
		snprintf(s_adr, sizeof(s_adr), "%.2f KBps", adr);
		snprintf(s_aur, sizeof(s_aur),"0 KBps");
		break;
	case NZB:
		adr = (double)log->ifile.iNzb.rate;
		snprintf(s_adr, sizeof(s_adr), "%.2f KBps", adr);
		snprintf(s_aur, sizeof(s_aur),"0 KBps");
		break;
	case ED2K:
		adr = (double)(log->ifile.iEd2k.downloadSpeed);
		aur = (double)(log->ifile.iEd2k.uploadSpeed);
		snprintf(s_adr, sizeof(s_adr), "%.2f KBps", adr);
		snprintf(s_aur, sizeof(s_aur),"%.2f KBps", aur);
		peers = log->ifile.iEd2k.downloadpeers;
		break;
	}
    // filesize
    switch(log->download_type)
    {
    case BT:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
        else if(log->filesize < 1073741824)
            snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));
        break;
    case FTP:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d MB", (int)(log->filesize / 1024));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1048576.00));
        break;
    case HTTP:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d MB", (int)(log->filesize / 1024));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1048576.00));
        break;
    case NZB:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
        else if(log->filesize < 1073741824)
            snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));
        break;
    case ED2K:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
        else if(log->filesize < 1073741824)
            snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));
        break;
    default:
        if(log->filesize < 1024)
            snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
        else if(log->filesize < 1048576)
            snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
        else if(log->filesize < 1073741824)
            snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
        else
            snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));
        break;
    }

	// errorString
	switch(log->download_type)
	{
	case BT:
		char_to_ascii_safe(s_error,log->ifile.iBt.errorString,256);
		snprintf(s_have, sizeof(s_have), "%.4f", (double)(log->progress));
		if(!strcmp(s_have, "nan"))
		{
			snprintf(s_have, sizeof(s_have),"%s","0.000");
		}
		break;
	case ED2K:
		char_to_ascii_safe(s_error,log->ifile.iEd2k.errorString,256);
		snprintf(s_have, sizeof(s_have), "%s", "0");
		break;
	default:
		if((log->error) && (log->fullname)){
			strncpy(s_error, log->fullname, min(strlen(log->fullname), sizeof(s_error)));
		}
		snprintf(s_have, sizeof(s_have), "%s", "0");
		break;
	}

	// download dir
	switch(log->download_type) //20121030 magic for open link
	{
	
	case BT:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s", log->ifile.iBt.downloadDir);
		break;
	case FTP:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s", log->ifile.iFtp.Destination);
		break;
	case HTTP:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s", log->ifile.iHttp.Destination);
		break;
	case NZB:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s", log->ifile.iNzb.Destination);
		break;
	case ED2K:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s", log->ifile.iEd2k.downloadDir);
		break;
	default:
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s","UNKNOWN");
	}

	//disk info
    struct disk_info *disk_tmp;
    disk_tmp = follow_disk_info_start;
    while(disk_tmp!=NULL)
    {
        if(disk_tmp->mountpath!=NULL)
        {
			if((strncmp(disk_tmp->serialnum,log->ifile.iHttp.serial,strlen(disk_tmp->serialnum))==0) \
					&&(strncmp(disk_tmp->product,log->ifile.iHttp.pid,strlen(disk_tmp->product))==0) \
					&&(strncmp(disk_tmp->vendor,log->ifile.iHttp.vid,strlen(disk_tmp->vendor))==0) \
					&&(disk_tmp->partitionport==log->ifile.iHttp.partitionnum))
					break;
			else if((strncmp(disk_tmp->serialnum,log->ifile.iFtp.serial,strlen(disk_tmp->serialnum))==0) \
					&&(strncmp(disk_tmp->product,log->ifile.iFtp.pid,strlen(disk_tmp->product))==0) \
					&&(strncmp(disk_tmp->vendor,log->ifile.iFtp.vid,strlen(disk_tmp->vendor))==0) \
					&&(disk_tmp->partitionport==log->ifile.iFtp.partitionnum))
					break;
			else if((strncmp(disk_tmp->serialnum,log->ifile.iNzb.serial,strlen(disk_tmp->serialnum))==0) \
					&&(strncmp(disk_tmp->product,log->ifile.iNzb.pid,strlen(disk_tmp->product))==0) \
					&&(strncmp(disk_tmp->vendor,log->ifile.iNzb.vid,strlen(disk_tmp->vendor))==0) \
					&&(disk_tmp->partitionport==log->ifile.iNzb.partitionnum))
					break;
			else if((strncmp(disk_tmp->serialnum,log->ifile.iEd2k.serial,strlen(disk_tmp->serialnum))==0) \
					&&(strncmp(disk_tmp->product,log->ifile.iEd2k.pid,strlen(disk_tmp->product))==0) \
					&&(strncmp(disk_tmp->vendor,log->ifile.iEd2k.vid,strlen(disk_tmp->vendor))==0) \
					&&(disk_tmp->partitionport==log->ifile.iEd2k.partitionnum))
					break;
			else if((strncmp(disk_tmp->serialnum,log->ifile.iBt.serial,strlen(disk_tmp->serialnum))==0) \
					&&(strncmp(disk_tmp->product,log->ifile.iBt.pid,strlen(disk_tmp->product))==0) \
					&&(strncmp(disk_tmp->vendor,log->ifile.iBt.vid,strlen(disk_tmp->vendor))==0) \
					&&(disk_tmp->partitionport==log->ifile.iBt.partitionnum))
                	break;
		}
		disk_tmp=disk_tmp->next;
    }

	char *p;
	if(disk_tmp){
		if((disk_tmp->mountpath)&&(strncmp(s_downloaddir,"not_found",9)!=0) \
		&&(memcmp(disk_tmp->mountpath,s_downloaddir,strlen(disk_tmp->mountpath))!=0))
		{
			p=my_nstrchr('/',s_downloaddir,4);
			char downloadtmp[strlen(p)+1];
			strncpy(downloadtmp,p,strlen(p));
			memset(s_downloaddir,'\0',sizeof(s_downloaddir));
			memcpy(s_downloaddir,disk_tmp->mountpath,sizeof(disk_tmp->mountpath));
			strcat(s_downloaddir,downloadtmp);
		}
		else if((disk_tmp->mountpath)&&(strncmp(s_downloaddir,"not_found",9)==0))
		{
			p=s_downloaddir;
			p=p+9;
			if(p){
				char downloadtmp[strlen(p)+1];
				strncpy(downloadtmp,p,strlen(p));
				memset(s_downloaddir,'\0',sizeof(s_downloaddir));
				memcpy(s_downloaddir,disk_tmp->mountpath,sizeof(disk_tmp->mountpath));
				strcat(s_downloaddir,downloadtmp);
			}
			else{
				memset(s_downloaddir,'\0',sizeof(s_downloaddir));
				memcpy(s_downloaddir,disk_tmp->mountpath,sizeof(disk_tmp->mountpath));
			}
		}
	} else {
		memset(s_downloaddir,'\0',sizeof(s_downloaddir));
		snprintf(s_downloaddir, sizeof(s_downloaddir), "%s/Download2/Complete",Base_dir);
	}

    char check_file[1024];
    memset(check_file, '\0', sizeof(check_file));
    snprintf(check_file, sizeof(check_file),"%s/%s", s_downloaddir,title);

    if ( !access(check_file,0) ){
	} else {
        snprintf(s_downloaddir, sizeof(s_downloaddir), "%s","not_found");
    }


	if(!strcmp(cmd,"All") || !strcmp(cmd,s_status))
	{
		if(first_log)
		{
			printf( "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logid, title, s_progress, s_filesize, s_status, s_type, s_elapsed, s_adr, s_aur, peers, s_error,s_have,s_downloaddir);
			first_log = 0;
		} else
			printf( ",[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logid, title, s_progress, s_filesize, s_status, s_type, s_elapsed, s_adr, s_aur, peers, s_error,s_have,s_downloaddir);
	}
}

void print_queue_jobs(struct Lognote *au, int qid)
{

    char logid[12];
    char *job_name;
    char *s_progress = "0.0";
    char *s_filesize = "";
    char *s_status = "notbegin";
    char *s_type;
    char *s_elapsed = "";
    char *s_adr = "";
    char *s_aur = "";
    int peers = 0;
    char *s_error = "";
    char *s_have = "0.0000";
    char *s_downloaddir = ""; //20121030 magic for open link

    memset(logid, '\0', sizeof(logid));
    snprintf(logid, sizeof(logid), "%d", qid);

    job_name = au->url;
    if(!strcmp(au->type,"3"))
    {
        s_type = "BT";
    }
    else if(!strcmp(au->type,"1"))
    {
        s_type = "HTTP";
    }
    else if(!strcmp(au->type,"2"))
    {
        s_type = "FTP";
    }
    else if(!strcmp(au->type,"4"))
    {
        s_type = "NZB";
    }
    else if(!strcmp(au->type,"6"))
    {
        s_type = "ED2K";
    }
    else if(!strcmp(au->type,"5"))
    {
        s_type = "OTHER";

    }
    else if(!strcmp(au->type,"UNKNOWN"))
    {
        s_type = "UNKNOWN";
    }

	switch(au->status){
		case S_PARSER_FAIL:
			//s_status="error";
			break;
		case S_NEEDACCOUNT:
			s_status="warning";
			break;
		default:
			break;
	}

    s_adr = "0 KBps";
    s_aur = "0 KBps";
    if(first_log){
        printf( "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logid, job_name, s_progress, s_filesize, s_status, s_type, s_elapsed, s_adr, s_aur, peers, s_error,s_have,s_downloaddir);       // SZ-Magic, 090713 20120618 S_have 20121030 s_downloaddir
        first_log = 0;
    } else{
        printf( ",[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logid, job_name, s_progress, s_filesize, s_status, s_type, s_elapsed, s_adr, s_aur, peers, s_error,s_have,s_downloaddir);        // SZ-Magic, 090713 20120618 S_have 20121030 s_downloaddir
    }
}

/*void delet(char *s,int d)
{

    int i,j;
    for(i=0;s[i]!='\0';i++)
	if(s[i]==d)
            for(j=i;s[j]!='\0';j++)
                s[j]=s[j+1];
}*/

void print_single_log( Log_struc *log,  char *logid, char *type)
{
	char * per = "%";
	char  s_pieces[32];
	char  s_piecesize[16];
	char  s_piececount[16];
	char  s_filesize[32];
	char  s_progress[16];
	char  s_status[16];
	char  s_elapsed[50];
	char  s_type[10];
	char  s_error[256];
	char  s_hash[64];
	char  s_secure[32];
	char  s_comment[32];
	char  s_creator[32];
	char  s_dateCreated[64];
	char  *s_downloaddir = NULL;

	char s_have[16];
	char s_verified[16];
	char s_availability[16];
	char s_upsize[16];
	char s_downsize[16];
	char s_upspeed[16];
	char s_downspeed[16];
	char s_errorstring[256];
	char s_ratio[8];
	char s_uppeer[8];
	char s_downpeer[8];


	memset(s_pieces, '\0', sizeof(s_pieces));
	memset(s_piecesize, '\0', sizeof(s_piecesize));
	memset(s_piececount, '\0', sizeof(s_piececount));
	memset(s_filesize, '\0', sizeof(s_filesize));
	memset(s_progress, '\0', sizeof(s_progress));
	memset(s_status, '\0', sizeof(s_status));
	memset(s_elapsed, '\0', sizeof(s_elapsed));
	memset(s_type, '\0', sizeof(s_type));
	memset(s_error, '\0', sizeof(s_error));
	memset(s_hash, '\0', sizeof(s_hash));
	memset(s_secure, '\0', sizeof(s_secure));
	memset(s_comment, '\0', sizeof(s_comment));
	memset(s_creator, '\0', sizeof(s_creator));
	memset(s_dateCreated, '\0', sizeof(s_dateCreated));

	memset(s_have, '\0', sizeof(s_have));
	memset(s_verified, '\0', sizeof(s_verified));
	memset(s_availability, '\0', sizeof(s_availability));
	memset(s_upsize, '\0', sizeof(s_upsize));
	memset(s_downsize, '\0', sizeof(s_downsize));
	memset(s_upspeed, '\0', sizeof(s_upspeed));
	memset(s_downspeed, '\0', sizeof(s_downspeed));
	memset(s_errorstring, '\0', sizeof(s_errorstring));
	memset(s_ratio, '\0', sizeof(s_ratio));
	memset(s_uppeer, '\0', sizeof(s_uppeer));
	memset(s_downpeer, '\0', sizeof(s_downpeer));

	// id
	if(!logid)
		logid = NULL;

	char *logtab;
	logtab = websGetVar(wp, "logTab", "");

	switch(log->download_type){
	case ED2K:
	{
		if(!strcmp(logtab,"0"))
		{
			/*
			if(log->ifile.iEd2k.pieceSize < 1024)
				snprintf(s_piecesize, sizeof(s_piecesize), "%d B", (int)log->ifile.iEd2k.pieceSize);
			else if(log->ifile.iBt.pieceSize < 1048576)
				snprintf(s_piecesize, sizeof(s_piecesize), "%d KB", (int)(log->ifile.iEd2k.pieceSize / 1024));
			else if(log->ifile.iBt.pieceSize < 1073741824)
				snprintf(s_piecesize, sizeof(s_piecesize), "%.2f MB", (double)((double)log->ifile.iEd2k.pieceSize / 1048576.00));
			else
				snprintf(s_piecesize, sizeof(s_piecesize), "%.2f GB", (double)((double)log->ifile.iEd2k.pieceSize / 1073741824.00));
			snprintf(s_piececount, sizeof(s_piececount),"%d",log->ifile.iEd2k.pieceCount);
			snprintf(s_pieces, sizeof(s_pieces),"%s,%s",s_piecesize,s_piececount);
			*/
			snprintf(s_pieces, sizeof(s_pieces),"%s,%s","0","0");//for tmp
			snprintf(s_hash, sizeof(s_hash), "%s", log->id);

			switch(log->ifile.iEd2k.isPrivate)
			{
			case 0:
				snprintf(s_secure, sizeof(s_secure), "%s", "Public ed2k");
				break;
			case 1:
				snprintf(s_secure, sizeof(s_secure), "%s", "Private ed2k");
				break;
			}
			snprintf(s_secure, sizeof(s_secure),"%s", "Public ed2k"); //for tmp
			//snprintf(s_comment, "%s", log->ifile.iEd2k.comment);
			//snprintf(s_comment, sizeof(s_comment), "%s", "");//for tmp
			snprintf(s_creator, sizeof(s_creator), "%s", "amule");
			snprintf(s_dateCreated, sizeof(s_dateCreated), "%s", ctime(&log->ifile.iEd2k.dateCreated));
			delet(s_dateCreated,10);

			struct disk_info *disks_tmp;
			disks_tmp = follow_disk_info_start;
			while(disks_tmp!=NULL)
			{
				if(disks_tmp->mountpath!=NULL)
				{
					if((strncmp(disks_tmp->serialnum,log->ifile.iEd2k.serial,strlen(disks_tmp->serialnum))==0)&&(strncmp(disks_tmp->product,log->ifile.iEd2k.pid,strlen(disks_tmp->product))==0)&&(strncmp(disks_tmp->vendor,log->ifile.iEd2k.vid,strlen(disks_tmp->vendor))==0)&&(disks_tmp->partitionport==log->ifile.iEd2k.partitionnum))
						break;
					else
						disks_tmp=disks_tmp->next;
				}
				else
					disks_tmp=disks_tmp->next;
			}
			if(disks_tmp)
			{

				char *p;
				if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iEd2k.downloadDir,"not_found",9)!=0))
				{
					p=my_nstrchr('/',log->ifile.iEd2k.downloadDir,4);
					char downloadtmp[strlen(p)+1];
					memset(downloadtmp,'\0',sizeof(downloadtmp));
					strncpy(downloadtmp,p,strlen(p));
					memset(log->ifile.iEd2k.downloadDir,'\0',sizeof(log->ifile.iEd2k.downloadDir));
					strcpy(log->ifile.iEd2k.downloadDir,disks_tmp->mountpath);
					strcat(log->ifile.iEd2k.downloadDir,downloadtmp);
				}
				else if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iEd2k.downloadDir,"not_found",9)==0))
				{
					p=log->ifile.iEd2k.downloadDir;
					p=p+9;
					if(p){
						char downloadtmp[strlen(p)+1];
						memset(downloadtmp,'\0',sizeof(downloadtmp));
						strncpy(downloadtmp,p,strlen(p));
						memset(log->ifile.iEd2k.downloadDir,'\0',sizeof(log->ifile.iEd2k.downloadDir));
						strcpy(log->ifile.iEd2k.downloadDir,disks_tmp->mountpath);
						strcat(log->ifile.iEd2k.downloadDir,downloadtmp);
					}
					else{
						memset(log->ifile.iEd2k.downloadDir,'\0',sizeof(log->ifile.iEd2k.downloadDir));
						strcpy(log->ifile.iEd2k.downloadDir,disks_tmp->mountpath);
					}
				}

				printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", logid, s_pieces, s_hash, s_secure, s_comment, s_creator, s_dateCreated, log->ifile.iEd2k.downloadDir);
			}
			else
			{

				char defaultdldir[100];
				memset(defaultdldir,'\0',sizeof(defaultdldir));
				snprintf(defaultdldir, sizeof(defaultdldir), "%s/Download2/Complete",Base_dir);
				printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", logid, s_pieces, s_hash, s_secure, s_comment, s_creator, s_dateCreated, defaultdldir);
			}
		}

		else if(!strcmp(logtab,"1"))
		{


			snprintf(s_progress, sizeof(s_progress), "%.2f %s ", log->progress*100,per);


			switch(log->status)
			{
			case S_INITIAL:
				snprintf(s_status, sizeof(s_status), "Initialing");
				break;
			case S_PROCESSING:
				snprintf(s_status, sizeof(s_status),"Downloading");
				break;
			case S_PAUSED:
				snprintf(s_status, sizeof(s_status),"Paused");
				break;
			case S_COMPLETED:
				snprintf(s_status, sizeof(s_status), "Finished");
				break;
			case S_SEEDING:
				snprintf(s_status, sizeof(s_status), "Seeding");
				break;
			case S_DEAD_OF_DELETED:
				snprintf(s_status, sizeof(s_status), "Stop");
				break;
			case S_DEAD_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Error");
				break;
			case S_TYPE_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Vfat4G");
				break;
			case S_HASH:
				snprintf(s_status, sizeof(s_status), "Hashing");
				break;
			case S_DISKFULL:
				snprintf(s_status, sizeof(s_status),"Diskfull");
				break;
			default:
				snprintf(s_status, sizeof(s_status), "Initialing");
			}

			snprintf(s_availability, sizeof(s_availability), "%.2f %s", log->ifile.iEd2k.availability *100,per);

			if(log->ifile.iBt.uploadedsize < 1024)
				snprintf(s_upsize, sizeof(s_upsize), "%d B", (int)log->ifile.iEd2k.uploadedsize);
			else if(log->ifile.iBt.haveValid < 1048576)
				snprintf(s_upsize, sizeof(s_upsize), "%d KB", (int)(log->ifile.iEd2k.uploadedsize / 1024));
			else if(log->ifile.iBt.haveValid < 1073741824)
				snprintf(s_upsize, sizeof(s_upsize), "%.2f MB", (double)((double)log->ifile.iEd2k.uploadedsize / 1048576.00));
			else
				snprintf(s_upsize, sizeof(s_upsize), "%.2f GB", (double)((double)log->ifile.iEd2k.uploadedsize / 1073741824.00));

			snprintf(s_upsize, sizeof(s_upsize), "%d", 0); //for tmp
			snprintf(s_upspeed, sizeof(s_upspeed), "%.2f KBps", log->ifile.iEd2k.uploadSpeed);
			snprintf(s_downspeed, sizeof(s_downspeed), "%.2f KBps", log->ifile.iEd2k.downloadSpeed);
			snprintf(s_errorstring, sizeof(s_errorstring), "%s", "");  //for tmp
			snprintf(s_ratio, sizeof(s_ratio), "%.2f", log->ifile.iEd2k.ratio);
			snprintf(s_uppeer, sizeof(s_uppeer), "%d", 0); //for tmp
			snprintf(s_downpeer, sizeof(s_downpeer), "%d", log->ifile.iEd2k.downloadpeers);


			if(log->filesize < 1024){
				snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
				snprintf(s_downsize, sizeof(s_downsize), "%.2f B", (double)((double)log->filesize * log->progress));
			}
			else if(log->filesize < 1048576){
				snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
				snprintf(s_downsize, sizeof(s_downsize), "%.2f KB", (double)((double)log->filesize /1024 * log->progress));
			}
			else if(log->filesize < 1073741824){
				snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
				snprintf(s_downsize, sizeof(s_downsize), "%.2f MB", (double)((double)log->filesize /1048576.00 * log->progress));
			}
			else{
				snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));
				snprintf(s_downsize, sizeof(s_downsize), "%.2f GB", (double)((double)log->filesize /1073741824.00 * log->progress));
			}
			printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", s_progress, s_filesize, s_status, s_availability, s_downsize, s_upsize, s_downspeed, s_upspeed, s_errorstring, s_ratio, s_uppeer, s_downpeer);
		}

	}
		break;
	case BT:
	{
		if(!strcmp(logtab,"0"))
		{

			if(log->ifile.iBt.pieceSize < 1024)
				snprintf(s_piecesize, sizeof(s_piecesize), "%d B", (int)log->ifile.iBt.pieceSize);
			else if(log->ifile.iBt.pieceSize < 1048576)
				snprintf(s_piecesize, sizeof(s_piecesize), "%d KB", (int)(log->ifile.iBt.pieceSize / 1024));
			else if(log->ifile.iBt.pieceSize < 1073741824)
				snprintf(s_piecesize, sizeof(s_piecesize), "%.2f MB", (double)((double)log->ifile.iBt.pieceSize / 1048576.00));
			else
				snprintf(s_piecesize, sizeof(s_piecesize), "%.2f GB", (double)((double)log->ifile.iBt.pieceSize / 1073741824.00));
			snprintf(s_piececount, sizeof(s_piececount),"%d",log->ifile.iBt.pieceCount);
			snprintf(s_pieces, sizeof(s_pieces),"%s,%s",s_piecesize,s_piececount);
			snprintf(s_hash, sizeof(s_hash), "%s", log->id);

			switch(log->ifile.iBt.isPrivate)
			{
			case 0:
				snprintf(s_secure, sizeof(s_secure), "%s", "Public Torrent");
				break;
			case 1:
				snprintf(s_secure, sizeof(s_secure), "%s", "Private Torrent");
				break;
			}

			snprintf(s_comment, sizeof(s_comment), "%s", log->ifile.iBt.comment);
			snprintf(s_creator, sizeof(s_creator), "%s", log->ifile.iBt.creator);
			snprintf(s_dateCreated, sizeof(s_dateCreated), "%s", ctime(&log->ifile.iBt.dateCreated));
			delet(s_dateCreated,10);

			struct disk_info *disks_tmp;
			disks_tmp = follow_disk_info_start;
			while(disks_tmp!=NULL)
			{
				if(disks_tmp->mountpath!=NULL)
				{
					if((strncmp(disks_tmp->serialnum,log->ifile.iBt.serial,strlen(disks_tmp->serialnum))==0)&&(strncmp(disks_tmp->product,log->ifile.iBt.pid,strlen(disks_tmp->product))==0)&&(strncmp(disks_tmp->vendor,log->ifile.iBt.vid,strlen(disks_tmp->vendor))==0)&&(disks_tmp->partitionport==log->ifile.iBt.partitionnum))
						break;
					else
						disks_tmp=disks_tmp->next;
				}
				else
					disks_tmp=disks_tmp->next;
			}
			if(disks_tmp)
			{
				char *p;
				if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iBt.downloadDir,"not_found",9)!=0))
				{
					p=my_nstrchr('/',log->ifile.iBt.downloadDir,4);
					char downloadtmp[strlen(p)+1];
					memset(downloadtmp,'\0',sizeof(downloadtmp));
					strncpy(downloadtmp,p,strlen(p));
					memset(log->ifile.iBt.downloadDir,'\0',sizeof(log->ifile.iBt.downloadDir));
					strcpy(log->ifile.iBt.downloadDir,disks_tmp->mountpath);
					strcat(log->ifile.iBt.downloadDir,downloadtmp);
				}
				else if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iBt.downloadDir,"not_found",9)==0))
				{
					//fprintf(stderr,"\nrecover base_dir22\n");
					p=log->ifile.iBt.downloadDir;
					p=p+9;
					if(p){
						char downloadtmp[strlen(p)+1];
						memset(downloadtmp,'\0',sizeof(downloadtmp));
						strncpy(downloadtmp,p,strlen(p));
						memset(log->ifile.iBt.downloadDir,'\0',sizeof(log->ifile.iBt.downloadDir));
						strcpy(log->ifile.iBt.downloadDir,disks_tmp->mountpath);
						strcat(log->ifile.iBt.downloadDir,downloadtmp);
					}
					else{
						memset(log->ifile.iBt.downloadDir,'\0',sizeof(log->ifile.iBt.downloadDir));
						strcpy(log->ifile.iBt.downloadDir,disks_tmp->mountpath);
					}
				}
				printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", logid, s_pieces, s_hash, s_secure, s_comment, s_creator, s_dateCreated, log->ifile.iBt.downloadDir);
			}
			else
			{
				//fprintf(stderr,"\nthe desired download folder is not mount\n");
				char defaultdldir[100];
				memset(defaultdldir,'\0',sizeof(defaultdldir));
				snprintf(defaultdldir, sizeof(defaultdldir), "%s/Download2/Complete",Base_dir);
				printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", logid, s_pieces, s_hash, s_secure, s_comment, s_creator, s_dateCreated, defaultdldir);
			}
		}

		else if(!strcmp(logtab,"1"))
		{


			snprintf(s_progress, sizeof(s_progress), "%.2f %s ", log->progress*100,per);

			if(log->ifile.iBt.have < 1024)
				snprintf(s_have, sizeof(s_have), "%d B", (int)log->ifile.iBt.have);
			else if(log->ifile.iBt.have < 1048576)
				snprintf(s_have, sizeof(s_have), "%d KB", (int)(log->ifile.iBt.have / 1024));
			else if(log->ifile.iBt.have < 1073741824)
				snprintf(s_have, sizeof(s_have), "%.2f MB", (double)((double)log->ifile.iBt.have / 1048576.00));
			else
				snprintf(s_have, sizeof(s_have), "%.2f GB", (double)((double)log->ifile.iBt.have / 1073741824.00));

			if(log->ifile.iBt.haveValid < 1024)
				snprintf(s_verified, sizeof(s_verified), "%d B", (int)log->ifile.iBt.haveValid);
			else if(log->ifile.iBt.haveValid < 1048576)
				snprintf(s_verified, sizeof(s_verified), "%d KB", (int)(log->ifile.iBt.haveValid / 1024));
			else if(log->ifile.iBt.haveValid < 1073741824)
				snprintf(s_verified, sizeof(s_verified), "%.2f MB", (double)((double)log->ifile.iBt.haveValid / 1048576.00));
			else
				snprintf(s_verified, sizeof(s_verified), "%.2f GB", (double)((double)log->ifile.iBt.haveValid / 1073741824.00));

			snprintf(s_filesize, sizeof(s_filesize), "%s (%s verified) ",s_have,s_verified);

			switch(log->status)
			{
			case S_INITIAL:
				snprintf(s_status, sizeof(s_status), "Initialing");
				break;
			case S_PROCESSING:
				snprintf(s_status, sizeof(s_status),"Downloading");
				break;
			case S_PAUSED:
				snprintf(s_status, sizeof(s_status),"Paused");
				break;
			case S_COMPLETED:
				snprintf(s_status, sizeof(s_status), "Finished");
				break;
			case S_SEEDING:
				snprintf(s_status, sizeof(s_status), "Seeding");
				break;
			case S_DEAD_OF_DELETED:
				snprintf(s_status, sizeof(s_status), "Stop");
				break;
			case S_DEAD_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Error");
				break;
			case S_TYPE_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Vfat4G");
				break;
			case S_HASH:
				snprintf(s_status, sizeof(s_status), "Hashing");
				break;
			case S_DISKFULL:
				snprintf(s_status, sizeof(s_status),"Diskfull");
				break;
			default:
				snprintf(s_status, sizeof(s_status), "Initialing");
			}

			snprintf(s_availability, sizeof(s_availability), "%.2f %s", log->ifile.iBt.availability *100,per);

			if(log->ifile.iBt.uploadedsize < 1024)
				snprintf(s_upsize, sizeof(s_upsize), "%d B", (int)log->ifile.iBt.uploadedsize);
			else if(log->ifile.iBt.haveValid < 1048576)
				snprintf(s_upsize, sizeof(s_upsize), "%d KB", (int)(log->ifile.iBt.uploadedsize / 1024));
			else if(log->ifile.iBt.haveValid < 1073741824)
				snprintf(s_upsize, sizeof(s_upsize), "%.2f MB", (double)((double)log->ifile.iBt.uploadedsize / 1048576.00));
			else
				snprintf(s_upsize, sizeof(s_upsize), "%.2f GB", (double)((double)log->ifile.iBt.uploadedsize / 1073741824.00));

			if(log->ifile.iBt.downloadedsize < 1024)
				snprintf(s_downsize, sizeof(s_downsize), "%d B", (int)log->ifile.iBt.downloadedsize);
			else if(log->ifile.iBt.haveValid < 1048576)
				snprintf(s_downsize, sizeof(s_downsize), "%d KB", (int)(log->ifile.iBt.downloadedsize / 1024));
			else if(log->ifile.iBt.haveValid < 1073741824)
				snprintf(s_downsize, sizeof(s_downsize), "%.2f MB", (double)((double)log->ifile.iBt.downloadedsize / 1048576.00));
			else
				snprintf(s_downsize, sizeof(s_downsize), "%.2f GB", (double)((double)log->ifile.iBt.downloadedsize / 1073741824.00));

			snprintf(s_upspeed, sizeof(s_upspeed), "%.2f KBps", log->ifile.iBt.uploadSpeed);
			snprintf(s_downspeed, sizeof(s_downspeed), "%.2f KBps", log->ifile.iBt.downloadSpeed);
			snprintf(s_errorstring, sizeof(s_errorstring), "%s", log->ifile.iBt.errorString);
			snprintf(s_ratio, sizeof(s_ratio), "%.2f", log->ifile.iBt.ratio);
			snprintf(s_uppeer, sizeof(s_uppeer), "%d", log->ifile.iBt.uploadpeers);
			snprintf(s_downpeer, sizeof(s_downpeer), "%d", log->ifile.iBt.downloadpeers);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", s_progress, s_filesize, s_status, s_availability, s_downsize, s_upsize, s_downspeed, s_upspeed, s_errorstring, s_ratio, s_uppeer, s_downpeer);
		}

		else if(!strcmp(logtab,"2"))
		{
			int file_num, id,i;
			id = log->ifile.iBt.id;
			file_num = log->ifile.iBt.trackCount;
			if(file_num > 0)
			{
				char lastAnnounceResult[file_num][128];
				char lastAnnounceStartTime[file_num][64];
				char nextAnnounceTime[file_num][64];
				char lastScrapeTime[file_num][64];
				char seederCount[file_num][8];
				char leecherCount[file_num][8];
				char downloadCount[file_num][8];
				char tracker[16];
				snprintf(tracker, sizeof(tracker), "tracker_%d", id);
				Tracker_struct tracker_files[file_num];
				read_tracker(tracker_files, tracker, file_num);
				for(i=0; i<file_num; i++)
				{
					snprintf(lastAnnounceResult[i], sizeof(lastAnnounceResult[i]), "lastAnnounceResult: %s", tracker_files[i].lastAnnounceResult);
					snprintf(lastAnnounceStartTime[i], sizeof(lastAnnounceStartTime[i]), "lastAnnounceStartTime: %s", ctime(&tracker_files[i].lastAnnounceStartTime));
					snprintf(nextAnnounceTime[i], sizeof(nextAnnounceTime[i]), "nextAnnounceTime: %s", ctime(&tracker_files[i].nextAnnounceTime));
					snprintf(lastScrapeTime[i], sizeof(lastScrapeTime[i]), "lastScrapeTime: %s", ctime(&tracker_files[i].lastScrapeTime));
					snprintf(seederCount[i], sizeof(seederCount[i]), "seederCount: %d", tracker_files[i].seederCount);
					snprintf(leecherCount[i], sizeof(leecherCount[i]), "leecherCount: %d", tracker_files[i].leecherCount);
					snprintf(downloadCount[i], sizeof(downloadCount[i]), "downloadCount: %d", tracker_files[i].downloadCount);

					delet(lastAnnounceStartTime[i],10);
					delet(nextAnnounceTime[i],10);
					delet(lastScrapeTime[i],10);

					if(i == 0)
						printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", lastAnnounceResult[i], lastAnnounceStartTime[i], nextAnnounceTime[i], lastScrapeTime[i],  seederCount[i], leecherCount[i], downloadCount[i]);
					else
						printf(",[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", lastAnnounceResult[i], lastAnnounceStartTime[i], nextAnnounceTime[i], lastScrapeTime[i],  seederCount[i], leecherCount[i], downloadCount[i]);
				}
			}

		}

		else if(!strcmp(logtab,"3"))
		{

			int file_num;
			file_num = log->ifile.iBt.fileCount;

			char file_name[file_num][512];
			char file_process[file_num][16];
			char file_total[file_num][16];
			char file_comp[file_num][16];
			File_detail files[file_num];

			memset(files, '\0', sizeof(files));

			read_file(files, logid, file_num);
			int x;
			for(x = 0; x < file_num;x++)
			{
				if(files[x].filecompletedsize < 1024)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%d B", (int)files[x].filecompletedsize);
				else if(files[x].filecompletedsize < 1048576)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%d KB", (int)(files[x].filecompletedsize / 1024));
				else if(files[x].filecompletedsize < 1073741824)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%.2f MB", (double)((double)files[x].filecompletedsize / 1048576.00));
				else
					snprintf(file_comp[x], sizeof(file_comp[x]), "%.2f GB", (double)((double)files[x].filecompletedsize / 1073741824.00));

				if(files[x].filetotalsize < 1024)
					snprintf(file_total[x], sizeof(file_total[x]), "%d B", (int)files[x].filetotalsize);
				else if(files[x].filetotalsize < 1048576)
					snprintf(file_total[x], sizeof(file_total[x]), "%d KB", (int)(files[x].filetotalsize / 1024));
				else if(files[x].filetotalsize < 1073741824)
					snprintf(file_total[x], sizeof(file_total[x]), "%.2f MB", (double)((double)files[x].filetotalsize / 1048576.00));
				else
					snprintf(file_total[x], sizeof(file_total[x]), "%.2f GB", (double)((double)files[x].filetotalsize / 1073741824.00));

				snprintf(file_name[x], sizeof(file_name[x]), "%s", files[x].filename);
				snprintf(file_process[x], sizeof(file_process[x]), "%.2f %s", files[x].progress*100, per);

				if(x == 0)
					printf("[\"%s\",\"%s\",\"%s\",\"%s\"]", file_name[x], file_process[x], file_comp[x], file_total[x]);
				else
					printf(",[\"%s\",\"%s\",\"%s\",\"%s\"]", file_name[x], file_process[x], file_comp[x], file_total[x]);
			}

		}

	}
		break;
	case HTTP:
	{
		if(!strcmp(logtab,"0"))
		{
			char s_destination[256];
			char s_url[128];

			memset(s_url, '\0', sizeof(s_url));
			memset(s_destination, '\0', sizeof(s_destination));

			snprintf(s_destination, sizeof(s_destination), "%s", log->ifile.iHttp.Destination);

			if(log->filesize < 1024)
				snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)log->filesize);
			else if(log->filesize < 1048576)
				snprintf(s_filesize, sizeof(s_filesize), "%d MB", (int)(log->filesize / 1024));
			else
				snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1048576.00));

			snprintf(s_dateCreated, sizeof(s_dateCreated), "%s", ctime(&log->ifile.iHttp.Created_time));
			delet(s_dateCreated,10);

			snprintf(s_url, sizeof(s_url), "%s", log->ifile.iHttp.URL);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\"]", s_destination, s_filesize, s_dateCreated, s_url);
		}

		else if(!strcmp(logtab,"1"))
		{
			char  s_starttime[50];
			char  s_lefttime[15];
			memset(s_starttime, '\0', sizeof(s_starttime));
			memset(s_lefttime, '\0', sizeof(s_lefttime));


			switch(log->status)
			{
			case S_INITIAL:
				snprintf(s_status, sizeof(s_status), "Initialing");
				break;
			case S_PROCESSING:
				snprintf(s_status, sizeof(s_status),"Downloading");
				break;
			case S_PAUSED:
				snprintf(s_status, sizeof(s_status),"Paused");
				break;
			case S_COMPLETED:
				snprintf(s_status, sizeof(s_status), "Finished");
				break;
			case S_SEEDING:
				snprintf(s_status, sizeof(s_status), "Seeding");
				break;
			case S_DEAD_OF_DELETED:
				snprintf(s_status, sizeof(s_status), "Stop");
				break;
			case S_DEAD_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Error");
				break;
			case S_TYPE_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Vfat4G");
				break;
			case S_HASH:
				snprintf(s_status, sizeof(s_status), "Hashing");
				break;
			case S_DISKFULL:
				snprintf(s_status, sizeof(s_status), "Diskfull");
				break;
			default:
				snprintf(s_status, sizeof(s_status), "Initialing");
			}

			uint64_t downsize;
			downsize = (int)((log->progress) * (log->filesize));

			if(downsize < 1024)
				snprintf(s_downsize, sizeof(s_downsize), "%d KB", (int)downsize);
			else if(downsize < 1048576)
				snprintf(s_downsize, sizeof(s_downsize), "%d MB", (int)(downsize / 1024));
			else
				snprintf(s_downsize, sizeof(s_downsize), "%.2f GB", (double)((double)downsize / 1048576.00));

			snprintf(s_progress, sizeof(s_progress), "%.2f %s ", log->progress*100,per);
			snprintf(s_downspeed, sizeof(s_downspeed), "%.2f KBps", log->ifile.iHttp.rate);
			snprintf(s_starttime, sizeof(s_starttime), "%s", ctime(&log->ifile.iHttp.Created_time));
			snprintf(s_lefttime, sizeof(s_lefttime), "%s", log->ifile.iHttp.Time_Left);

			delet(s_starttime,10);
			delet(s_lefttime,10);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", s_status, s_downsize, s_progress, s_downspeed, s_starttime, s_lefttime);
		}

	}
		break;
	case FTP:
	{
		if(!strcmp(logtab,"0"))
		{
			char s_destination[256];
			char s_url[128];

			memset(s_url, '\0', sizeof(s_url));
			memset(s_destination, '\0', sizeof(s_destination));

			snprintf(s_destination, sizeof(s_destination), "%s", log->ifile.iFtp.Destination);

			if(log->filesize < 1024)
				snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)log->filesize);
			else if(log->filesize < 1048576)
				snprintf(s_filesize, sizeof(s_filesize), "%d MB", (int)(log->filesize / 1024));
			else
				snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1048576.00));

			snprintf(s_dateCreated, sizeof(s_dateCreated), "%s", ctime(&log->ifile.iFtp.Created_time));
			delet(s_dateCreated,10);
			snprintf(s_url, sizeof(s_url), "%s", log->ifile.iFtp.URL);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\"]", s_destination, s_filesize, s_dateCreated, s_url);
		}

		else if(!strcmp(logtab,"1"))
		{
			char  s_starttime[50];
			char  s_lefttime[15];
			memset(s_starttime, '\0', sizeof(s_starttime));
			memset(s_lefttime, '\0', sizeof(s_lefttime));

			switch(log->status)
			{
			case S_INITIAL:
				snprintf(s_status, sizeof(s_status), "Initialing");
				break;
			case S_PROCESSING:
				snprintf(s_status, sizeof(s_status),"Downloading");
				break;
			case S_PAUSED:
				snprintf(s_status, sizeof(s_status),"Paused");
				break;
			case S_COMPLETED:
				snprintf(s_status, sizeof(s_status), "Finished");
				break;
			case S_SEEDING:
				snprintf(s_status, sizeof(s_status), "Seeding");
				break;
			case S_DEAD_OF_DELETED:
				snprintf(s_status, sizeof(s_status), "Stop");
				break;
			case S_DEAD_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Error");
				break;
			case S_TYPE_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Vfat4G");
				break;
			case S_HASH:
				snprintf(s_status, sizeof(s_status), "Hashing");
				break;
			case S_DISKFULL:
				snprintf(s_status, sizeof(s_status), "Diskfull");
				break;
			default:
				snprintf(s_status, sizeof(s_status), "Initialing");
			}

			uint64_t downsize;
			downsize = (int)((log->progress) * (log->filesize));

			if(downsize < 1024)
				snprintf(s_downsize, sizeof(s_downsize), "%d KB", (int)downsize);
			else if(downsize < 1048576)
				snprintf(s_downsize, sizeof(s_downsize), "%d MB", (int)(downsize / 1024));
			else
				snprintf(s_downsize, sizeof(s_downsize), "%.2f GB", (double)((double)downsize / 1048576.00));

			snprintf(s_progress, sizeof(s_progress), "%.2f %s ", log->progress*100,per);
			snprintf(s_downspeed, sizeof(s_downspeed), "%.2f KBps", log->ifile.iFtp.rate);
			snprintf(s_starttime, sizeof(s_starttime), "%s", ctime(&log->ifile.iFtp.Created_time));
			delet(s_starttime,10);
			snprintf(s_lefttime, sizeof(s_lefttime), "%s", log->ifile.iFtp.Time_Left);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", s_status, s_downsize, s_progress, s_downspeed, s_starttime, s_lefttime);
		}
	}
		break;
	case NZB:
	{
		if(!strcmp(logtab,"0"))
		{
			char s_destination[256];
			memset(s_destination, '\0', sizeof(s_destination));

			struct disk_info *disks_tmp;
			disks_tmp = follow_disk_info_start;
			while(disks_tmp!=NULL)
			{
				if(disks_tmp->mountpath!=NULL)
				{
					if((strncmp(disks_tmp->serialnum,log->ifile.iNzb.serial,strlen(disks_tmp->serialnum))==0)&&(strncmp(disks_tmp->product,log->ifile.iNzb.pid,strlen(disks_tmp->product))==0)&&(strncmp(disks_tmp->vendor,log->ifile.iNzb.vid,strlen(disks_tmp->vendor))==0)&&(disks_tmp->partitionport==log->ifile.iNzb.partitionnum))
						break;
					else
						disks_tmp=disks_tmp->next;
				}
				else
					disks_tmp=disks_tmp->next;
			}
			if(disks_tmp)
			{


				char *p;
				if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iNzb.Destination,"not_found",9)!=0))
				{
					//fprintf(stderr,"\nrecover base_dir11\n");
					p=my_nstrchr('/',log->ifile.iNzb.Destination,4);
					char downloadtmp[strlen(p)+1];
					memset(downloadtmp,'\0',sizeof(downloadtmp));
					strncpy(downloadtmp,p,strlen(p));
					memset(log->ifile.iNzb.Destination,'\0',sizeof(log->ifile.iNzb.Destination));
					strcpy(log->ifile.iNzb.Destination,disks_tmp->mountpath);
					strcat(log->ifile.iNzb.Destination,downloadtmp);
				}
				else if((disks_tmp)&&(disks_tmp->mountpath)&&(strncmp(log->ifile.iNzb.Destination,"not_found",9)==0))
				{
					//fprintf(stderr,"\nrecover base_dir22\n");
					p=log->ifile.iNzb.Destination;
					p=p+9;
					if(p){
						char downloadtmp[strlen(p)+1];
						memset(downloadtmp,'\0',sizeof(downloadtmp));
						strncpy(downloadtmp,p,strlen(p));
						memset(log->ifile.iNzb.Destination,'\0',sizeof(log->ifile.iNzb.Destination));
						strcpy(log->ifile.iNzb.Destination,disks_tmp->mountpath);
						strcat(log->ifile.iNzb.Destination,downloadtmp);
					}
					else{
						memset(log->ifile.iNzb.Destination,'\0',sizeof(log->ifile.iNzb.Destination));
						strcpy(log->ifile.iNzb.Destination,disks_tmp->mountpath);
					}
				}
			}
			else
			{
				//fprintf(stderr,"\nthe desired download folder is not mount\n");
				char defaultdldir[100];
				memset(defaultdldir,'\0',sizeof(defaultdldir));
				snprintf(defaultdldir, sizeof(defaultdldir), "%s/Download2/Complete",Base_dir);
				memset(log->ifile.iNzb.Destination,'\0',sizeof(log->ifile.iNzb.Destination));
				strcpy(log->ifile.iNzb.Destination,defaultdldir);

			}

			if(log->filesize < 1024)
				snprintf(s_filesize, sizeof(s_filesize), "%d B", (int)log->filesize);
			else if(log->filesize < 1048576)
				snprintf(s_filesize, sizeof(s_filesize), "%d KB", (int)(log->filesize / 1024));
			else if(log->filesize < 1073741824)
				snprintf(s_filesize, sizeof(s_filesize), "%.2f MB", (double)((double)log->filesize / 1048576.00));
			else
				snprintf(s_filesize, sizeof(s_filesize), "%.2f GB", (double)((double)log->filesize / 1073741824.00));

			snprintf(s_dateCreated, sizeof(s_dateCreated), "%s", ctime(&log->ifile.iNzb.Created_time));
			delet(s_dateCreated,10);

			printf("[\"%s\",\"%s\",\"%s\"]", log->ifile.iNzb.Destination, s_filesize, s_dateCreated);
		}

		else if(!strcmp(logtab,"1"))
		{
			char  s_starttime[50];
			char  s_lefttime[15];
			memset(s_starttime, '\0', sizeof(s_starttime));
			memset(s_lefttime, '\0', sizeof(s_lefttime));

			switch(log->status)
			{
			case S_INITIAL:
				snprintf(s_status, sizeof(s_status), "Initialing");
				break;
			case S_PROCESSING:
				snprintf(s_status, sizeof(s_status),"Downloading");
				break;
			case S_PAUSED:
				snprintf(s_status, sizeof(s_status),"Paused");
				break;
			case S_COMPLETED:
				snprintf(s_status, sizeof(s_status), "Finished");
				break;
			case S_SEEDING:
				snprintf(s_status, sizeof(s_status), "Seeding");
				break;
			case S_DEAD_OF_DELETED:
				snprintf(s_status, sizeof(s_status), "Stop");
				break;
			case S_DEAD_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Error");
				break;
			case S_TYPE_OF_ERROR:
				snprintf(s_status, sizeof(s_status), "Vfat4G");
				break;
			case S_HASH:
				snprintf(s_status, sizeof(s_status), "Hashing");
				break;
			case S_DISKFULL:
				snprintf(s_status, sizeof(s_status), "Diskfull");
				break;
			default:
				snprintf(s_status, sizeof(s_status), "Initialing");
			}

			uint64_t downsize;
			downsize = (int)((log->progress) * (log->filesize));

			if(downsize < 1024)
				snprintf(s_downsize, sizeof(s_downsize), "%d B", (int)downsize);
			else if(downsize < 1048576)
				snprintf(s_downsize, sizeof(s_downsize), "%d KB", (int)(downsize / 1024));
			else if(log->filesize < 1073741824)
				snprintf(s_downsize, sizeof(s_downsize), "%.2f MB", (double)((double)downsize / 1048576.00));
			else
				snprintf(s_downsize, sizeof(s_downsize), "%.2f GB", (double)((double)downsize / 1073741824.00));

			snprintf(s_progress, sizeof(s_progress), "%.2f %s ", log->progress*100,per);
			snprintf(s_downspeed, sizeof(s_downspeed), "%.2f KBps", log->ifile.iNzb.rate);
			snprintf(s_starttime, sizeof(s_starttime), "%s", ctime(&log->ifile.iNzb.Start_Time));
			delet(s_starttime,10);
			snprintf(s_lefttime, sizeof(s_lefttime), "%s", log->ifile.iNzb.Time_Left);

			printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", s_status, s_downsize, s_progress, s_downspeed,s_starttime,s_lefttime);
		}
		else if(!strcmp(logtab,"2"))
		{
			int i= 0;

			FILE *fp;
			char output[1024],nzblogdir[256], ch;
			memset(output, '\0', sizeof(output));
			memset(nzblogdir, '\0', sizeof(nzblogdir));
			//snprintf(nzblogdir, "%s/Download2/.logs/nzbget.log", getbasepath());
			snprintf(nzblogdir, sizeof(nzblogdir),"%s/Download2/.logs/nzbget.log", Base_dir);
			fp = fopen(nzblogdir,"r");

			if( fp != NULL )
			{
				while((ch = getc(fp)) !=EOF )
				{
					if( ch != '\n')
					{
						output[i]=ch;
						i++;
					}
					else
					{
						i = 0;
						if(first_log)
						{
							first_log = 0;
							printf("[\"%s\"]", output);
						}
						else
							printf(",[\"%s\"]", output);
					}
				}
			}

			fclose(fp);

		}
		else if(!strcmp(logtab,"3"))
		{
			int file_num;
			file_num = log->ifile.iNzb.fileCount;

			char file_name[file_num][512];
			char file_process[file_num][16];
			char file_total[file_num][16];
			char file_comp[file_num][16];

			File_detail files[file_num];
			memset(files, '\0', sizeof(files));

			read_file(files, logid, file_num);

			int x;
			for(x = 0; x < file_num;x++)
			{
				if(files[x].filecompletedsize < 1024)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%d B", (int)files[x].filecompletedsize);
				else if(files[x].filecompletedsize < 1048576)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%d KB", (int)(files[x].filecompletedsize / 1024));
				else if(files[x].filecompletedsize < 1073741824)
					snprintf(file_comp[x], sizeof(file_comp[x]), "%.2f MB", (double)((double)files[x].filecompletedsize / 1048576.00));
				else
					snprintf(file_comp[x], sizeof(file_comp[x]), "%.2f GB", (double)((double)files[x].filecompletedsize / 1073741824.00));


				if(files[x].filetotalsize < 1024)
					snprintf(file_total[x], sizeof(file_total[x]), "%d B", (int)files[x].filetotalsize);
				else if(files[x].filetotalsize < 1048576)
					snprintf(file_total[x], sizeof(file_total[x]), "%d KB", (int)(files[x].filetotalsize / 1024));
				else if(files[x].filetotalsize < 1073741824)
					snprintf(file_total[x], sizeof(file_total[x]), "%.2f MB", (double)((double)files[x].filetotalsize / 1048576.00));
				else
					snprintf(file_total[x], sizeof(file_total[x]), "%.2f GB", (double)((double)files[x].filetotalsize / 1073741824.00));

				snprintf(file_name[x], sizeof(file_name[x]), "%s", files[x].filename);
				snprintf(file_process[x], sizeof(file_process[x]), "%.2f %s", files[x].progress*100, per);

				if(x == 0)
					printf("[\"%s\",\"%s\",\"%s\",\"%s\"]", file_name[x], file_process[x], file_comp[x], file_total[x]);
				else
					printf(",[\"%s\",\"%s\",\"%s\",\"%s\"]", file_name[x], file_process[x], file_comp[x], file_total[x]);
			}
		}
	}
		break;

	default:
		break;
	}
}

int dm_print_single_status(char* cmd,char* task_id,char *type){
    DIR  *log_dir;
    struct dirent  *log_ptr;
    Log_struc  slog;
    char *pid;
    int  i, log_index;
    struct Lognote  *au;

    if((log_dir=opendir(Log_dir)) == NULL)
    {
        return -1;
    }

    for(i=0; i<MAX_SHOWS; i++)
    {
        memset(logs_id[i].id, '\0', HASHLEN);
        logs_id[i].status = 0;
    }

    log_index = 0;
    remain_lists = 0;
    first_log = 1;

    memset(&slog, '\0', sizeof(slog));

    pid= task_id;

    if(read_log(&slog, pid) > 0)
    {
        print_single_log(&slog, pid, type);
    }
    closedir(log_dir);
    return 0;
}

void print_error_log(const char *logname, char*cmd)
{
    FILE *fp;
    //int m = 0, n = 0;
    //char ch, errorname[64],error_log[5][256];
    char errorname[64],error_log[1024],error_log_tmp[1024],error_log_id[16];
    memset(errorname, '\0', sizeof(errorname));
    memset(error_log, '\0', sizeof(error_log));
    memset(error_log_tmp, '\0', sizeof(error_log_tmp));
    memset(error_log_id, '\0', sizeof(error_log_id));

    snprintf(errorname, sizeof(errorname), "%s/%s", Log_dir,logname);

	if (fp=fopen(errorname, "r"))
	{	
		int i=0;
		while (fgets(error_log_tmp,1024,fp)!=NULL)
		{
			if(error_log_tmp[strlen(error_log_tmp)-1]=='\n')
			{
				error_log_tmp[strlen(error_log_tmp)-1]='\0';
			}
			if(i==0){
			 snprintf(error_log, sizeof(error_log), "%s", error_log_tmp);
		         i++;
			}
			else{
				snprintf(error_log_id, sizeof(error_log_id), "%s", error_log_tmp);
			}
		}
		fclose(fp);
	}

    if(!strcmp(cmd,"All"))
    {
        if(first_log)
        {
            printf( "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logname, error_log,"0" , " ", "Error", "BT", " ", "0.00 KBps", "0 KBps", 0, error_log_id,"0","not_found");
            first_log = 0;
        }
        else
            printf( ",[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%d\",\"%s\",\"%s\",\"%s\"]\n", logname, error_log,"0", " ", "Error", "BT", " ", "0.00 KBps", "0 KBps", 0, error_log_id,"0","not_found");
    }
}

int dm_print_status(char* cmd)
{
    DIR  *log_dir;
    struct dirent  *log_ptr;
    Log_struc  slog;
    char *pid;
    int  i, log_index;
    struct Lognote *au = (struct Lognote*)0;
    int queue_id = 0;

    head = (struct Lognote *) malloc (sizeof(struct Lognote));
    memset(head, 0, sizeof(struct Lognote));

    // read .logs
    if((log_dir=opendir(Log_dir)) == NULL)
    {
        return -1;
    }

    for(i=0; i<MAX_SHOWS; i++)
    {
        memset(logs_id[i].id, '\0', HASHLEN);
        logs_id[i].status = 0;
    }

    log_index = 0;
    remain_lists = 0;
    first_log = 1;

    // print queuing jobs
    memset(jqs_file, '\0', MAX_NAMELEN);
    snprintf(jqs_file, sizeof(jqs_file), "%s/dm.jqs", Log_dir);

    if((!jqs_head) && (!jqs_rear))
    {
        jqs_head = jqs_rear = (struct Lognote*)0;
        read_queue_jobs(head);
    }
    else
    {
        free_jobs_queue();
        jqs_head = (struct Lognote*)0;
        jqs_rear = (struct Lognote*)0;
        read_queue_jobs(head);
    }

    while((log_ptr=readdir(log_dir)) != NULL)
    {

        if(strstr(log_ptr->d_name, "error_") != NULL)
        {
            print_error_log(log_ptr->d_name,cmd);
        }
        else if((pid = getlogid(log_ptr->d_name)) != NULL)
        {
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0)
            {
                if(log_index < MAX_SHOWS)
                {
                    strncpy(logs_id[log_index].id, pid, strlen(pid));
                    print_log(&slog, pid, log_index, cmd);
                    log_index++;
                }
                else
                    remain_lists++;
            }
        }
    }
    closedir(log_dir);

    if(!strcmp(cmd,"All")){
        for(au=head; au; au=au->next){

            if(queue_id >= MAX_QUES)
                break;
            if(strcmp(au->url,"")){

                print_queue_jobs(au, au->id);
                queue_id++;
            }
        }
    }
    return 0;
}

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

        old_disk = follow_disk;
        follow_disk = follow_disk->next;
        free(old_disk);
    }
}

int init_diskinfo_struct()
{
    int len;
    FILE *fp;
    while(access("/tmp/getdiskinfo_lock",F_OK) == 0)
    {
        usleep(50);
    }

    //unlink("/tmp/usbinfo_lock");
    //system("/tmp/APPS/DM2/Script/dm2_usbget start&");
    while(access("/tmp/usbinfo_lock",0)!=0){
            //sleep(1);
    }

    fp = fopen("/tmp/getdiskinfo_lock","w");
    fclose(fp);

    if(access("/tmp/usbinfo",0)==0)
    {
        fp =fopen("/tmp/usbinfo","r");
        if(fp)
        {
            fseek(fp,0,SEEK_END);
            len = ftell(fp);
            fseek(fp,0,SEEK_SET);
        }
        else
        {
            unlink("/tmp/getdiskinfo_lock");
            return -1;
        }
    }
    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    fread(buf,1,len,fp);
    fclose(fp);

    unlink("/tmp/getdiskinfo_lock");

    if(initial_disk_data(&follow_disk_tmp) == NULL){
        fprintf(stderr,"No memory!!(follow_disk_info)\n");
        return -1;
    }
    if(initial_disk_data(&follow_disk_info_start) == NULL){
        fprintf(stderr,"No memory!!(follow_disk_info)\n");
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
}

int main(void){
    char *data;
    printf("ContentType:text/html\r\n");
    printf("Cache-Control:private,max-age=0;\r\n");
    printf("\r\n");
    data = getenv("QUERY_STRING");
    init_cgi(data);	// there wasn't '?' in the head.

    char *value;
    char *url;
    char *type;
    char *current_url;
    char *next_url;
    char *next_host;
    char *script;

    url         = websGetVar(wp,    "usb_dm_url",       "");
    type        = websGetVar(wp,    "download_type",    "");
    value       = websGetVar(data,  "action_mode",      "");
    next_host   = websGetVar(wp,    "next_host",        "");
    current_url = websGetVar(wp,    "current_page",     "");
    next_url    = websGetVar(wp,    "next_page",        "");
    script      = websGetVar(wp,    "action_script",    "");

    char task_id[20];
    memset(task_id, '\0', sizeof(task_id));

    init_path();
    init_diskinfo_struct();

    if(!strcmp(value,"show_single_task")){
        snprintf(task_id, sizeof(task_id), "%s",websGetVar(data,"task_id", ""));
        dm_print_single_status("single",task_id,type);
    }
    else{
        //fprintf(stderr,"\nnot single task\n");show_s
        dm_print_status(value);
    }

    fflush(stdout);
    free_disk_struc(&follow_disk_info_start);
    return 0;
}
