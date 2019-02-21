#ifndef __BTDECODE_H
#define __BTDECODE_H
                                                                                                                                               
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/klog.h>
                                                                                                                                               
#ifndef uint8
#define uint8  unsigned char
#endif
                                                                                                                                               
#ifndef uint32
#define uint32 unsigned long int
#endif
                                                                                                                                               
typedef struct
{
    uint32 total[2];
    uint32 state[5];
    uint8 buffer[64];
}
sha1_context;

typedef struct _Files {
    long long unsigned length;
    struct _Files *next;
}Files;//added by eric 20120611,just for multi_file

typedef struct _Filess
{
    char path[256];
    int id;
    char name[256];
    unsigned long long length;
    struct _Filess *next;
}Filess;
                                                                                                                                               
void sha1_starts( sha1_context *ctx );
void sha1_update( sha1_context *ctx, uint8 *input, uint32 length );
void sha1_finish( sha1_context *ctx, uint8 digest[20] );
                                                                                                                                               
#define KEY_SP '|'      //the keyname list's delimiters
#define KEYNAME_SIZ 32
#define KEYNAME_LISTSIZ 256
#define MAX_INT_SIZ 64
#define QUERY_STR 0
#define QUERY_INT 1
#define QUERY_POS 2
                                                                                                                                               
#define meta_str(keylist,pstr,pint) decode_query(b,flen,(keylist),(pstr),(pint),QUERY_STR)
#define meta_int(keylist,pint) decode_query(b,flen,(keylist),(const char**) 0,(pint),QUERY_INT)
#define meta_pos(keylist) decode_query(b,flen,keylist,(const char**) 0,(int64_t*) 0,QUERY_POS)

#define MAX_METAINFO_FILESIZ 4194304
int multi_file;
char filename[256];
                                                                                                                                               
#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (uint32) (b)[(i)    ] << 24 )       \
        | ( (uint32) (b)[(i) + 1] << 16 )       \
        | ( (uint32) (b)[(i) + 2] <<  8 )       \
        | ( (uint32) (b)[(i) + 3]       );      \
}
                                                                                                                                               
#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8) ( (n)       );       \
}

size_t buf_int(const char *b,size_t len,char beginchar,char endchar,int64_t *pi);
size_t buf_str(const char *b,size_t len,const char **pstr,size_t* slen);
size_t decode_int(const char *b,size_t len);
size_t decode_str(const char *b,size_t len);
size_t decode_dict(const char *b,size_t len,const char *keylist);
size_t decode_list(const char *b,size_t len,const char *keylist);
size_t decode_rev(const char *b,size_t len,const char *keylist);
size_t decode_query(const char *b,size_t len,const char *keylist,const char **ps,int64_t *pi,int method);
size_t decode_list2path(const char *b, size_t n, char *pathname);
size_t bencode_buf(const char *str,size_t len,FILE *fp);
size_t bencode_str(const char *str, FILE *fp);
size_t bencode_int(const int integer, FILE *fp);
size_t bencode_begin_dict(FILE *fp);
size_t bencode_begin_list(FILE *fp);
size_t bencode_end_dict_list(FILE *fp);
size_t bencode_path2list(const char *pathname, FILE *fp);

int get_file_infohash(const char* metainfo_fname, char *infohash);
int get_file_name(const char* metainfo_fname, char *name);
int get_file_length(const char* metainfo_fname, int64_t *total_len);
int get_files_length_path(const char* metafile_name);
int get_files_path(const char* metainfo_fname, char *path);

char m_shake_buffer[68];

#endif
