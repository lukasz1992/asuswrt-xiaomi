/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

//#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#ifdef HAVE_STDARG_H
# include <stdarg.h>
//#endif

#include <netdb.h>

#ifdef USE_SOCKS5
#define SOCKS
#include <socks.h>
#endif

#include <errno.h>
#include "dm_http_parser.h"
//#include "util.h"
#include "dm_url_parser.h"
#include "dm_ftp_parser.h"
//#include "options.h"
#define safe_free(x)		if(x) free(x)
#define safe_strdup(x)		( (x) ? strdup(x) : NULL )
#define BUFSIZE (5*2048)
#define TIMEOUT 3

void
close_quit(int sock)
{
    if(sock) {
        write(sock, "QUIT\r\n", 6);
        close(sock);
    }
}


static void
ftp_set_defaults(UrlResource *rsrc, Url *u)
{
    if( !u->port )
        u->port = 21;
    if( !u->username )
        u->username = strdup("anonymous");
    if( !u->password )
        u->password = strdup("snarf@");

    if( !rsrc->outfile ) {
        if( u->file )
            rsrc->outfile = strdup(u->file);
        else
            rsrc->outfile = strdup("ftpindex.txt");
    }
}


void
send_control(int sock, char *string, ...)
{
    va_list args;
    char *line	= NULL;
    char *newline;
    char *s         = NULL;

    line = safe_strdup(string);

    va_start(args, string);
    s = va_arg(args, char *);
    while( s ) {
        newline = strconcat(line, s, NULL);
        safe_free(line);
        line = newline;
        s = va_arg(args, char *);
    }
    va_end(args);

    write(sock, line, strlen(line));
    safe_free(line);
}


char *
get_line(UrlResource *rsrc, int control)
{
    int bytes_read	= 0;
    char *end;
    char buf[BUFSIZE+1];

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;


    if ( setsockopt(control,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval)) )
    {
     perror("*********setsockopt recive is error:*******");
    }
    else
    {
        fprintf(stderr, "********setsockopt recv is ok **********\n");
    }

    while( (bytes_read = read(control, buf, BUFSIZE)) ) {
        if( rsrc->options & OPT_VERBOSE )
            fwrite(buf, 1, bytes_read, stderr);

        if( (buf[0] == '4' || buf[0] == '5') &&
                !((rsrc->options & (OPT_VERBOSE | OPT_QUIET))) ) {
            fwrite(buf, 1, bytes_read, stderr);
            return NULL;
        }

        if(bytes_read == -1 && errno == EAGAIN)
        {
            fprintf(stderr,"read timeout\n");
            //rsrc->url->status = P_FAIL;
            return  NULL;
        }

        /* in case there's a partial read */
        buf[bytes_read] = '\0';

        if( buf[bytes_read - 1] == '\n' )
            buf[bytes_read - 1] = '\0';

        if( buf[bytes_read - 2] == '\r' )
            buf[bytes_read - 2] = '\0';

        if( isdigit(buf[0]) && buf[3] == ' ' ) {
            //printf("$$$$ buf is %s\n",buf);
            return strdup(buf);
        }

        /* skip to last line of possibly multiple line input */

        if( (end = strrchr(buf, '\n')) ) {
            end++;
            if( isdigit(end[0]) && end[3] == ' ' )
            {
                return strdup(end);
                //printf("$$$$ end is %s\n",end);
            }

        }
    }

    return NULL;
}


static int
check_numeric(const char *numeric, const char *buf)
{
    return( (buf[0] == numeric[0]) &&
           (buf[1] == numeric[1]) &&
           (buf[2] == numeric[2]) );
}


/* cheesy I know */

int
sock_init(struct sockaddr_in *sa, int control)
{
    socklen_t i;
    int sock;

    if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket");
        return(0);
    }

    i = sizeof(*sa);

    getsockname (control, (struct sockaddr *)sa, &i) ;
    sa->sin_port = 0 ; /* let system choose a port */
    if (bind (sock, (struct sockaddr *)sa, sizeof (*sa)) < 0) {
        perror("bind");
        return 0;
    }

    return sock;
}



int
get_passive_sock(UrlResource *rsrc, int control)
{
    unsigned char *addr;
    struct sockaddr_in sa;
    int sock;
    int x;
    char *line, *orig_line;

    send_control(control, "PASV\r\n", NULL);

    if( !((line = get_line(rsrc, control)) &&
          check_numeric("227", line)) ) {
        safe_free(line);
        return 0;
    }

    orig_line = line;

    if( strlen(line) < 4 ) {
        safe_free(line);
        return 0;
    }

    if( !(sock = sock_init(&sa, control)) )
        return -1;

    /* skip the numeric response */
    line += 4;

    /* then find the digits */

    while( !(isdigit(*line)) )
        line++;

    /* ugliness from snarf 1.x */

    sa.sin_family = AF_INET;
    addr = (unsigned char *)&sa.sin_addr;

    for(x = 0; x < 4; x++) {
        addr[x] = atoi(line);
        line = strchr(line,',') + 1;
    }

    addr = (unsigned char *)&sa.sin_port ;
    addr[0] = atoi(line);
    line = strchr(line,',') + 1;
    addr[1] = atoi(line);

    if( connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0 ) {
        safe_free(orig_line);
        perror("connect");
        return -1;
    }

    safe_free(orig_line);
    return sock;
}


int
get_sock(UrlResource *rsrc, int control)
{
    struct sockaddr_in sa;
    unsigned char *addr;
    unsigned char *port;
    char *line;
    char port_string[BUFSIZE];
    unsigned int sock;
    socklen_t i;

    if(! (sock = sock_init(&sa, control)) )
        return 0;


    if ( listen(sock, 0) < 0 ) {
        perror("listen");
        return 0;
    }

    i = sizeof(sa);

    getsockname(sock, (struct sockaddr *)&sa, &i);

    addr = (unsigned char *)(&sa.sin_addr.s_addr);
    port = (unsigned char *)(&sa.sin_port);

    sprintf(port_string, "PORT %d,%d,%d,%d,%d,%d\r\n",
            addr[0], addr[1], addr[2], addr[3],
            port[0],(unsigned char)port[1]);

    send_control(control, port_string, NULL);

    if( !((line = get_line(rsrc, control)) &&
          check_numeric("200", line)) ) {
        safe_free(line);
        return 0;
    }
    safe_free(line);

    return sock;
}


/* I'm going to go to hell for not doing proper cleanup. */

int
ftp_transfer(UrlResource *rsrc)
{
    fprintf(stderr, "enter ftp_transfer() \n");
    Url 	*u		= NULL;
    char 	*line		= NULL;
    int 	sock		= 0;

    u = rsrc->url;

    /* first of all, if this is proxied, just pass it off to the
           http module, since that's how we support proxying. */

    //rsrc->proxy = get_proxy("FTP_PROXY");

    ftp_set_defaults(rsrc, u);

    if( !(sock = tcp_connect(u->host, u->port)) )
    {
        //return 0;
        rsrc->url->status = P_FAIL;
        return  P_FAIL;
    }

    if( !(line = get_line(rsrc, sock)) )
    {
        //return 0;
        rsrc->url->status = P_FAIL;
        return  P_FAIL;
    }

    if( !check_numeric("220", line) ) {
        safe_free(line);
        report(ERR, "bad server greeting: %s");
        //return 0;
        rsrc->url->status = P_FAIL;
        return  P_FAIL;
    }

   fprintf(stderr, "server respone is %s \n",line);

    send_control(sock, "USER ", u->username, "\r\n", NULL);

    if( !(line = get_line(rsrc, sock)) )
    {
        //return 0;
        rsrc->url->status = P_FAIL;
        return  P_FAIL;
    }

    fprintf(stderr, "server respone is %s \n",line);

    /* do the password dance */
    if( !check_numeric("230", line) ) {
        if( !check_numeric("331", line)) {
            safe_free(line);
            report(ERR, "bad/unexpected response: %s", line);
            //return 0;
            rsrc->url->status = P_NEEDACCOUNT;
            return P_NEEDACCOUNT;
        } else {
            safe_free(line);

            send_control(sock, "PASS ", u->password, "\r\n", NULL);

            if( !((line = get_line(rsrc, sock)) &&
                  check_numeric("230", line)) ) {
                safe_free(line);
                report(ERR, "login failed");
                //return 0;
                //rsrc->url->status = P_FAIL;
                //return P_FAIL;
                rsrc->url->status = P_NEEDACCOUNT;
                return P_NEEDACCOUNT;
            }
            safe_free(line);
        }
    }


    close(sock);

    char bt_type[10], nzb_type[10] ;
    memset(bt_type,0,sizeof(bt_type));
    memset(nzb_type,0,sizeof(nzb_type));

    strcpy(bt_type,u->full_url+strlen(u->full_url)-8);
    strcpy(nzb_type,u->full_url+strlen(u->full_url)-4);

    //strncpy(bt_type,u->full_url,strlen(u->full_url))

    if(strcmp(bt_type,".torrent") == 0) {
        u->service_type = SERVICE_BT;
    }

    if(strcmp(nzb_type,".nzb") == 0)
    {
        u->service_type = SERVICE_NZB;
    }

    if(u->service_type == SERVICE_UNKNOW)
        u->service_type == SERVICE_FTP;


    rsrc->url->status = P_PASS;

    return P_PASS;
}

