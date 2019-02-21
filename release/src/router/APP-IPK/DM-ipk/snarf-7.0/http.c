/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/statvfs.h>
#include "url.h"
#include "http.h"
#include "options.h"
#include "util.h"
#include "llist.h"
#include "logs.h"

extern int default_opts;

int redirect_count = 0;
#define REDIRECT_MAX 10

#define USER_AGENT "snarf/" VERSION " (http://www.xach.com/snarf)"
#define MOZILLA_USER_AGENT "Mozilla/4.0 (X11; Unix; Hi-mom)"
#define MSIE_USER_AGENT "Mozilla/4.0 (Compatible; MSIE 4.0)"

typedef struct _HttpHeader 	HttpHeader;
typedef struct _HttpHeaderEntry HttpHeaderEntry;

struct _HttpHeader {
        List *header_list;
};

struct _HttpHeaderEntry {
        char *key;
        char *value;
};

extern int socket_write;
extern int socket_read;
extern char basic_path[256];


static HttpHeader *
make_http_header(char *r)
{
        HttpHeader *h		= NULL;
        HttpHeaderEntry	*he 	= NULL;
        char *s			= NULL;
        char *raw_header	= NULL;
        char *raw_header_head	= NULL;

        raw_header = strdup(r);
        /* Track this to free at the end */
        raw_header_head = raw_header;
        
        h = malloc(sizeof(HttpHeader));
        h->header_list = list_new();

        /* Skip the first line: "HTTP/1.X NNN Comment\r?\n" */
        s = raw_header;
        while (*s != '\0' && *s != '\r' && *s != '\n') {
                s++;
        }
        while (*s != '\0' && isspace(*s)) {
                s++;
        }

        raw_header = s;

        s = strstr(raw_header, ": ");
        while (s) {
                /* Set ':' to '\0' to terminate the key */
                *s++ = '\0'; 
                he = malloc(sizeof(HttpHeaderEntry));
                he->key = strdup(raw_header);
                /* Make it lowercase so we can lookup case-insensitive */
                string_lowercase(he->key);

                /* Now get the value */
                s++;
                raw_header = s;
                while (*s != '\0' && *s != '\r' && *s != '\n') {
                        s++;
                }
                *s++ = '\0';
                he->value = strdup(raw_header);
                list_append(h->header_list, he);

                /* Go to the next line */
                while (*s != '\0' && isspace(*s)) {
                        s++;
                }
                raw_header = s;
                s = strstr(raw_header, ": ");
        }

        free(raw_header_head);
        return h;
}


static void
free_http_header(HttpHeader *h)
{
        List *l;
        List *l1;
        HttpHeaderEntry *he;

        if (h == NULL) {
                return;
        }

        l = h->header_list;
        while (l && l->data) {
                he = l->data;
                free(he->key);
                free(he->value);
                free(l->data);
                l = l->next;
        }

        l = h->header_list;
        while (l) {
                l1 = l->next;
                free(l);
                l = l1;
        }
}
                        

static char *
get_header_value(char *key, HttpHeader *header)
{
        char *value		= NULL;
        char *retval 		= NULL;
        List *l			= NULL;
        HttpHeaderEntry *he	= NULL;

        l = header->header_list;

        while (l && l->data) {
                he = l->data;
                if (strcmp(he->key, key) == 0) {
                        return strdup(he->value);
                }
                l = l->next;
        }

        return NULL;
}


static char *
get_raw_header(int fd)
{
        char *header = NULL;
        char buf[BUFSIZE]; 	/* this whole function is pathetic. please
                                   rewrite it for me. */
        int bytes_read = 0;
        int total_read = 0;

        header = strdup("");

        buf[0] = buf[1] = buf[2] = '\0';

        while( (bytes_read = read(fd, buf, 1)) ) {
                total_read += bytes_read;

                header = strconcat(header, buf, NULL);
                if( total_read > 1) {
                        if( strcmp(header + (total_read - 2), "\n\n") == 0 )
                                break;
                }

                if( total_read > 3 ) {
                        if( strcmp(header + (total_read - 4), "\r\n\r\n") 
                            == 0 )
                                break;
                }
        }

        return header;
}

static char *
get_request(UrlResource *rsrc)
{
        char *request = NULL;
        char *auth = NULL;
        char buf[BUFSIZE];
        Url *u;
        //off_t file_size;
        long long int file_size;
        char *new_path = NULL;
        char *new_file = NULL;

        u = rsrc->url;

        new_path = strdup(u->path);
        new_file = strdup(u->file);

        if(strchr(u->path,' '))
        {
           free(new_path);
           new_path = space_escape(u->path);
        }

        if(strchr(u->file,' '))
        {
           free(new_file);
           new_file = space_escape(u->file);
        }

        //request = strconcat("GET ", u->path, u->file, " HTTP/1.0\r\n",
        request = strconcat("GET ", new_path, new_file, " HTTP/1.0\r\n",
                            "Host: ", u->host, "\r\n",
                            "Referer: ","http://",u->host,"/","\r\n",NULL);

        free(new_path);
        free(new_file);

        if( u->username && u->password ) {
                auth = strconcat( u->username, ":", u->password, NULL);
                auth = base64(auth, strlen(auth));
                request = strconcat(request, "Authorization: Basic ",
                                    auth, "\r\n", NULL);
        }

        if( rsrc->proxy_username && rsrc->proxy_password ) {
                auth = strconcat( rsrc->proxy_username, ":", 
                                  rsrc->proxy_password, NULL);
                auth = base64(auth, strlen(auth));
                request = strconcat(request, "Proxy-Authorization: Basic ",
                                    auth, "\r\n", NULL);
        }

        if( (rsrc->options & OPT_RESUME) 
            && (file_size = get_file_size(rsrc->outfile)) ) {
                sprintf(buf, "%lld-", (long  long int )file_size); //modify by gauss
                request = strconcat(request, "Range: bytes=", buf, "\r\n",
                                    NULL);
        }

        /* Use user's SNARF_HTTP_USER_AGENT env. var if present,
           as they might want to spoof some discriminant sites.
           (Or just increase the hit count for their favorite
           browser.)  Alternately, use Mozilla or MSIE's User-Agent
           strings based on options set.  */

        request = strconcat(request, "User-Agent: ", NULL);

        if (getenv("SNARF_HTTP_USER_AGENT")) {
            request = strconcat(request, getenv("SNARF_HTTP_USER_AGENT"), 
                                NULL);
        } else if (rsrc->options & OPT_BE_MOZILLA) {
            request = strconcat(request, MOZILLA_USER_AGENT, NULL);
        } else if (rsrc->options & OPT_BE_MSIE) {
            request = strconcat(request, MSIE_USER_AGENT, NULL);
        } else {            /* let snarf be snarf :-) */
            request = strconcat(request, USER_AGENT, NULL);
        }
        /* This CRLF pair closes the User-Agent key-value set. */
        request = strconcat(request, "\r\n", NULL);

        /* If SNARF_HTTP_REFERER is set, spoof it. */
        if (getenv("SNARF_HTTP_REFERER")) {
                request = strconcat(request, "Referer: ",
                                    getenv("SNARF_HTTP_REFERER"),
                                    "\r\n", NULL);
        }
        
        request = strconcat(request, "\r\n", NULL);

        return request;
}

int GetLocationIsType(char *str)
{
    //int len;

    char *s = NULL;

    if(str == NULL)
        return 0;
    s = str;

    if( strstr(str, "http://") )
    {
        s += 7;		/* skip past that part */

        while (*s != '/' && *s != '\0')
            s++;

        if(strlen(s) <= 1) //such as http://www.baidu.com or http://www.baidu.com/
            return 0;
        else
            return 1;      //nomal location;
    }
    else                   //such as "down.asp?id=463"
    {
        return 2;
    }

}

int IsCompleted(UrlResource *rsrc)
{
    char *request = NULL;
    int socket_read = 0;
    int bytes_read = 0;
    char buf[BUFSIZE];
    char *raw_header	= NULL;
    HttpHeader *header	= NULL;
    char *len_string 	= NULL;
    long long size,file_size;
    int sock;

    if( (rsrc->options & OPT_RESUME)
        && (file_size = get_file_size(rsrc->outfile)) )
    {
        if( ! (sock = tcp_connect(rsrc->url->host,rsrc->url->port)) ){
                err_log(rsrc, "connect sock error",HTTP);
                return 0;
        }

        rsrc->options = 0 ;
        request = get_request(rsrc);
        write(sock, request, strlen(request));
        socket_read = 1;
        bytes_read = read(sock, buf, 8);
        socket_read = 0;

        if( bytes_read == 0 ) {
                close(sock);
                err_log(rsrc, "read sock error",HTTP);
                return 0;
        }

        /* skip the header */
        buf[bytes_read] = '\0';
        raw_header = get_raw_header(sock);

        raw_header = strconcat(buf, raw_header, NULL);
        header = make_http_header(raw_header);

        len_string = get_header_value("content-length", header);

        if (len_string)
        {
                size = (long long int )atoll(len_string);
                if(size == rsrc->outfile_offset)
                {
                    return 1;
                }
        }
    }

    return 0;
}

int IsCompleted1(HttpHeader *header,UrlResource *rsrc)
{
    char *len_string 	= NULL;
    int i = 0;

    long long size = -1;

    len_string = get_header_value("content-range", header);


    if (len_string)
    {
            for(i=0;len_string[i] != '/';i++);
            size = (long long int )atoll(len_string+i+1);
            if(size == rsrc->outfile_offset)
            {
                return 1;
            }
       free(len_string);
    }
    return 0;
}


int
http_transfer(UrlResource *rsrc)
{
        FILE *out 		= NULL;
        Url *u			= NULL;
        Url *proxy_url		= NULL;
        Url *redir_u		= NULL;
        char *request		= NULL;
        char *raw_header	= NULL;
        HttpHeader *header	= NULL;
        char *len_string 	= NULL;
        char *new_location	= NULL;
        //char *content_disposition =NULL;
        char buf[BUFSIZE];
        int sock 		= 0;
        ssize_t bytes_read	= 0;
        int retval		= 0;
        int i;

        /* make sure we haven't recursed too much */

        if( redirect_count > REDIRECT_MAX ) {
                //report(ERR, "redirection max count exceeded " 
                //      "(looping redirect?)");
                redirect_count = 0;
                err_log(rsrc, "redirection max conut exceeded",HTTP);
                return 0;
        }


        /* make sure everything's initialized to something useful */
        u = rsrc->url;
     
        if( ! *(u->host) ) {
                //report(ERR, "no host specified");
                err_log(rsrc, "no host specified",HTTP);
                return 0;
        }

        /* fill in proxyness */
        if( !rsrc->proxy ) {
                rsrc->proxy = get_proxy("HTTP_PROXY");
        }

        if( !rsrc->outfile ) {
            if( u->file )
            {
                char real_name[256] = {0};
                get_real_download_filename(u->file,real_name);
                rsrc->outfile = strdup(real_name);
            }
            else
                rsrc->outfile = strdup("index.html");
        }

        if( !u->path )
                u->path = strdup("/");

        if( !u->file )
                u->file = strdup("");  /* funny looking */

        if( !u->port )
                u->port = 80;

        rsrc->options |= default_opts;
                
        /* send the request to either the proxy or the remote host */
        if( rsrc->proxy ) {
                proxy_url = url_new();
                url_init(proxy_url, rsrc->proxy);
                
                if( !proxy_url->port ) 
                        proxy_url->port = 80;

                if( !proxy_url->host ) {
                        //report(ERR, "bad proxy `%s'", rsrc->proxy);
                        err_log(rsrc, "bad proxy",HTTP);
                        return 0;
                }

                if( proxy_url->username )
                        rsrc->proxy_username = strdup(proxy_url->username);

                if( proxy_url->password )
                        rsrc->proxy_password = strdup(proxy_url->password);

                /* Prompt for proxy password if not specified */
                if( proxy_url->username && !proxy_url->password ) {
                        char *prompt = NULL;
                        prompt = strconcat("Password for proxy ",
                                           proxy_url->username, "@",
                                           proxy_url->host, ": ", NULL);
                        proxy_url->password = strdup(getpass(prompt));
                        free(prompt);
                }

                if( ! (sock = tcp_connect(proxy_url->host, proxy_url->port)) ){
                        err_log(rsrc, "proxy connect sock error",HTTP);
                        return 0;
		}


                u->path = strdup("");
                u->file = strdup(u->full_url);
                request = get_request(rsrc);

                write(sock, request, strlen(request));

        } else /* no proxy */ {



                if( ! (sock = tcp_connect(u->host, u->port)) ){
                        err_log(rsrc, "connect sock error",HTTP);
                        return 0;
		}

                request = get_request(rsrc);

                socket_write = 1;
                write(sock, request, strlen(request));
                socket_write = 0;
        }

        
        out = open_outfile(rsrc);
        if( !out ) {
                //report(ERR, "opening %s: %s", rsrc->outfile, strerror(errno));
                err_log(rsrc, "open error",HTTP);
                return 0;
        }

        /* check to see if it returned a HTTP 1.x response */
        memset(buf, '\0', 5);

        socket_read = 1;
        bytes_read = read(sock, buf, 8);
        socket_read = 0;

        if( bytes_read == 0 ) {
                close(sock);
                err_log(rsrc, "read sock error",HTTP);
                if(IsCompleted(rsrc))
                    return -3;
                else
                    return 0;
        }

        if( ! (buf[0] == 'H' && buf[1] == 'T' 
               && buf[2] == 'T' && buf[3] == 'P') ) {
                if ((rsrc->options & OPT_RESUME) && 
                    rsrc->outfile_offset) {
                        report(WARN,
                               "server does not support resume, "
                               "try again"
                               " with -n (no resume)");
                        retval = 0;
                        goto cleanup;
                }
                write(fileno(out), buf, bytes_read);
        } else {
                /* skip the header */
                buf[bytes_read] = '\0';
                raw_header = get_raw_header(sock);
                raw_header = strconcat(buf, raw_header, NULL);
                header = make_http_header(raw_header);

                if (rsrc->options & OPT_VERBOSE) {
                        fwrite(raw_header, 1, strlen(raw_header), stderr);
                }

                /* check for redirects */
                new_location = get_header_value("location", header);

              if (raw_header[9] == '3') {
                  int type = GetLocationIsType(new_location);
                  if(type != 0)
                  {
                      if(type == 2)
                      {
                         new_location = strconcat("http://",rsrc->url->host,"/",new_location,NULL);
                      }

                      redir_u = url_new();

                      /* make sure we still send user/password along */
                      redir_u->username = safe_strdup(u->username);
                      redir_u->password = safe_strdup(u->password);

                      url_init(redir_u, new_location);
                      rsrc->url = redir_u;
                      redirect_count++;
                      retval = transfer(rsrc);
                      goto cleanup;
                  }

                }

                if (raw_header[9] == '4' || raw_header[9] == '5') {
                        if(raw_header[9] == '4' && raw_header[10] == '1' && raw_header[11] == '6')
                        {
                            IsCompleted1(header,rsrc);
                            retval = -3;
                        }
                        else
                            retval = 0;

                        for(i = 0; raw_header[i] && raw_header[i] != '\n'; i++);
                        raw_header[i] = '\0';
                        report(ERR, "HTTP error from server: %s", raw_header);
                        err_log(rsrc,"HTTP error from server",HTTP);

                        goto cleanup;
                }
                        
                len_string = get_header_value("content-length", header);

                if (len_string)
                        //rsrc->outfile_size = (off_t )atoi(len_string);
                        rsrc->outfile_size = (long long int )atoll(len_string); //add by gauss

                if (get_header_value("content-range", header))
                        rsrc->outfile_size += rsrc->outfile_offset;

                if( (!rsrc->outfile_size) && 
                    (rsrc->options & OPT_RESUME) && 
                    !(rsrc->options & OPT_NORESUME)
                    && rsrc->outfile_offset ) {
                        report(WARN,
                             "unable to determine remote file size, try again"
                             " with -n (no resume)");
                        retval = 0;
                        goto cleanup;
                }
        }
#if 1
        /* check disk space is full*/
        struct statvfs diskdata;
        long long int free_disk_space;
        if (!statvfs(basic_path, &diskdata))
        {
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
#endif

        retval = dump_data(rsrc, sock, out);
        return retval;
                        
 cleanup:
        free_http_header(header);
        close(sock); fclose(out);
        return retval;

}

