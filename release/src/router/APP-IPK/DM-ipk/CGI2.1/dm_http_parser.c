/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <errno.h>
#include "dm_url_parser.h"
#include "dm_http_parser.h"

#define safe_strdup(x)		( (x) ? strdup(x) : NULL )

extern int default_opts;

int redirect_count = 0;
char *data = NULL;
char *content_type = NULL;
#define REDIRECT_MAX 10

#define USER_AGENT "snarf/" VERSION_1 " (http://www.xach.com/snarf)"
#define MOZILLA_USER_AGENT "Mozilla/4.0 (X11; Unix; Hi-mom)"
#define MSIE_USER_AGENT "Mozilla/4.0 (Compatible; MSIE 4.0)"
#define BUFSIZE (5*2048)
#define TIMEOUT 3
# define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))

typedef struct _HttpHeader 	HttpHeader;
typedef struct _HttpHeaderEntry HttpHeaderEntry;

struct _HttpHeader {
        List *header_list;
};

struct _HttpHeaderEntry {
        char *key;
        char *value;
};

static void *xmalloc_fatal(size_t size) {
    if (size==0) return NULL;
    fprintf(stderr, "Out of memory.");
    //exit(1);
    return;
}

void *xmalloc (size_t size) {
    void *ptr = malloc (size);
    if (ptr == NULL) return xmalloc_fatal(size);
    return ptr;
}

void *xcalloc (size_t nmemb, size_t size) {
    void *ptr = calloc (nmemb, size);
    if (ptr == NULL) return xmalloc_fatal(nmemb*size);
    return ptr;
}

void *xrealloc (void *ptr, size_t size) {
    void *p = realloc (ptr, size);
    if (p == NULL) return xmalloc_fatal(size);
    return p;
}

char *xstrdup (const char *s) {
    void *ptr = xmalloc(strlen(s)+1);
    strcpy (ptr, s);
    return (char*) ptr;
}

char *
base64(char *bin, int len)
{
	char *buf= (char *)malloc((len+2)/3*4+1);
	int i=0, j=0;

		char BASE64_END = '=';
		char base64_table[64]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789+/";

	while( j < len - 2 ) {
		buf[i++]=base64_table[bin[j]>>2];
		buf[i++]=base64_table[((bin[j]&3)<<4)|(bin[j+1]>>4)];
		buf[i++]=base64_table[((bin[j+1]&15)<<2)|(bin[j+2]>>6)];
		buf[i++]=base64_table[bin[j+2]&63];
		j+=3;
	}

	switch ( len - j ) {
		case 1:
		buf[i++] = base64_table[bin[j]>>2];
		buf[i++] = base64_table[(bin[j]&3)<<4];
		buf[i++] = BASE64_END;
		buf[i++] = BASE64_END;
		break;
		case 2:
		buf[i++] = base64_table[bin[j] >> 2];
		buf[i++] = base64_table[((bin[j] & 3) << 4)
									   | (bin[j + 1] >> 4)];
		buf[i++] = base64_table[(bin[j + 1] & 15) << 2];
		buf[i++] = BASE64_END;
		break;
		case 0:
		  break;
	}
	buf[i]='\0';
	return buf;
}

char *oauth_url_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;

        switch(in){
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': case '~': case '.': case '-':
            ns[strindex++]=in;
            break;
        default:
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
            break;
        }
        string++;
    }
    ns[strindex]=0;
    return ns;
}

char *space_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;
        if(in != ' ')
        {
            ns[strindex++]=in;
        }
        else
        {
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
        }

        string++;
    }
    ns[strindex]=0;
    return ns;
}

char *oauth_url_unescape(const char *string, size_t *olen) {
    size_t alloc, strindex=0;
    char *ns = NULL;
    unsigned char in;
    long hex;

    if (!string) return NULL;
    alloc = strlen(string)+1;
    ns = (char*) xmalloc(alloc);

    while(--alloc > 0) {
        in = *string;
        if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
            char hexstr[3]; // '%XX'
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;
            hex = strtol(hexstr, NULL, 16);
            in = (unsigned char)hex; /* hex is always < 256 */
            string+=2;
            alloc-=2;
        }
        ns[strindex++] = in;
        string++;
    }
    ns[strindex]=0;
    if(olen) *olen = strindex;
    return ns;
}

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

        int head_end = 0;
        HttpHeader *head	= NULL;

        header = strdup("");

        data = NULL;
        

        buf[0] = buf[1] = buf[2] = '\0';

        while( (bytes_read = read(fd, buf, 1)) ) {
                total_read += bytes_read;


               //if(head_end == 0)
                    header = strconcat(header, buf, NULL);
               //else {
                     //fprintf(stderr,"obtain data=%s,buf=%s \n",data,buf);
                     //data = strconcat(data,buf,NULL);

               //}

               //fprintf(stderr,"header=%s\n",header);
               //fprintf(stderr,"read_buf=%s,total_read=%d\n",buf,total_read);

                if( total_read > 1) {
                        if( strcmp(header + (total_read - 2), "\n\n") == 0 )
                                break;
                }

                if( total_read > 3 ) {
                        if( strcmp(header + (total_read - 4), "\r\n\r\n")== 0 )
                            {

                                //if(content_type == NULL )
                                            // break;

                                /*if(strcmp(content_type,"text/html") == 0)
                               {
                                    //fprintf(stderr,"set head end to true \n");
                                    head_end = 1;
                                    data = strdup("");
                                    //fprintf(stderr,"~~~~~~~~data is %s ~~~~~~~~~~\n",data);
                                }*/
                                //else{
                                        break;
                                //}
                            }
                }


        }
        //fprintf(stderr,"~~~~~~~~data is %s ~~~~~~~~~~\n",data);
        return header;
}

char *
get_raw_url(char *data)
{
       //char *url = NULL;
       char url[512];
       memset(url,0,sizeof(url));

       char *p = strstr(data,"HTTP-EQUIV=\"REFRESH\"");

        //fprintf(stderr,"HTTP-EQUIV is %s \n",p);

        if(p)
        {
            char *p1 = strstr(p,"url=");
            //fprintf(stderr,"url is %s \n",p1);
            if(p1)
            {
                p1 = p1+4;

                char *p2 = strrchr(p1,'"');

                //fprintf(stderr,"duo yu is %s \n",p2);

                if(p2)
                {
                    size_t len = strlen(p1)-strlen(p2);
                    //fprintf(stderr,"copy length is %d \n",len);

                    strncpy(url,p1,len);
                    //memcpy(url,p1,len);
                    //fprintf(stderr,"final url  is %s \n",url);
                }



            }

        }
        return strdup(url);
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

		if( u->username && u->password ) {
				auth = strconcat( u->username, ":", u->password, NULL);
				auth = base64(auth, strlen(auth));
				request = strconcat(request, "Authorization: Basic ",
									auth, "\r\n", NULL);
		}

        free(new_path);
        free(new_file);

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

int
http_transfer(UrlResource *rsrc)
{

        FILE *out 		= NULL;
        Url *u			= NULL;

        Url *redir_u		= NULL;
        char *request		= NULL;
        char *raw_header	= NULL;
        HttpHeader *header	= NULL;

        char *new_location	= NULL;
        char buf[BUFSIZE];
        int sock 		= 0;
        ssize_t bytes_read	= 0;
        int retval		= 0;
        int i;

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        /* make sure we haven't recursed too much */

        if( redirect_count > REDIRECT_MAX ) {
                report(ERR, "redirection max count exceeded "
                      "(looping redirect?)");
                redirect_count = 0;
                //return 0;
                rsrc->url->status = P_FAIL;
                return  P_FAIL;
        }


        /* make sure everything's initialized to something useful */
        u = rsrc->url;

        if( ! *(u->host) ) {
                report(ERR, "no host specified");
                rsrc->url->status = P_FAIL;
                return  P_FAIL;
        }


        if( !u->path )
                u->path = strdup("/");

        if( !u->file )
                u->file = strdup("");  /* funny looking */

        if( !u->port )
                u->port = 80;

        if( !(sock = tcp_connect(u->host, u->port)) ){
             rsrc->url->status = P_FAIL;
             return  P_FAIL;        
                }

        request = get_request(rsrc);

        if ( setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval)) )
        {
            rsrc->url->status = P_FAIL;
            return  P_FAIL;
        }

        int write_len = write(sock, request, strlen(request));
        if(write_len == -1 && errno == EAGAIN )
        {
            rsrc->url->status = P_FAIL;
            return  P_FAIL;
        }

        /* check to see if it returned a HTTP 1.x response */
        memset(buf, '\0', 5);

        bytes_read = read(sock, buf, 8);

        if(bytes_read == -1 && errno == EAGAIN)
        {
            rsrc->url->status = P_FAIL;
            return  P_FAIL;
        }

        if( bytes_read == 0 ) {
                close(sock);
                rsrc->url->status = P_FAIL;
                return  P_FAIL;
        }

        if( ! (buf[0] == 'H' && buf[1] == 'T'
               && buf[2] == 'T' && buf[3] == 'P') ) {
               rsrc->url->status = P_FAIL;
               return  P_FAIL;

        } else {
                /* skip the header */
                buf[bytes_read] = '\0';
                raw_header = get_raw_header(sock);
                raw_header = strconcat(buf, raw_header, NULL);
                header = make_http_header(raw_header);

                /* check for redirects */
                new_location = get_header_value("location", header);

                //if (raw_header[9] == '3' && new_location ) {
                if (raw_header[9] == '3' ) {
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

                            if( strlen(redir_u->host) == 0 )
                            {
                                redir_u = url_new();
                                new_location = strconcat("http://",rsrc->url->host,new_location,NULL);
                                url_init(redir_u, new_location);
                            }


                            rsrc->url = redir_u;
                            redirect_count++;

                            http_transfer(rsrc);
                            //goto cleanup;
                            //free_http_header(header);
                            //close(sock);
                        }

                }

                if (raw_header[9] == '4' || raw_header[9] == '5') {
                        for(i = 0; raw_header[i] && raw_header[i] != '\n'; i++);
                        raw_header[i] = '\0';
						report(ERR, "HTTP error from server: %s\n", raw_header);
                        free_http_header(header);
                        close(sock);
                        rsrc->url->status = P_NEEDACCOUNT;
                        return P_NEEDACCOUNT;
                }


        }

        /*
        if( ! dump_data(rsrc, sock, out) )
                retval = 0;
        else
                retval = 1;
        */

        free_http_header(header);
        close(sock);

        char *new_url = NULL;
        if(data != NULL)
        {
            new_url = get_raw_url(data);
            if(strlen(new_url) > 0)
                rsrc->url->full_url = new_url;
         }

        char bt_type[10], nzb_type[10] ;
        memset(bt_type,0,sizeof(bt_type));
        memset(nzb_type,0,sizeof(nzb_type));

        strcpy(bt_type,u->full_url+strlen(u->full_url)-8);
        strcpy(nzb_type,u->full_url+strlen(u->full_url)-4);

        if(strcmp(bt_type,".torrent") == 0) {
            u->service_type = SERVICE_BT;
        }

        if(strcmp(nzb_type,".nzb") == 0)
        {
            u->service_type = SERVICE_NZB;
        }

        if(u->service_type == SERVICE_UNKNOW)
            u->service_type = SERVICE_HTTP;
        rsrc->url->status = P_PASS;

        return P_PASS;

}

