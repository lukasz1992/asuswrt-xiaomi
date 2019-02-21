/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/statvfs.h>

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif

#include <netdb.h>

#ifdef USE_SOCKS5
#define SOCKS
#include <socks.h>
#endif

#include <errno.h>
#include "http.h"
#include "util.h"
#include "url.h"
#include "ftp.h"
#include "options.h"
#include "logs.h"

#define RESERVE_SPACE    (int64_t)50*1024*1024 // add by gauss

extern int socket_write  ;
extern int socket_read  ;
extern char basic_path[256];

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

        socket_write = 1;
        write(sock, line, strlen(line));
        socket_write = 0;


        safe_free(line);
}


char *
get_line(UrlResource *rsrc, int control)
{
        int bytes_read	= 0;
        char *end;
        char buf[BUFSIZE+1];

        while( (bytes_read = read(control, buf, BUFSIZE)) ) {

                socket_read = 0 ;

                if( rsrc->options & OPT_VERBOSE )
                        fwrite(buf, 1, bytes_read, stderr);

                if( (buf[0] == '4' || buf[0] == '5') && 
                    !((rsrc->options & (OPT_VERBOSE | OPT_QUIET))) ) {
                        fwrite(buf, 1, bytes_read, stderr);
                        return NULL;
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

             socket_read = 1;
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

char *get_new_item(char *item)
{
    char *new_item = NULL;

    if(item == NULL)
        return NULL;

    new_item = strdup(item);
    char *p = strchr(item,'%');
    if(p && ISXDIGIT(p[1]) && ISXDIGIT(p[2]) )
    {
        free(new_item);
        new_item = oauth_url_unescape(item,NULL);
       //fprintf(stderr,"item=%s,new_item=%s\n",item,new_item);
    }

    return new_item;
}


/* I'm going to go to hell for not doing proper cleanup. */
        
int
ftp_transfer(UrlResource *rsrc)
{
        Url 	*u		= NULL;
        FILE 	*out		= NULL;
        char 	*line		= NULL;
        int 	sock		= 0;
        int 	data_sock	= 0;
        int	passive		= 1;
        int	retval		= 0;
        char *new_path          = NULL;
        char *new_file          = NULL;

        u = rsrc->url;

        /* first of all, if this is proxied, just pass it off to the
           http module, since that's how we support proxying. */

        rsrc->proxy = get_proxy("FTP_PROXY");

        if( rsrc->proxy ) {
                return http_transfer(rsrc);
        }

        ftp_set_defaults(rsrc, u);

        if( !(sock = tcp_connect(u->host, u->port)) ){
                err_log(rsrc, "sock error",FTP);
                return 0;
	}

        if( !(line = get_line(rsrc, sock)) ){
                err_log(rsrc, "get line error",FTP);
                return 0;
	}

        if( !check_numeric("220", line) ) {
                safe_free(line);
                //report(ERR, "bad server greeting: %s");
                err_log(rsrc, "bad server greeting",FTP);
                return 0;
        }


        send_control(sock, "USER ", u->username, "\r\n", NULL);

        if( !(line = get_line(rsrc, sock)) ){
                err_log(rsrc, "get line error",FTP);
                return 0;
	}

        /* do the password dance */
        if( !check_numeric("230", line) ) {
                if( !check_numeric("331", line)) {
                        safe_free(line);
                        //report(ERR, "bad/unexpected response: %s", line);
                        err_log(rsrc, "bad/unexpected response",FTP);
                        return 0;
                } else {
                        safe_free(line);

                        send_control(sock, "PASS ", u->password, "\r\n", NULL);
                        
                        if( !((line = get_line(rsrc, sock)) &&
                              check_numeric("230", line)) ) {
                                safe_free(line);
                                //report(ERR, "login failed");
                                err_log(rsrc, "login failed",FTP);
                                return 0;
                        }
                        safe_free(line);
                }
        }
        
        /* set binmode */
        send_control(sock, "TYPE I\r\n", NULL);

        if( !(line = get_line(rsrc, sock)) ){
                err_log(rsrc, "get line error",FTP);
                return 0;
	}
        safe_free(line);

        if( u->path ) {

           new_path = get_new_item(u->path);
            //send_control(sock, "CWD ", u->path, "\r\n", NULL);
            send_control(sock, "CWD ",new_path, "\r\n", NULL);

            free(new_path);

            if( !((line = get_line(rsrc, sock)) &&
                  //check_numeric("250", line)) ) {
                  ( check_numeric("250", line) || check_numeric("200", line) )) ) {
                safe_free(line);
                close_quit(sock);
                err_log(rsrc, "get line error",FTP);
                return 0;
            }
            safe_free(line);
        }
        
        /* finally, the good stuff */

        /* get a socket for reading. try passive first. */
        
        if( ! (rsrc->options & OPT_ACTIVE) ) {
                if( (data_sock = get_passive_sock(rsrc, sock)) == -1 ){
                        err_log(rsrc, "get passive sock error",FTP);
                        return 0;
		}
        }


        if( !data_sock ) {
                if( (data_sock = get_sock(rsrc, sock)) < 1 ){
                        return 0;
                        err_log(rsrc, "get sock error",FTP);
                } else
                        passive = 0;
        }

        if (u->file) {
            new_file = get_new_item(u->file);
            //send_control(sock, "SIZE ", u->file, "\r\n", NULL);
            send_control(sock, "SIZE ", new_file, "\r\n", NULL);
            //free(new_file);
            line = get_line(rsrc, sock);
            if (line && check_numeric("213", line)) {
                //rsrc->outfile_size = atoi(line + 3);
                rsrc->outfile_size = atoll(line + 3); //modify by gauss
            } else {
                rsrc->outfile_size = 0;
            }
        }

        /* handle resume */
        if( rsrc->outfile_offset && (rsrc->options & OPT_RESUME) ) {
                char numstring[BUFSIZE]; /* ugly hack */

                sprintf(numstring, "%ld", (long int )rsrc->outfile_offset);
                send_control(sock, "REST ", numstring, "\r\n", NULL);

                if( !((line = get_line(rsrc, sock)) &&
                    check_numeric("350", line)) ) {
                        safe_free(line);
                        close_quit(sock);
                        //report(ERR, "server does not support FTP resume, "
                        //       "try again without -r");
                        err_log(rsrc, "get line error",FTP);
                        return 0;
                }
                safe_free(line);
        }

        if (u->file)
        {
                //send_control(sock, "RETR ", u->file, "\r\n", NULL);
                send_control(sock, "RETR ", new_file, "\r\n", NULL);
                free(new_file);
        }
        else
                send_control(sock, "NLST\r\n", NULL);

        if( !((line = get_line(rsrc, sock)) &&
              (check_numeric("150", line) || check_numeric("125", line))) ) {
                safe_free(line);
                close_quit(sock);
                return 0;
        }

        if( !passive ) 
                data_sock = accept(data_sock, NULL, NULL);

        /*        rsrc->outfile_size = guess_file_size(line); */

        safe_free(line);

        if( ! (out = open_outfile(rsrc)) ) {
                report(ERR, "opening %s: %s", rsrc->outfile, 
                      strerror(errno));
                close_quit(sock);
                err_log(rsrc, "open file error",FTP);
                return 0;
        }

        /* check disk space is full add by gauss*/
        struct statvfs diskdata;
        long long int free_disk_space;
        //long long int reserve_disk_space = 50*1024*1024*1024;
        //long long int reserve_disk_space = 79891619300;
        if (!statvfs(basic_path, &diskdata))
        {
                //return (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
            free_disk_space = (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
            printf("free disk space is %lld \n",free_disk_space);

            if( rsrc->outfile_offset > 0 )
            {
                if( rsrc->outfile_size - rsrc->outfile_offset >= free_disk_space - RESERVE_SPACE )
                {
                    err_log(rsrc,"Disk space is not enough",HTTP);
                    return 0;
                }
            }
            else
            {
                if( rsrc->outfile_size >= free_disk_space - RESERVE_SPACE )
                {
                    err_log(rsrc,"Disk space is not enough",HTTP);
                    return 0;
                }
            }

        }
        else
        {
            printf("obtain disk space is failed ,basic_path is %s \n",basic_path);
        }

        retval = dump_data(rsrc, data_sock, out);


        //Allen 20101014, 防止接收不到226而阻塞住 {
        struct timeval  tval;
        fd_set  rset;
        int  nready ;

        tval.tv_sec = 2;
        tval.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        if((nready = select(sock+1, &rset, NULL, NULL, &tval)) <= 0)
                fprintf(stderr, "[snarf] read line TIMEOUT nready:%d\n",nready);
        else
        {
                line = get_line(rsrc, sock); /* 226 Transfer complete */
                safe_free(line);
        }
        //Allen--}

        //line = get_line(rsrc, sock); /* 226 Transfer complete */
        //safe_free(line);


        send_control(sock, "QUIT\r\n", NULL);
        line = get_line(rsrc, sock); /* 221 Goodbye */
        safe_free(line);


        fclose(out);
        //printf("before close sock\n");
        close(sock);
        close(data_sock);
        //printf("after close sock\n");
        return retval;
}

