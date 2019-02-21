
#include <assert.h>
#include <errno.h>
#include <string.h> /* memcpy */
#include <limits.h> /* INT_MAX */

#include <sys/types.h> /* open */
#include <sys/stat.h>  /* open */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* close */

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h> /* TODO: eventually remove this */

#include "transmission.h"
//#include "bencode.h"
#include "crypto.h"
#include "fdlimit.h"
#include "list.h"
#include "net.h"
#include "platform.h"
#include "ptrarray.h"
#include "rpcimpl.h"
#include "socket-server.h"
#include "session.h"
#include "trevent.h"
#include "utils.h"
#include "web.h"
#include "torrent.h"
#include "dm_log.h"
#include "rpc-server.h"   //leo added

#define MY_NAME "SOCKET Server"
#define MY_REALM "Transmission"

#ifdef WIN32
#define strncasecmp _strnicmp
#endif
//20150907 leo added  {
struct tr_rpc_server
{
    bool               isEnabled;
    bool               isPasswordEnabled;
    bool               isWhitelistEnabled;
    tr_port            port;
    char             * url;
    struct in_addr     bindAddress;
    struct evhttp    * httpd;
    tr_session       * session;
    char             * username;
    char             * password;
    char             * whitelistStr;
    tr_list          * whitelist;

    char             * sessionId;
    time_t             sessionIdExpiresAt;

#ifdef HAVE_ZLIB
    bool               isStreamInitialized;
    z_stream           stream;
#endif
};

//20150907 leo added  }
static void event_callback( int, short, void* );
void canceltask(char *, tr_session *);
void starttask(char *, tr_session *);
void pausetask(char *, tr_session *);
void pause_alltask(tr_session *);
void start_alltask(tr_session *);
void clean_completedtask(tr_session *);
void addtask_url( char *, tr_session *);
void addtask_path( char *, tr_session *);
void apply_config( char *, tr_session *);

char down_url[256];

void tr_tcpInit( tr_session  * session )
{
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;

    session->tcp_port = 9092;      //setting socket port is "9092";
    session->tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(session->tcp_port);


    setsockopt(session->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));

    if (bind(session->tcp_socket, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
    {
        perror("call to bind error!");
        exit(1);
    }

    //if (listen(session->tcp_socket, 1) < 0)
    if (listen(session->tcp_socket, 15) < 0) //2012.07.31 magic added for more scoket
    {
        perror("call to listen error!");
        exit(1);
    }

    session->tcp_event = event_new(session->event_base, session->tcp_socket, EV_READ | EV_PERSIST, event_callback, session);

    event_add(session->tcp_event, NULL);
}


void tr_tcpUninit(tr_session *session)
{
    if(session->tcp_socket >= 0) {
        tr_netCloseSocket( session->tcp_socket );
        session->tcp_socket = -1;
    }

    if(session->tcp_event) {
        event_free(session->tcp_event);
        session->tcp_event = NULL;
    }
}


static void gotMetadataFromURL( tr_session       * session UNUSED,
                    		bool               did_connect UNUSED,
                    		bool               did_timeout UNUSED,
                            long               response_code,
                            const void       * response,
                            size_t             response_byte_count,
                            void             * user_data )
{
    tr_ctor * ctor = user_data;
    if( response_code==200 || response_code==221 )
    {
        int err = 0;
        tr_torrent * tor = NULL;
        tr_ctorSetMetainfo( ctor, response, response_byte_count );
        tor = tr_torrentNew(ctor, &err, NULL );

        if (err == 0){
                //fprintf(stderr,"\ngotMetadataFromURL call torrentlog\n");
            tr_torrentlog(tor);
		}
        else
        {
            FILE *fp = NULL;
            int i = 0;
            char error_log[128];
            char errorname[16];
            memset(error_log, '\0', sizeof(error_log));
            memset(errorname, '\0', sizeof(errorname));

            do
            {
                i++;
                sprintf(errorname, "error_%d", i);
                sprintf(error_log, "%s.logs/%s", base_path,errorname);

            }
            while(access(error_log,0) == 0);

            fp = fopen(error_log,"w+");
            fprintf(fp,"%s\n%d",down_url,err);
            fclose(fp);

        }
    }
    else
    {
        FILE *fp = NULL;
        int i = 0;
        char error_log[128];
        char errorname[16];
        memset(error_log, '\0', sizeof(error_log));
        memset(errorname, '\0', sizeof(errorname));

        do
        {
            i++;
            sprintf(errorname, "error_%d", i);
            sprintf(error_log, "%s.logs/%s", base_path,errorname);
        
}
        while(access(error_log,0) == 0);

        fp = fopen(error_log,"w+");
        fprintf(fp,"%s\n%ld",down_url,response_code);
        fclose(fp);
    }
}



void canceltask(char *arg, tr_session *session)
{
    int id = atoi(arg);
    tr_torrent * tor = NULL;

    /*if( ( tor = tr_torrentFindFromId( session, id ) ) )
            tr_torrentRemove( tor, TR_RPC_TORRENT_REMOVING, NULL );*/
    //2012.01.30 magic {
    if( ( tor = tr_torrentFindFromId( session, id ) ) )
    {
			const tr_stat * st = tr_torrentStat(tor);
                        if( st->activity == TR_STATUS_SEED || st->percentDone==1)
			{
				tr_torrentRemove( tor, FALSE, NULL );
			}
			else{
				tr_torrentRemove( tor, TR_RPC_TORRENT_REMOVING, NULL );
			}

        }
    //2012.01.30 magic }
}

void starttask(char *arg, tr_session *session)
{
    int id = atoi(arg);
    tr_torrent * tor = NULL;

    if( ( tor = tr_torrentFindFromId( session, id ) ) )
        if( !tor->isRunning )
            tr_torrentStart( tor );
}

void pausetask(char *arg, tr_session *session)
{
    int id = atoi(arg);
    tr_torrent * tor = NULL;

    if( ( tor = tr_torrentFindFromId( session, id ) ) ){
        if( tor->isRunning ){
            tor->isStopping = TRUE;
	}
        else if( tor->isFinished )//eric added 2013.5.21
	{
            tor->isFinished = FALSE;
	}
    }
}

void pause_alltask(tr_session *session)
{
    tr_torrent * tor = NULL;
    while(( tor = tr_torrentNext( session, tor ))){
        if( tor->isRunning )
            tor->isStopping = TRUE;
        else if( tor->isFinished )//eric added 2013.5.21
            tor->isFinished = FALSE;
    }
}

void start_alltask(tr_session *session)
{
    tr_torrent * tor = NULL;
    while(( tor = tr_torrentNext( session, tor )))
    {
        //if( (!tor->isRunning) && (tor->stats.percentDone != (double)1)  )
        if(!tor->isRunning)
        {
            if(tor->stats.percentDone != (double)1){
                tr_torrentStart( tor );
            }
            else{
                if(misc_seeding_x==1){
                    tr_torrentStart( tor );
                }
            }
        }
    }
}

void clean_completedtask(tr_session *session)
{
    tr_torrent * tor = NULL;
    while(( tor = tr_torrentNext( session, tor )))
    {
        const tr_stat * st = tr_torrentStat(tor);
        if( st->activity == TR_STATUS_SEED )
            tr_torrentRemove( tor, FALSE, NULL );
        else if( (st->activity == TR_STATUS_STOPPED) && (tor->isFinished == TRUE) )//eric added 2013.5.21
            tr_torrentRemove( tor, FALSE, NULL );
        else if( (st->activity == TR_STATUS_STOPPED) && (tor->stats.percentDone == (double)1) )
            tr_torrentRemove( tor, FALSE, NULL);
    }
}

void addtask_url( char *arg, tr_session *session)
{
    tr_ctor    * ctor = tr_ctorNew( session );

    if(!strncmp( arg, "ftp://", 6 ) ||
       !strncmp( arg, "http://", 7 ) ||
       !strncmp( arg, "https://", 8 ))
    {
        memset(down_url, '\0', sizeof(down_url));
        sprintf(down_url, "%s", arg);

        //tr_webRun( session, arg, NULL, gotMetadataFromURL, ctor );
        //fprintf(stderr,"\nsession->configDir=%s\n",session->configDir);
        tr_webRun( session, arg, gotMetadataFromURL, ctor );
    }
    else
    {
        int err = 0;
        tr_torrent * tor= NULL;
        char * fname = tr_strstrip( tr_strdup( arg ) );

        if( !strncmp( fname, "magnet:?", 8 ) )
        {
            tr_ctorSetMetainfoFromMagnetLink( ctor, fname );
        }
        else
        {
            tr_ctorSetMetainfoFromFile( ctor, fname );
        }

        tr_free( fname );

        tor = tr_torrentNew( ctor, &err, NULL );
        if (err == 0){
            tr_torrentlog(tor);
	}
        else
        {
            FILE *fp;
            int len, i = 0;
            char *tmp = NULL, error_log[128], value[512], errorname[16];

            memset(error_log, '\0', sizeof(error_log));
            memset(value, '\0', sizeof(value));
            memset(errorname, '\0', sizeof(errorname));
            tmp=strstr(arg,"&");

            do
            {
                i++;
                sprintf(errorname, "error_%d", i);
                sprintf(error_log, "%s.logs/%s", base_path,errorname);
                printf("error_log1: %s \n",error_log);

            }
            while(access(error_log,0) == 0);

            printf("error_log2: %s \n",error_log);

            fp = fopen(error_log,"w+");

            if( tmp == NULL)
                fprintf(fp,"%s\n%d",arg,err);
            else
            {
                len=strlen(arg) - strlen(tmp);
                strncpy(value,arg,len);

                fprintf(fp,"%s\n%d",value,err);
            }
            printf("value: %s \n",value);

            fclose(fp);
        }
    }
}

void addtask_path( char *arg, tr_session *session)
{
    int err = 0;
    char * fname = tr_strstrip( tr_strdup( arg ) );
    tr_torrent * tor = NULL;
    tr_ctor * ctor = tr_ctorNew( session );

    tr_ctorSetMetainfoFromFile( ctor, fname );
    tor = tr_torrentNew( ctor, &err, NULL );

    tr_free( fname );
    tr_ctorFree( ctor );

    if (err == 0){
        tr_torrentlog(tor);
		}
    else
    {
        FILE *fp;
        int i = 0;
        char error_log[128], errorname[16];
        memset(errorname, '\0', sizeof(errorname));
        memset(error_log, '\0', sizeof(error_log));

        do
        {
            i++;
            sprintf(errorname, "error_%d", i);
            sprintf(error_log, "%s.logs/%s", base_path,errorname);
        }
        while(access(error_log,0) == 0);

        fp = fopen(error_log,"w+");
        fprintf(fp,"%s\n%d",arg,err);
        fclose(fp);
    }
}

void addtask_path2( char *arg, tr_session *session)
{
    //fprintf(stderr,"\narg=%s\n",arg);
    int err = 0;
    char *p;
    int i;
    p = strstr(arg,"@");
    char *wantedid;
    wantedid = (char *)malloc(strlen(arg) - strlen(p) + 1);
    memset(wantedid, '\0', sizeof(wantedid));
    strncpy(wantedid, arg, strlen(arg) - strlen(p));
    if(p == NULL)
        return ;
    p = p + 1;
    //fprintf(stderr,"\np=%s\n",p);
    char * fname = tr_strstrip( tr_strdup( p ) );
    tr_torrent * tor = NULL;
    tr_ctor * ctor = tr_ctorNew( session );

    tr_ctorSetMetainfoFromFile( ctor, fname );
    tor = tr_torrentNew( ctor, &err, NULL );

    for(i = 0; i < tor->info.fileCount; i++)
    {
        setFileDND (tor, i, FALSE);
    }
        const char *split = ",";
        char *point;

        tr_torrentLock (tor);
        //fprintf(stderr,"\nwantedid=%s\n",wantedid);
        point = strtok(wantedid,split);
        //fprintf(stderr,"\np=%s\n",point);
        while(point != NULL)
        {
            if(atoi(point) < tor->info.fileCount)
                setFileDND (tor, atoi(point), TRUE);

            point = strtok(NULL,split);
        }
        tr_cpInvalidateDND (&tor->completion);
        tr_torrentUnlock (tor);

    tr_free( fname );
    tr_ctorFree( ctor );

    if (err == 0){
        tr_torrentlog(tor);
                }
    else
    {
        FILE *fp;
        int i = 0;
        char error_log[128], errorname[16];
        memset(errorname, '\0', sizeof(errorname));
        memset(error_log, '\0', sizeof(error_log));

        do
        {
            i++;
            sprintf(errorname, "error_%d", i);
            sprintf(error_log, "%s.logs/%s", base_path,errorname);
        }
        while(access(error_log,0) == 0);

        fp = fopen(error_log,"w+");
        fprintf(fp,"%s\n%d",p,err);
        fclose(fp);
    }
}

void getproduct(void)
{
    if (access("/tmp/asus_router.conf",0) == 0)
	{
        int fd, len, i=0;
		char ch, tmp[256], name[256], content[256] ,result[64];
		memset(tmp, 0, sizeof(tmp));
		memset(name, 0, sizeof(name));
		memset(content, 0, sizeof(content));
		memset(result, 0, sizeof(result));
	 	sprintf(result, "1");
		if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0)
		{
			fprintf(stderr,"\nread conf error!\n");
		} else {
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
	                if(!strcmp(name, "DEVICE_TYPE"))
	    			{
                       snprintf(disktype, sizeof(disktype), "%s", content);
                    }
			            continue;
			        }


			        memcpy(tmp+i, &ch, 1);
			        i++;
			    }
			    close(fd);
				return;
	      		}
		}
	    else
	        return;

}

void apply_config(char *arg, tr_session *session)
{
    int peer_port,max_torrent_peer,max_peer;
    int auth_type,en_dht,up_limit,up_rate,down_limit,down_rate,en_pex,len;
    char *tmp = NULL, *p = NULL, value[10];
    memset(value, '\0', sizeof(value));
    {
        p=strstr(arg,"peer_port=");
        p=p+10;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        value[len]= '\0';
        peer_port = atoi(value);
        printf("receive config peer_port: %d\n",peer_port);

        tr_sessionSetPeerPort( session, peer_port );
    }

    {

        p=strstr(arg,"auth_type=");
        p=p+10;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        auth_type = atoi(value);

        printf("receive config auth_type: %d\n",auth_type);

        if( auth_type == 2 )             //required
            tr_sessionSetEncryption( session, TR_ENCRYPTION_REQUIRED );
        else if( auth_type == 0 )        //tolerated
            tr_sessionSetEncryption( session, TR_CLEAR_PREFERRED );
        else  if( auth_type == 1 )                           //preferred
            tr_sessionSetEncryption( session, TR_ENCRYPTION_PREFERRED );
    }


    {
        p=strstr(arg,"max_peer=");
        p=p+9;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        max_peer = atoi(value);
        printf("receive config max_peer: %d\n",max_peer);

        tr_sessionSetPeerLimit( session, max_peer );
		max_peer_tmp = max_peer;
    }

    {
        p=strstr(arg,"max_torrent_peer=");
        p=p+17;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        max_torrent_peer = atoi(value);
        printf("receive config max_torrent_peer: %d\n",max_torrent_peer);

        //tr_sessionSetPeerLimitPerTorrent( session, max_torrent_peer );
                tr_torrent * tor = NULL;
                while(( tor = tr_torrentNext( session, tor )))
                {
                    tr_torrentSetPeerLimit (tor, max_torrent_peer);
                }
        max_torrent_peer_tmp = max_torrent_peer;
    }

    {
        p=strstr(arg,"en_dht=");
        p=p+7;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        en_dht = atoi(value);
        printf("receive config en_dht: %d\n",en_dht);

        if(en_dht == 0)             //Disable DHT
            tr_sessionSetDHTEnabled( session, FALSE);
        else
            tr_sessionSetDHTEnabled( session, TRUE);
    }


    {
        p=strstr(arg,"down_limit=");
        p=p+11;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        down_limit = atoi(value);
        printf("receive config down_limit: %d\n",down_limit);

        if(down_limit == 1)         //enable max download speed limit
        {
            p=strstr(arg,"down_rate=");
            p=p+10;
            tmp=strstr(p,"@");
            len=strlen(p) - strlen(tmp);
            strncpy(value,p,len);
            *(value+len)='\0';
            down_rate = atoi(value);
            printf("receive config down_rate: %d\n",down_rate);

            tr_sessionLimitSpeed( session, TR_DOWN, TRUE );
            tr_sessionSetSpeedLimit_KBps( session, TR_DOWN, down_rate );
        }
        else                        //Disable max download speed

            tr_sessionLimitSpeed( session, TR_DOWN, FALSE );
    }


    {
        p=strstr(arg,"up_limit=");
        p=p+9;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        up_limit = atoi(value);

        printf("receive config up_limit: %d\n",up_limit);

        if(up_limit == 1)           //enable max upload speed limit
        {
            p=strstr(arg,"up_rate=");
            p=p+8;
            tmp=strstr(p,"@");
            len=strlen(p) - strlen(tmp);
            strncpy(value,p,len);
            value[len]='\0';
            up_rate = atoi(value);
            printf("receive config up_rate: %d\n",up_rate);

            tr_sessionLimitSpeed( session, TR_UP, TRUE );
            tr_sessionSetSpeedLimit_KBps( session, TR_UP, up_rate );
            up_limit_enable = 1;
            up_rate_tmp = up_rate;
        }
        else                        //Disable max upload speed
        {
            tr_sessionLimitSpeed( session, TR_UP, FALSE );
			up_limit_enable = 0;
        }
    }
    {
        p=strstr(arg,"en_pex=");
        p=p+7;
        tmp=strstr(p,"@");
        len=strlen(p) - strlen(tmp);
        strncpy(value,p,len);
        *(value+len)='\0';
        en_pex = atoi(value);
        printf("receive config en_pex: %d\n",en_pex);

        if(en_pex == 0)             //Disable DHT
        	tr_sessionSetPexEnabled( session, FALSE);
        else
        	tr_sessionSetPexEnabled( session, TRUE);
    }
}

static void event_callback(const int sfd, short what UNUSED, void * vsession)
{
    char buf_ev[1024];
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    int len, fd;
    tr_session *session = vsession;
    memset(buf_ev, '\0', sizeof(buf_ev));

    fd = accept(sfd, (struct sockaddr *) &addr, &addrlen);
    if (recv(fd, buf_ev, 1024, 0) == -1)
    {
        perror("call to recv");
        exit(1);
    }

    fprintf(stderr,"receive: %s \n",buf_ev);
    len = strlen(buf_ev);

    if(strncmp(buf_ev,"add@0@",6)==0)    //add url
    {
        addtask_url(buf_ev+6, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"add@1@All",9)==0)  //add torrent file
    {
        addtask_path(buf_ev+10, session);
        send(fd, buf_ev, len, 0);
    }
    else if((strncmp(buf_ev,"add@1@",6)==0) &&(strncmp(buf_ev,"add@1@All",9)!=0))
    {
        addtask_path2(buf_ev+6, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"start@",6)==0)   //start
    {
        starttask(buf_ev+6, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"pause@",6)==0)   //pause
    {
        pausetask(buf_ev+6, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"cancel@",7)==0)   //cancel
    {
        canceltask(buf_ev+7, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"apply@",6)==0)   //config UI
    {
        apply_config(buf_ev+6, session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"all_paused@",11)==0)   //	Pause All
    {
        pause_alltask(session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"all_start@",10)==0)   //Resume All
    {
        start_alltask(session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"clean_completed@",16)==0)   //Clear Completed
    {
        clean_completedtask(session);
        send(fd, buf_ev, len, 0);
    }
    else if(strncmp(buf_ev,"all-renew-now",13)==0)
    {
        FILE *fp;
        //char dldir[100];//eric delete
        dldirchangednum++;
        memset(dldir,'\0',sizeof(dldir));
        memset(generalbuf,'\0',sizeof(generalbuf));
        memset(serialtmp,'\0',sizeof(serialtmp));
        memset(producttmp,'\0',sizeof(producttmp));
        memset(vondertmp,'\0',sizeof(vondertmp));
        fp = fopen("/tmp/APPS/DM2/Config/dm2_general.conf","r");
        if(fp)
        {
            while(!feof(fp))
            {
                fscanf(fp,"%[^\n]%*c",generalbuf);
                if(strncmp(generalbuf,"serial=",7) == 0)
                {
                    strcpy(serialtmp,generalbuf+7);
                }
                else if(strncmp(generalbuf,"vonder=",7) == 0)
                {
                    strcpy(vondertmp,generalbuf+7);
                }
                else if(strncmp(generalbuf,"product=",8) == 0)
                {
                    strcpy(producttmp,generalbuf+8);
                }
                else if(strncmp(generalbuf,"partition=",10) == 0)
                {
                    partitiontmp = atoi(generalbuf+10);
                }
                else if(strncmp(generalbuf,"Download_dir=",13) == 0)
                {
                    strcpy(dldir,generalbuf+13);
                }
                else if(strncmp(generalbuf,"MISC_SEEDING_X=",15) == 0)
                {
                    misc_seeding_x = atoi(generalbuf+15);
                }
            }
        }
        fclose(fp);

	    tr_torrent * tor = NULL;
	    while(( tor = tr_torrentNext( session, tor )))
	    {
		const tr_stat * st = tr_torrentStat(tor);
		if( tor->isDone == TRUE || tor->move_failed == TRUE)
		    continue;
		if( st->activity == TR_STATUS_DOWNLOAD ){
		    tr_torrentSetDownloadDir( tor, dldir );
		}
		else if( (st->activity == TR_STATUS_STOPPED) && (tor->isFinished != TRUE)){
		    tr_torrentSetDownloadDir( tor, dldir );
		}
                else if((st->activity == TR_STATUS_STOPPED) && ((tor->stats.percentDone < (double)1))){
		    tr_torrentSetDownloadDir( tor, dldir );
		}
		else if( (st->activity == TR_STATUS_STOPPED) && (tor->isFinished == TRUE) && misc_seeding_x==1){
		    tr_torrentStart( tor );
		}
		else if( (st->activity == TR_STATUS_SEED) && misc_seeding_x==0){
		    tor->isStopping = TRUE;
		    tor->isFinished = TRUE;
		}	
		
	    }
    }
    else if(strncmp(buf_ev,"seeding-renew-now",13)==0)
    {
        FILE *fp;
//        char dldir[100];
//        memset(dldir,'\0',sizeof(dldir));
//        memset(generalbuf,'\0',sizeof(generalbuf));
//        memset(serialtmp,'\0',sizeof(serialtmp));
//        memset(producttmp,'\0',sizeof(producttmp));
//        memset(vondertmp,'\0',sizeof(vondertmp));//eric delete
        fp = fopen("/tmp/APPS/DM2/Config/dm2_general.conf","r");
        if(fp)
        {
            while(!feof(fp))
            {
                fscanf(fp,"%[^\n]%*c",generalbuf);
		if(strncmp(generalbuf,"MISC_SEEDING_X=",15) == 0)
                {
                    misc_seeding_x = atoi(generalbuf+15);
                }
            }
        }
        fclose(fp);

	    tr_torrent * tor = NULL;
	    while(( tor = tr_torrentNext( session, tor )))
	    {
		const tr_stat * st = tr_torrentStat(tor);
		if( tor->isDone == TRUE || tor->move_failed == TRUE)
		    continue;
                //if( (st->activity == TR_STATUS_STOPPED) && (tor->isFinished == TRUE) && misc_seeding_x==1){
                  if( (st->activity == TR_STATUS_STOPPED) && (st->percentDone == (double)1) && misc_seeding_x==1){
		    tr_torrentStart( tor );
		}
		else if( (st->activity == TR_STATUS_SEED) && misc_seeding_x==0){
		    tor->isStopping = TRUE;
		    tor->isFinished = TRUE;
		}	
	    }
    }
    else if(strncmp(buf_ev,"dir-renew-now",13)==0)
    {
        FILE *fp;
        dldirchangednum++;
        //char dldir[100];//eriuc delete
        memset(dldir,'\0',sizeof(dldir));
        memset(generalbuf,'\0',sizeof(generalbuf));
        memset(serialtmp,'\0',sizeof(serialtmp));
        memset(producttmp,'\0',sizeof(producttmp));
        memset(vondertmp,'\0',sizeof(vondertmp));
        fp = fopen("/tmp/APPS/DM2/Config/dm2_general.conf","r");
        if(fp)
        {
            while(!feof(fp))
            {
                fscanf(fp,"%[^\n]%*c",generalbuf);
                if(strncmp(generalbuf,"serial=",7) == 0)
                {
                    strcpy(serialtmp,generalbuf+7);
                }
                else if(strncmp(generalbuf,"vonder=",7) == 0)
                {
                    strcpy(vondertmp,generalbuf+7);
                }
                else if(strncmp(generalbuf,"product=",8) == 0)
                {
                    strcpy(producttmp,generalbuf+8);
                }
                else if(strncmp(generalbuf,"partition=",10) == 0)
                {
                    partitiontmp = atoi(generalbuf+10);
                }
                else if(strncmp(generalbuf,"Download_dir=",13) == 0)
                {
                    strcpy(dldir,generalbuf+13);
                }
            }
        }
        fclose(fp);

	    tr_torrent * tor = NULL;
	    while(( tor = tr_torrentNext( session, tor )))
	    {
		const tr_stat * st = tr_torrentStat(tor);
		if( tor->isDone == TRUE || tor->move_failed == TRUE)
		    continue;
		if( st->activity == TR_STATUS_DOWNLOAD )
		    tr_torrentSetDownloadDir( tor, dldir );
		else if( (st->activity == TR_STATUS_STOPPED) && (tor->isFinished != TRUE))
		    tr_torrentSetDownloadDir( tor, dldir );
                else if((st->activity == TR_STATUS_STOPPED) && ((tor->stats.percentDone < (double)1)))
		    tr_torrentSetDownloadDir( tor, dldir );
	    }
    }
    // 20150902 leo added transmission username {
    /* else if(strncmp(buf_ev,"password-renew-now",18)==0)   // username and password  have changed   socket
     {
           tr_rpc_server * s;
              s = tr_new0 (tr_rpc_server, 1);
              s->session = session;

           char buf_username[32];   // leo added
           char buf_password[32];   // leo added

           FILE *fp;
                        system("/opt/etc/asus_script/dm2_transmission_user_changed");
                        while(access("/tmp/dm2_check_user",0)== 0)
                        {
                          sleep(1);
                        }
                        if(access("/tmp/dm2_check_user",0)!= 0){
                        fp=fopen("/tmp/transmission_get_up","r+");
                        memset(buf_username,0,sizeof(buf_username));
                        memset(buf_password,0,sizeof(buf_password));
                          if(fp != NULL)
                            {
                               fgets(buf_username,32,fp);
                               if(buf_username[strlen(buf_username)-1]=='\n')
                                 {
                                    buf_username[strlen(buf_username)-1]='\0';
                                 }
                                 fgets(buf_password,32,fp);
                                if(buf_password[strlen(buf_password)-1]=='\n')
                                 {
                                    buf_password[strlen(buf_password)-1]='\0';
                                 }


                            }
                         fclose(fp);
                       }
           tr_sessionSetRPCUsername (session,buf_username);

           tr_sessionSetRPCPassword (session,buf_password);


     }*/
    // 20150902 leo added for transmission username }
    else
        printf("receive message is wrong! \n");
    close(fd);

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

void tr_torrentlog(tr_torrent * torrent )
{
    if ( ! tr_isTorrent( torrent ) )
    {
        //fprintf(stderr,"\ntorrent struct is NULL\n");
        return ;
    }
    int fd ,n;
    long long int oneG;
    oneG = 1024*1024*1024;
    char trans_log[16], logname[128], tracker_log[16], trackername[128];
    Log_struc * log_p = NULL;
    tr_file_index_t file_n;
    int allfilesdownload = 1;
    uint64_t filesdownloadsize = 0;

    const tr_stat * st = tr_torrentStat((tr_torrent*)torrent);
    const tr_info * inf = tr_torrentInfo((tr_torrent*) torrent );
    char *p="Download2";//Added by eric
    char *q;
    char str[40]={0};
    dir_path=str;
	q=strstr(base_path,p);
   	strncpy(dir_path,base_path,q-base_path-1);

    //Tr_files torrent_files[inf->fileCount];
    tr_file_stat * files = tr_torrentFiles( torrent, &file_n );
    tr_tracker_stat ** tracker = tr_torrentTrackers( torrent, &n );

    log_p = (Log_struc *)malloc(sizeof(Log_struc));
    memset(trans_log, '\0', sizeof(trans_log));
    memset(logname, '\0', sizeof(logname));
    memset(tracker_log, '\0', sizeof(tracker_log));
    memset(trackername, '\0', sizeof(trackername));
    //memset(torrent_files, '\0', sizeof(torrent_files));

    sprintf(trans_log, "transm_%d", st->id);
    sprintf(logname, "%s.logs/%s", base_path,trans_log);
    sprintf(tracker_log, "tracker_%d", st->id);
    sprintf(trackername, "%s.logs/%s", base_path,tracker_log);
    if((fd = open(logname,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
        printf("Open log error\n");
    else
    {
        sprintf(log_p->id, "%s",inf->hashString);
        log_p->begin_t = st->addedDate;
        log_p->download_type = BT;
        log_p->error = torrent->error;
        sprintf(log_p->filename, "%s",inf->name);
        //log_p->filesize = inf->totalSize;
        log_p->now_t = st->activityDate;
        //log_p->progress = (float)(st->percentComplete);
        //fprintf(stderr,"\nfilename=%s\n",log_p->filename);
        //fprintf(stderr,"\nactivity=%d\n",st->activity);

        for(tr_file_index_t m = 0; m < inf->fileCount; m++ )
        {
            if(inf->files[m].dnd == 1) {
                allfilesdownload = 0;
                break;
            }
        }
        if(allfilesdownload == 1) {
            log_p->progress = (float)(st->percentDone);
            log_p->filesize = inf->totalSize;

        }
        else {
                log_p->filesize = 0;

                for(tr_file_index_t m = 0; m < inf->fileCount; m++ )
                {
                    if(inf->files[m].dnd == 0) {

                        log_p->filesize = log_p->filesize + inf->files[m].length;
                        filesdownloadsize = filesdownloadsize + files[m].bytesCompleted;
                    }
                }


                if (log_p->filesize == 0)
                {
                 log_p->progress=0;
                }
                else
                 log_p->progress = (float)(filesdownloadsize)/(float)(log_p->filesize);

        }



	if(st->activity == TR_STATUS_STOPPED)
        {
            if((st->leftUntilDone > tr_getDirFreeSpace(dir_path))&&(strstr(st->errorString,"No space left on device")!=NULL))//Added by eric
            {
                log_p->status = S_DISKFULL;
            }
            else if(torrent->isFinished == TRUE)
            {
                log_p->status = S_COMPLETED;
            }
            else if((log_p->progress - 1.000000) >= 0.000000)
            {
                /*if(misc_seeding_x ==1){
			torrent->isStopping = FALSE;
            		tr_torrentStart( torrent );
			log_p->status = S_SEEDING;
		}
		else{
                	log_p->status = S_COMPLETED;
                }*/
                if(misc_seeding_x ==0){
                        log_p->status = S_COMPLETED;
                   }
                else{
                        log_p->status = S_PAUSED;
                    }
            }
            else
                log_p->status = S_PAUSED;
        }
        else if((st->activity == TR_STATUS_SEED) && ((log_p->progress - 1.000000) >= 0.000000))
        {
		if(misc_seeding_x == 0){
			torrent->isStopping = TRUE;
            		log_p->status = S_COMPLETED;
		}else{
			log_p->status =S_SEEDING;
		}
        }
        else if(st->activity == TR_STATUS_DOWNLOAD)
        {
            if(st->hash_flag == 1)
                log_p->status = S_HASH;
            else
                log_p->status = S_PROCESSING;
        }
        else
        { log_p->status = S_INITIAL;

        }
        log_p->ifile.iBt.availability = (float)(st->sizeWhenDone - st->leftUntilDone + st->desiredAvailable)/(st->sizeWhenDone);
        snprintf(log_p->ifile.iBt.comment,1024, "%s",inf->comment);
        sprintf(log_p->ifile.iBt.creator, "%s",inf->creator);
        log_p->ifile.iBt.dateCreated = inf->dateCreated;
        log_p->ifile.iBt.partitionnum = 0;
        struct disk_info *disk_tmp;
        disk_tmp = follow_disk_info_start;
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
            sprintf(log_p->ifile.iBt.serial, "%s",disk_tmp->serialnum);
            sprintf(log_p->ifile.iBt.pid, "%s",disk_tmp->product);
            sprintf(log_p->ifile.iBt.vid, "%s",disk_tmp->vendor);
            log_p->ifile.iBt.partitionnum = disk_tmp->partitionport;
        }
        else
        {
            memset(log_p->ifile.iBt.serial,0,sizeof(log_p->ifile.iBt.serial));
            memset(log_p->ifile.iBt.pid,0,sizeof(log_p->ifile.iBt.pid));
            memset(log_p->ifile.iBt.vid,0,sizeof(log_p->ifile.iBt.vid));
        }

        if((disk_tmp)&&(strncmp(torrent->downloadDir,disk_tmp->mountpath,strlen(disk_tmp->mountpath)) != 0))
        {
            char *p;
            p = my_nstrchr('/',torrent->downloadDir,4);
            if(p)
				{
            char downloaddirtmp[strlen(p)+1];
            memset(downloaddirtmp,'\0',sizeof(downloaddirtmp));
            strncpy(downloaddirtmp,p,strlen(p));
            memset(torrent->downloadDir,'\0',sizeof(torrent->downloadDir));
            strcpy(torrent->downloadDir,disk_tmp->mountpath);
            strcat(torrent->downloadDir,downloaddirtmp);
            }
            else
				{
                memset(torrent->downloadDir,'\0',sizeof(torrent->downloadDir));
                strcpy(torrent->downloadDir,disk_tmp->mountpath);
            }
        }

        log_p->ifile.iBt.downloadedsize = st->downloadedEver;
        log_p->ifile.iBt.downloadpeers = st->peersSendingToUs;
        log_p->ifile.iBt.downloadSpeed = st->pieceDownloadSpeed_KBps;
        snprintf(log_p->ifile.iBt.errorString, 256,"%s",st->errorString);
        log_p->ifile.iBt.fileCount = inf->fileCount;
        log_p->ifile.iBt.have = st->haveValid + st->haveUnchecked;
        log_p->ifile.iBt.haveValid = st->haveValid;
        log_p->ifile.iBt.health = torrent->healthRatio;
        log_p->ifile.iBt.id = st->id;
        log_p->ifile.iBt.isPrivate = inf->isPrivate;
        log_p->ifile.iBt.pieceCount = inf->pieceCount;
        log_p->ifile.iBt.pieceSize = inf->pieceSize;
        log_p->ifile.iBt.ratio = st->ratio;
        log_p->ifile.iBt.trackCount = n;
        log_p->ifile.iBt.uploadedsize = st->uploadedEver;
        log_p->ifile.iBt.uploadpeers = st->peersGettingFromUs;
        log_p->ifile.iBt.uploadSpeed = st->pieceUploadSpeed_KBps;

	/*if(tr_cpGetStatus( &torrent->completion ) == TR_SEED){
		        log_p->ifile.iBt.downloadSpeed = 0.00;
				log_p->ifile.iBt.uploadSpeed = 0.00;
	}*/

        for(tr_file_index_t m = 0; m < inf->fileCount; m++ )
        {
            //sprintf(torrent_files[m].filename, "%s",inf->files[m].name);
            //torrent_files[m].filecompletedsize = files[m].bytesCompleted;
            //torrent_files[m].filetotalsize = inf->files[m].length;
            //torrent_files[m].progress = files[m].progress;

            if(inf->files[m].length > 4*oneG)//eric added for disktype
            {
                if((strcmp(disktype,"vfat") == 0) || (strcmp(disktype,"tfat") == 0))
                {
                    torrent->isStopping = TRUE;
                    log_p->status = S_TYPE_OF_ERROR;
                    memset(log_p->ifile.iBt.errorString,'\0',sizeof(log_p->ifile.iBt.errorString));
                    strcpy(log_p->ifile.iBt.errorString,"The system disk format does not support files more than 4GB");
                }
                else
                    torrent->have4Gfile = 1;
            }//eric added 2012.11.29
        }
        char *incompletedir = (char *)malloc(strlen(base_path)+12);
        memset(incompletedir,'\0',sizeof(incompletedir));
        sprintf(incompletedir,"%s%s",base_path,"/InComplete");
        if(torrent->move_failed)
        {
            memset(log_p->ifile.iBt.errorString,'\0',sizeof(log_p->ifile.iBt.errorString));
            strcpy(log_p->ifile.iBt.errorString,"No space left on download disk");
            log_p->status = S_MOVEDISKFULL;
            tr_torrentSetDownloadDir( torrent, incompletedir );
        }
        else if(torrent->isDone)
        {
             memset(log_p->ifile.iBt.errorString,'\0',sizeof(log_p->ifile.iBt.errorString));
             strcpy(log_p->ifile.iBt.errorString,"Download disk cannot support more than 4GB file, so we put it in Download2/InComplete folder");
             log_p->status = S_MOVE4GBERROR;
             tr_torrentSetDownloadDir( torrent, incompletedir );
        }
		if(incompletedir != NULL){
        	free(incompletedir);
		}

        char *realdldir;
        realdldir = my_nstrchr('/',torrent->downloadDir,4);
        if ( !access(torrent->downloadDir,0) ){
            sprintf(log_p->ifile.iBt.downloadDir, "%s",torrent->downloadDir);
        }
        else{
            sprintf(log_p->ifile.iBt.downloadDir, "%s%s","not_found",realdldir);
        }


        if(write(fd, log_p, LOG_SIZE) < 0)
            printf("Write log error\n");

        //if(write(fd, torrent_files, sizeof(torrent_files))<0)
        //    printf("Write torrent_files error\n");

        close(fd);
    }

    if((fd = open(trackername,O_RDWR|O_CREAT|O_TRUNC, 0777))<0)
        printf("Open tracker_log error\n");
    else
    {
        if(write(fd, tracker, n*sizeof(tr_tracker_stat)) < 0)
            printf("Write tracker_file_log error\n");

        close(fd);
    }
    free(log_p);
    free(tracker);
    tr_free(files);
}
int
getFreeMem(void)
{
    char buf[512];
    int memfree=0, buffers=0, cached=0;
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    while (fgets(buf, sizeof(buf), fp)) {
	if (!strncmp(buf, "MemFree:", 8)) {
	    if (sscanf(buf, "MemFree: %d", &memfree) != 1) {
		fclose(fp);
		return -1;
	    }
	} else if (!strncmp(buf, "Buffers:", 8)) {
	    if (sscanf(buf, "Buffers: %d", &buffers) != 1) {
		fclose(fp);
		return -1;
	    }
	} else if (!strncmp(buf, "Cached:", 7)) {
	    if (sscanf(buf, "Cached: %d", &cached) != 1) {
		fclose(fp);
		return -1;
	    }
	}
    }
    fclose(fp);
    /* return memfree + buffers + cached; */
    return memfree;
}


int
getMinMem(void)   //2013.06.24 magic added
{
    char buf[16];
    int MemMem=0;
    FILE *fp = fopen("/proc/sys/vm/min_free_kbytes", "r");
    if (!fp) return -1;

    while (fgets(buf, sizeof(buf), fp)) {
        MemMem=atoi(buf);
    }
    fclose(fp);
    //fprintf(stderr,"\nMemMem=%d\n",MemMem);
    return MemMem;
}
