/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#ifndef URL_H
#define URL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct _UrlResource 	UrlResource;
typedef struct _Url		Url;

struct _Url {
        char *full_url;
	int service_type;
	char *username;
	char *password;
	char *host;
	int port;
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
        //off_t outfile_size;
        long long int outfile_size; //modify by gauss
        //off_t outfile_offset;
        long long int outfile_offset;
};


/* Service types */
enum url_services {
        SERVICE_HTTP = 1,
        SERVICE_FTP,
        SERVICE_GOPHER,
        SERVICE_FINGER
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

#endif /* PROTOTYPES */

#endif /* URL_H */

	
