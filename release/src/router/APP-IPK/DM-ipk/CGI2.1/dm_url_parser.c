/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#define HAVE_STRINGS_H 1
#define safe_free(x)		if(x) free(x)
#define TIME_OUT_TIME 3        //set timeout to 20s

//#include "config.h"

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include "dm_url_parser.h"
//#include "util.h"

static sigjmp_buf jmpbuf;

static void alarm_func()
{
     siglongjmp(jmpbuf, 1);
}

static struct hostent *gngethostbyname(char *HostName, int timeout)
{
#ifdef DM_I686
     struct hostent *lpHostEnt;

     char   str[32];//leo

     char *h_ip=NULL;//leo

     if(signal(SIGALRM, alarm_func)==SIG_ERR)
     {
        perror("signal");
     }
     if(sigsetjmp(jmpbuf, 1) != 0)
     {
           alarm(0);//timout
           signal(SIGALRM, SIG_IGN);
           return NULL;
     }
     alarm(timeout);//setting alarm

     int tu=0;
     for(;tu<timeout;tu++)
     {

         lpHostEnt = gethostbyname(HostName);
          h_ip=inet_ntop(lpHostEnt->h_addrtype, lpHostEnt->h_addr, str, sizeof(str));

          if((lpHostEnt==NULL)||(!strcmp(h_ip,"10.0.0.1")))
        {
          sleep(1);
         }
         else
         {

         break;//internet is fine
         }

     }
     signal(SIGALRM, SIG_IGN);

     return lpHostEnt;
#else
     struct hostent *lpHostEnt;

     signal(SIGALRM, alarm_func);
     if(sigsetjmp(jmpbuf, 1) != 0)
     {
           alarm(0);//timout
           signal(SIGALRM, SIG_IGN);
           return NULL;
     }
     alarm(timeout);//setting alarm
     lpHostEnt = gethostbyname(HostName);
     signal(SIGALRM, SIG_IGN);

     return lpHostEnt;
#endif
}

/*
void
safe_free(void *ptr)
{
        if( ptr ) {
                debug("safe_free [%p]", ptr);
                free(ptr);
                ptr = NULL;
        }
}
*/

char *
        strconcat (const char *string1, ...) //from util.c
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


int
is_probably_an_url(char *string)
{
        if( strstr(string, "://") )
                return 1;

        return 0;
}

static char *
        get_service_type(char *string, Url *u)
{
    /* fixme: what if the string isn't at the beginning of the
           *string? */

    char bt_type[10], nzb_type[10] ;
    memset(bt_type,0,sizeof(bt_type));
    memset(nzb_type,0,sizeof(nzb_type));

    strcpy(bt_type,string+strlen(string)-8);
    strcpy(nzb_type,string+strlen(string)-4);

    if(strcmp(bt_type,".torrent") == 0) {
        if( strstr(string, "http://") )
            string += 7;
        u->service_type = SERVICE_BT;
        return string;
    }

    if(strcmp(nzb_type,".nzb") == 0) {
        string += 7;
        u->service_type = SERVICE_NZB;
        return string;
    }

    //fprintf(stderr,"file type is %s \n",file_type);

    if( strstr(string, "magnet:") ) {
        string += 7;
        u->service_type = SERVICE_BT;
        return string;
    }

    //if( strstr(string, "http://") &&  strcmp(bt_type,".torrent") != 0  &&  strcmp(nzb_type,".nzb") != 0) {
    if( strstr(string, "http://") ) {
        string += 7;		/* skip past that part */
        u->service_type = SERVICE_HTTP;
        return string;
    }

    if( strstr(string, "ftp://") ) {
        string += 6;
        u->service_type = SERVICE_FTP;
        return string;
    }


    if( strstr(string, "ed2k://") ) {
        string += 7;
        u->service_type = SERVICE_ED2K;
        return string;
    }


    /*
        if( strstr(string, "gopher://") ) {
                string += 9;
                u->service_type = SERVICE_GOPHER;
                return string;
        }
        */

    if( strncasecmp(string, "www", 3) == 0 ) {
        u->service_type = SERVICE_HTTP;
        u->full_url = strconcat("http://", u->full_url, NULL);
        return string;
    }

    if( strncasecmp(string, "ftp", 3) == 0 ) {
        u->service_type = SERVICE_FTP;
        u->full_url = strconcat("ftp://", u->full_url, NULL);
        return string;
    }

    /* default to browser-style serviceless http URL */
    char *p = NULL;
    p = strstr(u->full_url, "://");
    if( p == NULL)
    {
        u->full_url = strconcat("http://", u->full_url, NULL);

    }
    else
    {
        p = p + 3;
        u->full_url = strconcat("http://", p, NULL);
        //string += 7;
        string = p ;
    }
        u->service_type = SERVICE_HTTP;
        return string;
}


static char *
get_username(char *string, Url *u)
{
        int i;
        char *username;
        char *at;
        char *slash;
        char *colon;
        char tmp[256] = {0};
        char tmp1[256] = {0};
        int len;
        
        at = strchr(string, '@');
        slash = strchr(string, '/');

        if( (!at) || (slash && (at >= slash)) )
                return string;

        strncpy(tmp,string,strlen(string)-strlen(slash)); // user:pwd@host
        at = strrchr(tmp,'@');

        if(!at)
            return string;

        strncpy(tmp1,tmp,strlen(tmp)-strlen(at));         //user:pwd or user
        //fprintf(stderr,"user&pwd=%s\n",tmp1);
        colon = strchr(tmp1,':');

        if(colon)                                       //user:pwd
            len = strlen(tmp1)-strlen(colon);    
        else                                           //user
            len = strlen(tmp1);

        username = malloc(len+1);
        memcpy(username, tmp1, len);
        username[len] = '\0';
        string += len + 1 ;



        u->username = username;
        return string;
}


static char *
get_password(char *string, Url *u)
{
        int i;
        char *password;
        char *at;
        char *slash;
        char *colon;
        char tmp[256] = {0};
        char tmp1[256] = {0};
        int len;
        
        at = strchr(string, '@');
        slash = strchr(string, '/');

        if( (!at) || (slash && (at >= slash)) )
                return string;

        /* skipping to the end of the host portion.
           this is kinda messy for the (rare) cases where someone
           has a slash and/or at in their password. It's not perfect;
           but it catches easy cases. 
           
           If you know of a better way to do this, be my guest. I do not
           feel a particular paternal instinct towards my ugly code.

           I guess that applies to this whole program.
        */



        strncpy(tmp,string,strlen(string)-strlen(slash)); // pwd@host
        at = strrchr(tmp,'@');

        if(!at)
            return string;

        strncpy(tmp1,tmp,strlen(tmp)-strlen(at));         //pwd


        len = strlen(tmp1);
        password = malloc(len+1);
        memcpy(password,tmp1,len);
        password[len] = '\0';

        string += len + 1;

        u->password = password;
        
        return string;
}


static char *
get_hostname(char *url, Url *u)
{
        char *hostname;
        int i;

        /* skip to end, slash, or port colon */
        for( i = 0; url[i] && url[i] != '/' && url[i] != ':'; i++ );

        hostname = malloc(i + 1);

        memcpy(hostname, url, i);

        hostname[i] = '\0';

        /* if there's a port */
        if(url[i] == ':')
                url += i + 1;
        else
                url += i;

        u->host = hostname;
        return url;
}

static char *
get_port(char *url, Url *u)
{
        char *port_string;
        int i;

        for(i = 0; isdigit(url[i]); i++);

        if(i == 0)
                return url;


        port_string = malloc(i + 1);
        memcpy(port_string, url, i + 1);

        port_string[i] = '\0';

        url += i;
        
        u->port = atoi(port_string);

        return url;
}


static char *
get_path(char *url, Url *u)
{
        int i;
        char *path;

        /* find where the last slash is */
        for(i = strlen(url); i > 0 && url[i] != '/'; i--);

        if(url[i] != '/')
                return url;

        path = malloc(i + 2);
        memcpy(path, url, i + 1);
        path[i] = '/';
        path[i + 1] = '\0';

        url += i + 1;
        u->path = path;

        return url;
}


static char *
get_file(char *string, Url *u)
{
        char *file;
        
        if( !string[0] ) 
                return NULL;

        file = malloc(strlen(string) + 1);

        memcpy(file, string, strlen(string) + 1);

        u->file = file;

        return string;
}


Url *
url_new(void)
{
	Url *new_url;

	new_url = malloc(sizeof(Url));

        new_url->full_url	= NULL;
	new_url->service_type 	= 0;
	new_url->username 	= NULL;
	new_url->password 	= NULL;
	new_url->host 		= NULL;
	new_url->port 		= 0;
	new_url->path 		= NULL;
	new_url->file		= NULL;

	return new_url;
}


void 
url_destroy(Url *u)
{
        if( !u )
                return;
        
        safe_free(u->full_url);
        safe_free(u->username);
        safe_free(u->password);
        safe_free(u->host);
        safe_free(u->path);
        safe_free(u->file);
}
        

UrlResource *
url_resource_new(void)
{
        UrlResource *new_resource;
        
        new_resource = malloc(sizeof(UrlResource));
        
        new_resource->url 		= NULL;
        new_resource->outfile		= NULL;
        new_resource->proxy		= NULL;
        new_resource->proxy_username	= NULL;
        new_resource->proxy_password	= NULL;
        new_resource->options		= 0;
        new_resource->outfile_size	= 0;
        new_resource->outfile_offset	= 0;

        return new_resource;
}


void 
url_resource_destroy(UrlResource *rsrc)
{
        if( !rsrc )
                return;

        if(rsrc->url)
                url_destroy(rsrc->url);

        safe_free(rsrc->outfile);

        free(rsrc);
}


Url *
url_init(Url *u, char *string)
{
        char *sp;	/* since we're going to walk through string,
                           use a copy instead. */

        sp = string;

        u->full_url = (char *)strdup(string);

        if( ! (sp = get_service_type(sp, u)) )
                return 0;

        /* only get username/password if they are not null,
           allows us to handle redirects properly */

        if( !u->username )
                sp = get_username(sp, u);
        if( !u->password )
                sp = get_password(sp, u);

        sp = get_hostname(sp, u);

        if( ! (u->host && *(u->host)) )
                return NULL;

        sp = get_port(sp, u);

        sp = get_path(sp, u);
        sp = get_file(sp, u);

        return u;
}

// from list.c {
List *
list_new(void)
{
        List *new_list;

        new_list = malloc(sizeof(List));

        new_list->data = NULL;
        new_list->next = NULL;

        return new_list;
}


List *
list_append(List *l, void *data)
{
        if (l->data == NULL) {
                l->data = data;
                return l;
        }

        while (l->next) {
                l = l->next;
        }

        l->next = list_new();
        l->next->data = data;
        l->next->next = NULL;
}
//from list.c }

//from util.c
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

void
        report(enum report_levels lev, char *format, ...)
{
    switch( lev ) {
    case DEBUG:
        //fprintf(stderr, "debug: ");
        break;
    case WARN:
        //fprintf(stderr, "warning: ");

        break;
    case ERR:
        //fprintf(stderr, "error: ");
        break;
    default:
        break;
    }

    if( format ) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        //fprintf(stderr, "\n");
    }
}

int
        tcp_connect(char *remote_host, int port)
{
    struct hostent *host;
    struct sockaddr_in sa;
    int sock_fd;

    if((host = (struct hostent *)gngethostbyname(remote_host,TIME_OUT_TIME)) == NULL) {
        herror(remote_host);
        return 0;
    }

    /* get the socket */
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 0;
    }

    //add by gauss

   //fprintf(stderr,"start connect function!\n");
    //int error =1 ,len;
    //len =sizeof(int);
    struct timeval tm = {TIME_OUT_TIME,0};
    socklen_t len =sizeof(struct timeval);
    setsockopt(sock_fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&tm,len);
    //setsockopt(sock_fd,SOL_SOC)
    //struct timeval tm;
    //fd_set set;
    unsigned long ul = 1;
    //ioctl(sock_fd,FIONBIO,&ul);
    //bool ret = false;
    //int ret = 0;


    /* connect the socket, filling in the important stuff */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr, host->h_addr,host->h_length);

    if(connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0){

       /*
        //tm.tv_set = TIME_OUT_TIME;
        tm.tv_sec = TIME_OUT_TIME;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(sock_fd,&set);
        if( select(sock_fd+1,NULL,&set,NULL,&tm)> 0)
        {
            getsockopt(sock_fd,SOL_SOCKET,SO_ERROR,&error,(socklen_t *)&len);
            if( error == 0)
                ret = 1;
            else
                ret = 0;
        }
        else
            ret = 0;
        //perror(remote_host);
        //return 0;
       */


       if(errno == EINPROGRESS )
        {
           perror(remote_host);
           //fprintf(stderr,"connect host timeout\n");
           return 0;
       }

           //fprintf(stderr,"connect host error\n");
           return 0;

    }
/*
    else
        ret = 1;
    ul = 0;
    ioctl(sock_fd,FIONBIO,&ul);
    if(!ret)
    {
        close(sock_fd);
        fprintf(stderr,"Can't connect the server!\n");
        return 0;
    }

        return 0;
*/

    //fprintf(stderr,"Connect OK!\n");
    //return 1;
    return sock_fd;
}

//from util.c }



		

