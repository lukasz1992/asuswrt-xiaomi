/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#ifndef DM_URL_PARSER_H
#define DM_URL_PARSER_H

#define PROTOTYPES 1
#define HAVE_STDARG_H 1

//from option.h {
#define OPT_RESUME 	(1 << 0)
#define OPT_VERBOSE	(1 << 1)
#define OPT_QUIET	(1 << 2)
#define OPT_ACTIVE	(1 << 3)       	/* For FTP only */
#define OPT_NORESUME	(1 << 4)	/* see comments in option.c about
                                           this lameness */
#define OPT_PROGRESS	(1 << 5)	/* for python aka markus fleck */
#define OPT_BE_MOZILLA	(1 << 6)        /* To act like Mozilla */
#define OPT_BE_MSIE	(1 << 7)        /* To act like MSIE */
// form option.h }

#define VERSION_1 "7.0"               //from config.h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>


enum report_levels { DEBUG, WARN, ERR }; //from util.h

typedef struct _List 		List; //from list.h

struct _List {
        void *data;
        List *next;
};


typedef struct _UrlResource 	UrlResource;
typedef struct _Url		Url;

struct _Url {
    char *full_url;
	int service_type;
	char *username;
	char *password;
	char *host;
	int port;
    int status;
	char *path;
	char *file;
};

struct _UrlResource {
	Url *url;
	char *outfile;
    char *proxy;
    char *proxy_username;
    char *proxy_password;
	unsigned char options;
    long long int outfile_size; //modify by gauss
    long long int outfile_offset;
};


/* Service types */
enum url_services {
        SERVICE_HTTP = 1,
        SERVICE_FTP,
        SERVICE_GOPHER,
        SERVICE_FINGER,
        SERVICE_BT,
        SERVICE_NZB,
        SERVICE_ED2K,
        SERVICE_UNKNOW
};

enum parser_result {
     P_PASS = 1,
     P_FAIL,
     P_NEEDACCOUNT

};
                
/* Error string */

extern char *url_error;

/* Funcs */

#ifdef PROTOTYPES

Url *url_new(void);
void url_destroy(Url *);
Url *url_init(Url *, char *);
UrlResource *url_resource_new(void);
void url_resource_destroy(UrlResource *);
int is_probably_an_url(char *);

/* form list.h */
List *		list_new(void);
List *		list_append(List *, void *);

//from util.h
char *string_lowercase(char *);
void report(enum report_levels, char *, ...);
int tcp_connect(char *, int ,int);
char *strconcat(const char *, ...);

#endif /* PROTOTYPES */

#endif /* URL_H */

	
