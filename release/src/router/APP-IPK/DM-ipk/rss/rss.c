#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <bits/signum.h>
#include <libgen.h>
#include <time.h>
#include "rss.h"

pthread_cond_t cond;
pthread_mutex_t mutex;

/*Handle for the SIGTERM and SIGINT signal*/

void sigterm(int sig)
{
    signal(sig, SIG_IGN);
    fprintf(stderr,"\n received signal %d Goodbye\n", sig);
    quitting = 1;
    run_again = 0;
    pthread_cond_signal(&cond);
}

void sigusr1(int sig)
{
    fprintf(stderr,"\n received signal %d, User change rss setting!!\n", sig);
    quitting = 1;
    run_again = 1;
    pthread_cond_signal(&cond);
}

void my_mkdir(char *path)
{
    //char error_message[256];
    //printf("my_mkdir path = %s\n",path);
    DIR *dir;
    if(NULL == (dir = opendir(path)))
    {
        if(-1 == mkdir(path,0777))
        {
            printf("please check disk can write or dir has exist???");
            printf("mkdir %s fail\n",path);
            return;
        }
    }
    else
        closedir(dir);
}

char *my_str_malloc(size_t len){

    char *s;
    s = (char *)malloc(sizeof(char)*len);
    if(s == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    memset(s,'\0',sizeof(char)*len);
    return s;
}

int init()
{
    /*get base path*/
    char asusware_path[128];
    memset(asusware_path, 0, sizeof(asusware_path));
    realpath("/tmp/opt/etc", asusware_path);

    base_path = strdup(dirname(asusware_path));//basename();
    base_path = strdup(dirname(base_path));
    //fprintf(stderr,"\nbase path=%s\n", base_path);
    sprintf(rss_file,"%s/Download2/RSS/rssxml", base_path);
    sprintf(rss_path, "%s/Download2/RSS", base_path);
    sprintf(jqs_path, "%s/Download2/.logs/rss.jqs", base_path);
    sprintf(rss_date_file,"%s/Download2/RSS/rss_data_file",base_path);

    memset(logurl,0,sizeof(logurl));

    my_mkdir(rss_path);

    if(access(jqs_path,F_OK))
    {
        FILE *fp;
        fp = fopen(jqs_path,"w");
        if(fp)
            fclose(fp);
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    urlcount = 0;
    namecount = 0;
    quitting = 0;
    run_again = 0;

    //    int i;
    //    fprintf(stderr,"\nrss_check_time=%d\n",rss_check_time);
    //    for(i = 0;i < urlcount; i++)
    //        fprintf(stderr,"\nurl=%s\n", urllist[i]);

    //    for(i = 0;i < namecount; i++)
    //        fprintf(stderr,"\nname=%s\n", namelist[i]);

    return 0;
}

int read_config()
{
    if(access(CONFIG_FILE, F_OK) == 0)
    {
        FILE *fp = fopen(CONFIG_FILE, "r");
        if(fp == NULL)
            return -1;

        char content[2048];
        int i;
        memset(content, 0, sizeof(content));
        while(fgets(content, 2048, fp))
        {
            if(content[strlen(content) - 1] == '\n')
                content[strlen(content) - 1] = '\0';

            for(i = 0;i < strlen(content); i++)
            {
                if(content[i] == '=')
                    break;
            }

            //fprintf(stderr,"content=%s\n",content);
            if(strncmp(content, "rss_check_time", 14) == 0) {
                rss_check_time = atoi(content + i + 1);
                continue;
            }

            if(strncmp(content, "rss_url", 7) == 0)
            {
                char *nv, *nvp, *b;
                nv = nvp = strdup(content + 8);
                if(nv) {
                    while((b = strsep(&nvp, "<")) != NULL) {
                        if(!strlen(b))
                            continue;
                        strncpy(urllist[urlcount], b ,128);
                        urlcount++;
                    }
                }
                if(nv)
                    free(nv);
            }

            if(strncmp(content, "rss_name", 8) == 0)
            {
                char *nv2, *nvp2, *c;
                nv2 = nvp2 = strdup(content + 9);
                if(nv2) {
                    while((c = strsep(&nvp2, "<")) != NULL) {
                        if(!strlen(c))
                            continue;
                        strncpy(namelist[namecount], c ,128);
                        namecount++;
                    }
                }
                if(nv2)
                    free(nv2);
            }
        }
        fclose(fp);

        if(urlcount > 0)
            return 1;
        else
        {
            quitting = 1;
            unlink(rss_date_file);
            unlink(LOG_FILE);
            return 0;
        }
    }
    else
    {
        quitting = 1;
        return 0;
    }
}

int read_date_file()
{
    int i,j;
    FILE *fp;
    char buf[256];
    fp = fopen(rss_date_file,"r");
    if(fp)
    {
        while(fgets(buf,256,fp))
        {
            if(buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = '\0';
            char *p,*rss_url;
            p = buf;
            if(NULL != (rss_url = strsep(&p,"<")))
            {
                for(i=0;i<urlcount;i++)
                {
                    if(!strcmp(urllist[i],rss_url))
                    {
                        sprintf(new_pubdate[i], "%s", p);
                    }
                }
            }
        }
        fclose(fp);
    }
    return 0;
}

/*get rss xml file*/
int get_rss_file(char *rss_path)
{
    rss_xml_get_success = 1;
    FILE *fp;
    fp = fopen(rss_file, "wb");
    if(fp == NULL)
    {
        printf("open rssxml error 1\n");
        return -1;
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if(curl)
    {
        struct curl_slist *headers_l=NULL;
        static const char header1_l[]="User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0";

        headers_l=curl_slist_append(headers_l, header1_l);
        curl_easy_setopt(curl, CURLOPT_URL, rss_path);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_l);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
        res = curl_easy_perform(curl);
        if(res != 0)
        {
            rss_xml_get_success = 0;
            fclose(fp);
            return -1;
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers_l);
    }
    fclose(fp);
}

//SHA1 function
typedef struct
{
    unsigned long int total[2];
    unsigned long int state[5];
    unsigned char buffer[64];
}
SHA1_CTX;

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (unsigned long int) (b)[(i)    ] << 24 )       \
    | ( (unsigned long int) (b)[(i) + 1] << 16 )       \
    | ( (unsigned long int) (b)[(i) + 2] <<  8 )       \
    | ( (unsigned long int) (b)[(i) + 3]       );      \
    }

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
    }

void SHA1Init( SHA1_CTX *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process( SHA1_CTX *ctx, unsigned char data[64] )
{
    unsigned long int temp, W[16], A, B, C, D, E;

    GET_UINT32( W[0],  data,  0 );
    GET_UINT32( W[1],  data,  4 );
    GET_UINT32( W[2],  data,  8 );
    GET_UINT32( W[3],  data, 12 );
    GET_UINT32( W[4],  data, 16 );
    GET_UINT32( W[5],  data, 20 );
    GET_UINT32( W[6],  data, 24 );
    GET_UINT32( W[7],  data, 28 );
    GET_UINT32( W[8],  data, 32 );
    GET_UINT32( W[9],  data, 36 );
    GET_UINT32( W[10], data, 40 );
    GET_UINT32( W[11], data, 44 );
    GET_UINT32( W[12], data, 48 );
    GET_UINT32( W[13], data, 52 );
    GET_UINT32( W[14], data, 56 );
    GET_UINT32( W[15], data, 60 );
#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
    (                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
    W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
    )

#define P(a,b,c,d,e,x)                                  \
    {                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P( A, B, C, D, E, W[0]  );
    P( E, A, B, C, D, W[1]  );
    P( D, E, A, B, C, W[2]  );
    P( C, D, E, A, B, W[3]  );
    P( B, C, D, E, A, W[4]  );
    P( A, B, C, D, E, W[5]  );
    P( E, A, B, C, D, W[6]  );
    P( D, E, A, B, C, W[7]  );
    P( C, D, E, A, B, W[8]  );
    P( B, C, D, E, A, W[9]  );
    P( A, B, C, D, E, W[10] );
    P( E, A, B, C, D, W[11] );
    P( D, E, A, B, C, W[12] );
    P( C, D, E, A, B, W[13] );
    P( B, C, D, E, A, W[14] );
    P( A, B, C, D, E, W[15] );
    P( E, A, B, C, D, R(16) );
    P( D, E, A, B, C, R(17) );
    P( C, D, E, A, B, R(18) );
    P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P( A, B, C, D, E, R(20) );
    P( E, A, B, C, D, R(21) );
    P( D, E, A, B, C, R(22) );
    P( C, D, E, A, B, R(23) );
    P( B, C, D, E, A, R(24) );
    P( A, B, C, D, E, R(25) );
    P( E, A, B, C, D, R(26) );
    P( D, E, A, B, C, R(27) );
    P( C, D, E, A, B, R(28) );
    P( B, C, D, E, A, R(29) );
    P( A, B, C, D, E, R(30) );
    P( E, A, B, C, D, R(31) );
    P( D, E, A, B, C, R(32) );
    P( C, D, E, A, B, R(33) );
    P( B, C, D, E, A, R(34) );
    P( A, B, C, D, E, R(35) );
    P( E, A, B, C, D, R(36) );
    P( D, E, A, B, C, R(37) );
    P( C, D, E, A, B, R(38) );
    P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P( A, B, C, D, E, R(40) );
    P( E, A, B, C, D, R(41) );
    P( D, E, A, B, C, R(42) );
    P( C, D, E, A, B, R(43) );
    P( B, C, D, E, A, R(44) );
    P( A, B, C, D, E, R(45) );
    P( E, A, B, C, D, R(46) );
    P( D, E, A, B, C, R(47) );
    P( C, D, E, A, B, R(48) );
    P( B, C, D, E, A, R(49) );
    P( A, B, C, D, E, R(50) );
    P( E, A, B, C, D, R(51) );
    P( D, E, A, B, C, R(52) );
    P( C, D, E, A, B, R(53) );
    P( B, C, D, E, A, R(54) );
    P( A, B, C, D, E, R(55) );
    P( E, A, B, C, D, R(56) );
    P( D, E, A, B, C, R(57) );
    P( C, D, E, A, B, R(58) );
    P( B, C, D, E, A, R(59) );

#undef K
#undef F
#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P( A, B, C, D, E, R(60) );
    P( E, A, B, C, D, R(61) );
    P( D, E, A, B, C, R(62) );
    P( C, D, E, A, B, R(63) );
    P( B, C, D, E, A, R(64) );
    P( A, B, C, D, E, R(65) );
    P( E, A, B, C, D, R(66) );
    P( D, E, A, B, C, R(67) );
    P( C, D, E, A, B, R(68) );
    P( B, C, D, E, A, R(69) );
    P( A, B, C, D, E, R(70) );
    P( E, A, B, C, D, R(71) );
    P( D, E, A, B, C, R(72) );
    P( C, D, E, A, B, R(73) );
    P( B, C, D, E, A, R(74) );
    P( A, B, C, D, E, R(75) );
    P( E, A, B, C, D, R(76) );
    P( D, E, A, B, C, R(77) );
    P( C, D, E, A, B, R(78) );
    P( B, C, D, E, A, R(79) );

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

void SHA1Update( SHA1_CTX *ctx, unsigned char *input, unsigned long int length )
{
    unsigned long int left, fill;

    if( ! length ) return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;
    if( ctx->total[0] < length )
        ctx->total[1]++;

    if( left && length >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        sha1_process( ctx, ctx->buffer );
        length -= fill;
        input  += fill;
        left = 0;
    }

    while( length >= 64 )
    {
        sha1_process( ctx, input );
        length -= 64;
        input  += 64;
    }

    if( length )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, length );
    }
}

static unsigned char sha1_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void SHA1Final( SHA1_CTX *ctx, unsigned char digest[20] )
{
    unsigned long int last, padn;
    unsigned long int high, low;
    unsigned char msglen[8];
    high = ( ctx->total[0] >> 29 )
            | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32( high, msglen, 0 );
    PUT_UINT32( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    SHA1Update( ctx, sha1_padding, padn );
    SHA1Update( ctx, msglen, 8 );

    PUT_UINT32( ctx->state[0], digest,  0 );
    PUT_UINT32( ctx->state[1], digest,  4 );
    PUT_UINT32( ctx->state[2], digest,  8 );
    PUT_UINT32( ctx->state[3], digest, 12 );
    PUT_UINT32( ctx->state[4], digest, 16 );
}
//SHA function

char * get_hash(char *filename)
{
    long i;
    FILE *fp = fopen(filename,"rb");
    if(fp == NULL)
    {
        fprintf(stderr,"file open failed\n");
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    unsigned long filesize = ftell(fp);
    if(filesize == -1)
    {
        fprintf(stderr,"file fseek failed\n");
        return NULL;
    }
    char *metafile_content = (char *)malloc(filesize+1);
    if(metafile_content == NULL)
    {
        fprintf(stderr,"mem malloc failed\n");
        return NULL;
    }

    fseek(fp,0,SEEK_SET);
    for(i=0;i<filesize;i++)
        metafile_content[i] = fgetc(fp);
    metafile_content[i] = '\0';

    fclose(fp);

    int push_pop = 0;
    long begin, end;

    for(i=0;i<filesize-strlen("4:info");i++)
    {
        if(memcmp(&metafile_content[i],"4:info",strlen("4:info")) == 0)
        {
            break;
        }
    }

    i = i + 6;
    begin = i;
    for(;i < filesize;)
    {
        if(metafile_content[i] == 'd')
        {
            push_pop++;
            i++;
        }
        else if(metafile_content[i] == 'l')
        {
            push_pop++;
            i++;
        }
        else if(metafile_content[i] == 'i')
        {
            i++;
            if(i == filesize)   return NULL;
            while(metafile_content[i] != 'e')
            {
                if((i+1) == filesize)
                    return NULL;
                else
                    i++;
            }
            i++;
        }
        else if((metafile_content[i] >= '0') && (metafile_content[i] <= '9'))
        {
            int number = 0;
            while((metafile_content[i] >= '0') && (metafile_content[i] <= '9'))
            {
                number = number*10 + metafile_content[i] - '0';
                i++;
            }
            i++;
            i = i+number;
        }
        else if(metafile_content[i] == 'e')
        {
            push_pop--;
            if(push_pop == 0)
            {
                end = i;
                break;
            }
            else
                i++;
        }
        else
        {
            return NULL;
        }
    }

    if(i == filesize)
        return NULL;

    char info[20];

    SHA1_CTX context;
    SHA1Init(&context);
    SHA1Update(&context, &metafile_content[begin], end-begin + 1);
    SHA1Final(&context,info);

    int j = 0;
    unsigned char *info_hash = (char *)malloc(40);
    memset(info_hash, 0, 40);

    for(i=0;i<20;i++)
        sprintf(info_hash+(2*j++),"%02X",info[i]&0xff);

    if(metafile_content != NULL)
        free(metafile_content);

    //fprintf(stderr,"infohash is %s\n",info_hash);
    return info_hash;
}

void create_lock_file()
{
    char path[14];
    FILE *fp;
    snprintf(path, sizeof(path), "%s", "/tmp/rss_lock");
    fp = fopen(path, "w");
    if(fp)
        fclose(fp);
}

void initjqs()
{
    jqs_count = 0;
    FILE *fp = fopen(jqs_path, "r");
    if(fp == NULL) {
        fprintf(stderr, "open rss_jqs_file failed.\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char buf[length + 1];//buf内容： ;1.torrent;2.torrent;3.torrent
    memset(buf, 0 ,sizeof(buf));
    fread(buf, 1, length, fp);
    fclose(fp);

    jqs_count = 0;
    char *b;
    char *nvp = buf;
    while((b = strsep(&nvp, ";")) != NULL) {
        jqs_count++;
    }
}

void addjqs(char *url)
{
    if(url == NULL)
        return;

    FILE *fp;
    fp = fopen(jqs_path, "a");
    if(fp == NULL)
        return;

    fprintf(fp, ";%s", url);
    fclose(fp);
}

static char * get_host_name(char *url)
{
    char *hostname;
    int i;

    if(strncasecmp(url, "http", 4) == 0)
        i = 7;
    else
        i = 0;

    /* skip to end, slash, or port colon */
    for( ; url[i] && url[i] != '/' && url[i] != ':'; i++ );

    hostname = malloc(i + 1);

    memcpy(hostname, url, i);

    hostname[i] = '\0';

    /* if there's a port */
    /*if(url[i] == ':')
            url += i + 1;
    else
            url += i;

    u->host = hostname;*/
    return hostname;
}

int download_bt_file(xmlChar *url)
{
//    char bt_file[strlen(base_path) + 30];
//    char bt_head[strlen(base_path) + 30];

    char bt_file[128];
    char bt_head[1280];

    sprintf(bt_file, "%s/Download2/RSS/tmp.torrent", base_path);
    sprintf(bt_head, "%s/Download2/RSS/tmp.txt", base_path);

    memset(logurl, 0, sizeof(logurl));

    CURL *curl;
    CURLcode res;
    curl=curl_easy_init();
    FILE *fp;
    fp = fopen(bt_file,"wb");
    FILE *fd;
    fd = fopen(bt_head, "wb");
    if(fp == NULL || fd == NULL)
    {
        printf("\nopen rssxml error 2\n");
        return -1;
    }

    if(curl)
    {
        struct curl_slist *headers_l=NULL;
        static const char header4_l[]="User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0";

        headers_l=curl_slist_append(headers_l,header4_l);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers_l);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,60);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, fd);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
        res=curl_easy_perform(curl);
        if(res != 0)
        {
            printf("\ndownload bt file failed\n");
            fclose(fp);
            fclose(fd);
            return -1;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers_l);
    }
    fclose(fp);
    fclose(fd);

    fd = fopen(bt_head,"r");
    fseek(fd, 0, SEEK_END);
    int length = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    char buf[length + 1];
    memset(buf, 0 ,sizeof(buf));
    fread(buf, 1, length, fd);
    fclose(fd);

    char dl_file_name[128];

    if(buf && *(buf + 9) == '3') {
        char *p, *q;
        char location[256];
        memset(location, 0 , sizeof(location));
        if(((p = strstr(buf, "Location")) != NULL) || ((p = strstr(buf, "location")) != NULL))
        {
            while(*p != ':')
                p++;

            p++;p++;
            if(((q = strchr(p,'\n')) != NULL) || ((q = strchr(p,'\r')) != NULL))
                snprintf(location, strlen(p) - strlen(q), "%s", p);
        }

        //char *hostname = get_host_name(url);
        //char newurl[strlen(location) + strlen(url) + 1];
        //sprintf(newurl, "%s%s", hostname, location);
        char *newurl = my_str_malloc(strlen(location)+1);
        sprintf(newurl, "%s", location);
        //free(hostname);

        memset(dl_file_name, 0, sizeof(dl_file_name));
        if(p = strrchr(location,'/'))
        {
            p++;
            sprintf(dl_file_name, "%s/Download2/RSS/%s", base_path, p);
        }

        curl=curl_easy_init();
        fp = fopen(bt_file,"wb");
        fd = fopen(bt_head, "wb");
        if(fp == NULL || fd == NULL)
        {
            free(newurl);
            printf("\nopen rssxml error 3\n");
            return -1;
        }

        if(curl)
        {
            struct curl_slist *headers_l=NULL;
            static const char header4_l[]="User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0";

            headers_l=curl_slist_append(headers_l,header4_l);
            curl_easy_setopt(curl, CURLOPT_URL, newurl);
            curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers_l);
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,60);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_WRITEHEADER, fd);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
            res=curl_easy_perform(curl);
            free(newurl);
            if(res != 0)
            {
                printf("\ndownload bt file failed\n");
                fclose(fp);
                fclose(fd);
                return -1;
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers_l);
        }
        fclose(fp);
        fclose(fd);

        if(access(dl_file_name, F_OK) != 0){
            rename(bt_file, dl_file_name);
            sprintf(logurl, "%s", basename(dl_file_name));
            dlcount++;
        }
        else
        {
            char *hash1 = get_hash(bt_file);
            char *hash2 = get_hash(dl_file_name);
            if(strcmp(hash1, hash2) == 0)
            {
                unlink(bt_file);
            }//eric mark 同名文件不同hash
            if(hash1)
                free(hash1);
            if(hash2)
                free(hash2);
        }
        //unlink(bt_head);
    }
    else if(buf && *(buf + 9) == '2') {//2-->http 200ok
        char *p, *q;
        char torrentname[256];
        memset(torrentname, 0 , sizeof(torrentname));
        if((p = strstr(buf, "filename")) != NULL)
        {
            while(*p != '\"')
                p++;

            p++;
            if((q = strchr(p, '\"')) != NULL)
                snprintf(torrentname, strlen(p) - strlen(q) + strlen(rss_path) + 2, "%s/%s", rss_path, p);
        }

        if(torrentname != NULL || strlen(torrentname) != 0)
        {
            if(access(torrentname, F_OK) != 0){
                rename(bt_file, torrentname);
                sprintf(logurl, "%s", basename(torrentname));
                dlcount++;
            }
            else
            {
                char *hash1 = get_hash(bt_file);
                char *hash2 = get_hash(torrentname);
                if(strcmp(hash1, hash2) == 0)
                {
                    unlink(bt_file);
                }
                if(hash1)
                    free(hash1);
                if(hash2)
                    free(hash2);
            }

           // unlink(bt_head);
        }
    }
    else
        download_bt_file(url);

    return 0;
}

int update_log()
{
    int i;
    char *date_file_content;
    date_file_content = my_str_malloc(2048);
    for(i=0;i<urlcount;i++)
    {
        if(0 != strlen(new_pubdate[i]))
        {
            if(strlen(date_file_content) != 0)
                sprintf(date_file_content,"%s%s<%s\n",date_file_content,urllist[i],new_pubdate[i]);
        }
    }

    FILE *fp;

    fp = fopen(LOG_FILE,"w");
    if(fp)
    {
        fprintf(fp,"%s",date_file_content);
        fclose(fp);
    }
	if(date_file_content!=NULL){
    		free(date_file_content);
	}
    return 0;
}

/*rss jqs function*/
int parseNode(xmlNodePtr node, int i)
{
    int parse_wait = 0;
    xmlChar *date = NULL;
    xmlChar *channelname = NULL;
    int item_flag = 0;
    while(node != NULL && !quitting) {
        if(xmlStrcmp(node->name, (const xmlChar *) "title") == 0)
        {
            channelname = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
            sprintf(namelist[i], "%s", channelname);
            xmlFree(channelname);
        }
        else if(xmlStrcmp(node->name, (const xmlChar *) "item") == 0)
        {
            itemcount[i]++;
            if(strlen(new_pubdate[i]) != 0) {
                xmlNodePtr item_child = NULL;
                item_child = node->xmlChildrenNode;
                while(item_child != NULL) {
                    if(xmlStrcmp(item_child->name, (const xmlChar *)"pubDate") == 0) {
                         date = xmlNodeListGetString(doc, item_child->xmlChildrenNode, 1);
                         if(strcmp(date, new_pubdate[i]) == 0)
                             item_flag = itemcount[i];

                         xmlFree(date);
                    }
                    item_child = item_child->next;
                }
            }
            else
            {
                xmlNodePtr item_child = NULL;
                item_child = node->xmlChildrenNode;
                while(item_child != NULL) {
                    item_child = item_child->next;
                }
            }
        }
        if(node->next == NULL)
            break;
        node = node->next;
    }

    node = node->prev;
    int item2 = 0;
    while(node != NULL && !quitting)
    {
        if(xmlStrcmp(node->name, (const xmlChar *) "item") == 0)
        {

            xmlNodePtr item_child = NULL;
            item_child = node->xmlChildrenNode;

            while(item_child != NULL && !quitting) {
                while((access("/tmp/lighttpd_lock", F_OK) == 0) && !quitting)
                    usleep(1000);

                if(quitting)
                    break;

                create_lock_file();
                initjqs();
                unlink("/tmp/rss_lock");

                if(parse_wait)
                {
                    if(jqs_count<5)
                        parse_wait = 0;
                    sleep(3);
                    continue;
                }

                if(jqs_count > RSS_QUEUE_NUM)
                {
                    sleep(3);
                    parse_wait = 1;
                    continue;
                }

                if(xmlStrcmp(item_child->name, (const xmlChar *)"pubDate") == 0) {
                    date = xmlNodeListGetString(doc, item_child->xmlChildrenNode, 1);

                    if(date) {
                        sprintf(new_pubdate[i], "%s", date);
                        update_log();
                        xmlFree(date);
                    }
                }
                else if(xmlStrcmp(item_child->name, (const xmlChar *)"enclosure") == 0) {
                    xmlChar *url;
                    url = xmlGetProp(item_child, "url");
                    item2++;

                    if((item_flag == 0) || (item_flag != 0 && item2 > itemcount[i] - item_flag + 1)) {
                        while(download_bt_file(url) != 0);

                        while(access("/tmp/lighttpd_lock", F_OK) == 0)
                            usleep(1000);

                        create_lock_file();
                        addjqs(logurl);
                        unlink("/tmp/rss_lock");
                        jqs_count++;
                    }

                    if(url)
                        xmlFree(url);

                }
                item_child = item_child->next;
            }
        }
        else if(xmlStrcmp(node->name, (const xmlChar *) "title") == 0)
            break;
        node = node->prev;
    }

    return 0;
}

int parse_rssxml(int i)
{
    xmlKeepBlanksDefault(0);

    doc = xmlParseFile(rss_file);
    if(doc == NULL)
    {
        fprintf(stderr,"Document not parsed successfully. \n");
        return -1;
    }

    root = xmlDocGetRootElement(doc);
    if(root == NULL)
    {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(root->name, (const xmlChar *) "rss")) {
        fprintf(stderr,"document of the wrong type, root node != rss");
        xmlFreeDoc(doc);
        return -1;
    }

    xmlNodePtr child = NULL;
    child = root->xmlChildrenNode;
    while(child != NULL) {
        if(xmlStrcmp(child->name, (const xmlChar *) "channel") == 0)
            break;
        child = child->next;
    }

    parseNode(child->xmlChildrenNode, i);
    xmlFreeDoc(doc);
    return 0;
}

int date_file_record()
{
    int i;
    char *date_file_content;
    date_file_content = my_str_malloc(1024);
    for(i=0;i<urlcount;i++)
    {
        if(0 != strlen(new_pubdate[i]))
            sprintf(date_file_content,"%s%s<%s\n",date_file_content,urllist[i],new_pubdate[i]);
    }

    FILE *fp;
    fp = fopen(rss_date_file,"w");
    if(fp)
    {
        fprintf(fp,"%s",date_file_content);
        fclose(fp);
    }

    free(date_file_content);

    return 0;
}

int clean_for_run_again()
{
    int i;

    for(i=0;i<urlcount;i++)
    {
        memset(urllist[i],'\0',128);
        memset(namelist[i],'\0',128);
        memset(new_pubdate[i],'\0',64);
    }

    urlcount = 0;
    namecount = 0;

    return 0;
}

int run()
{
    quitting = 0;
    run_again = 0;

    read_config();

    while(!quitting)
    {
        int i;
        read_date_file();
        update_log();

        for(i = 0; i < urlcount && !quitting;i++)
        {
            get_rss_file(urllist[i]);
            if(rss_xml_get_success == 0)
                continue;

            parse_rssxml(i);
            //fprintf(stderr,"\npudate=%s\n", new_pubdate[i]);
        }

        pthread_mutex_lock(&mutex);
        if(quitting == 0) {
            struct timeval now;
            struct timespec outtime;

            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + rss_check_time*3600;
            outtime.tv_nsec = 0;
            pthread_cond_timedwait(&cond, &mutex, &outtime);
        }
        pthread_mutex_unlock(&mutex);
    }

    //date_file_record();

    clean_for_run_again();

    if(run_again)
        run();
    return 0;
}

int main()
{
    int ret;  

    unlink("/tmp/APPS/DM2/Status/rss_stop");

    /*set signal handlers*/
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigterm;
    if(sigaction(SIGTERM, &sa, NULL))
        fprintf(stderr,"Failed to set %s handler. EXITING.\n", "SIGTERM");
    if(sigaction(SIGINT, &sa, NULL))
        fprintf(stderr,"Failed to set %s handler. EXITING.\n", "SIGINT");
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        fprintf(stderr,"Failed to set %s handler. EXITING.\n", "SIGPIPE");
    if(signal(SIGHUP, SIG_IGN) == SIG_ERR)
        fprintf(stderr,"Failed to set %s handler. EXITING.\n", "SIGHUP");
    signal(SIGUSR1, &sigusr1);

    ret = init();

    run();

    FILE *fp;
    fp = fopen("/tmp/APPS/DM2/Status/rss_stop","w");
    if(fp)
        fclose(fp);

    return 0;
}
