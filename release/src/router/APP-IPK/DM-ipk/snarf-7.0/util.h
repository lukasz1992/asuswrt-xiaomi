/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include <sys/types.h>
#include <stdio.h>
#include "url.h"
#include "config.h"

#ifndef UTIL_H
#define UTIL_H

#define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
typedef struct _Progress Progress;

struct _Progress {
        unsigned char tty;	/* if we have a tty */
        //long int length;	/* total length */
        //long int current;	/* current position */
        //long int offset;	/* if we're resuming, save the offset */
        long long int length;   //modify by gauss for 2GB
        long long int current;
        long long int offset;

        int max_hashes;		/* max number of hashes to print */
        int cur_hashes;		/* total hashes printed so far*/
        int overflow;		/* save the remainder */
        unsigned char frame;	/* frame for the spinny animation */
        double start_time;	/* for calculating k/sec */
        UrlResource *rsrc;	/* Info such as file name and offset */
};

enum report_levels { DEBUG, WARN, ERR };

#ifdef PROTOTYPES

Progress *progress_new(void);
//int progress_init(Progress *, UrlResource *, long int);
int progress_init(Progress *, UrlResource *, long long int);  //modify by gauss
//void progress_update(Progress *, long int);
void progress_update(Progress *, long long int);
void progress_destroy(Progress *);
double double_time(void);

char *string_lowercase(char *);
char *get_proxy(const char *);
int dump_data(UrlResource *, int, FILE *);
char *strconcat(const char *, ...);
char *base64(char *, int);
void report(enum report_levels, char *, ...);
int tcp_connect(char *, int);
//off_t get_file_size(const char *);
long long int get_file_size(const char *);
void repchar(FILE *fp, char ch, int count);
int transfer(UrlResource *rsrc);

void err_log(UrlResource *rsrc, char *err,int type);

#ifndef HAVE_STRDUP
char *strdup(const char *s);
#endif

#endif /* PROTOTYPES */

extern int debug_enabled;
int do_chmod;
int sv_fd;
int err_counts;
int stopp;      // 2007_fix
int writing_log;

#define open_outfile(x)  (((x)->outfile[0] == '-') ? stdout : real_open_outfile(x))
#define real_open_outfile(x)  (((x)->options & OPT_RESUME && !((x)->options & OPT_NORESUME)) ? (fopen((x)->outfile, "a")) : (fopen((x)->outfile, "w")))

#define safe_free(x)		if(x) free(x)
#define safe_strdup(x)		( (x) ? strdup(x) : NULL )
#define BUFSIZE (5*2048)

char *my_nstrchr(const char chr,char *str,int n);//2012.12.20 magic added
char *oauth_url_escape(const char *string);
char *space_escape(const char *string);
char *oauth_url_unescape(const char *string, size_t *olen);
void final_log(char *logName,char *complete_path,UrlResource *rsrc);
int handle_complete(UrlResource *rsrc,char *logName,char *complete_path,char *basepath);

#endif
