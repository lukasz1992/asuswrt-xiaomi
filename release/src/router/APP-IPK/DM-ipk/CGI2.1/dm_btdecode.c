#include "dm_btdecode.h"

Filess *filess_head = NULL;//for mutifile

void sha1_starts( sha1_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process( sha1_context *ctx, uint8 data[64] )
{
    uint32 temp, W[16], A, B, C, D, E;

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

void sha1_update( sha1_context *ctx, uint8 *input, uint32 length )
{
    uint32 left, fill;

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
static uint8 sha1_padding[64] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sha1_finish( sha1_context *ctx, uint8 digest[20] )
{
    uint32 last, padn;
    uint32 high, low;
    uint8 msglen[8];
    high = ( ctx->total[0] >> 29 )
           | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32( high, msglen, 0 );
    PUT_UINT32( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_update( ctx, sha1_padding, padn );
    sha1_update( ctx, msglen, 8 );

    PUT_UINT32( ctx->state[0], digest,  0 );
    PUT_UINT32( ctx->state[1], digest,  4 );
    PUT_UINT32( ctx->state[2], digest,  8 );
    PUT_UINT32( ctx->state[3], digest, 12 );
    PUT_UINT32( ctx->state[4], digest, 16 );
}

static void Sha1(char *ptr,size_t len,unsigned char *dm)
{
#ifdef WINDOWS
    ;
#else
    sha1_context context;
    sha1_starts(&context);
    sha1_update(&context,(unsigned char*)ptr,len);
    sha1_finish(&context,dm);
#endif
}

char* _file2mem(const char *fname, size_t *psiz)
{
    char *b = (char*) 0;
    struct stat sb;
    FILE* fp;
    fp = fopen(fname,"r");
    if( !fp ){
        fprintf(stderr,"error, open %s failed. %s\n",fname,strerror(errno));
        return (char*) 0;
    }

    if(stat(fname,&sb) < 0){
        fprintf(stderr,"error, stat %s failed, %s\n",fname,strerror(errno));
        return (char*) 0;
    }

    if( sb.st_size > MAX_METAINFO_FILESIZ ){
        fprintf(stderr,"error, %s is really a metainfo file???\n",fname);
        return (char*) 0;
    }

    //b = new char[sb.st_size];
    b = (char*)malloc(sb.st_size);
#ifndef WINDOWS
    if( !b ) return (char*) 0;
#endif

    if(fread(b, sb.st_size, 1, fp) != 1){
        if( ferror(fp) ){
            free(b);
            return (char*) 0;
        }
    }
    fclose(fp);

    if(psiz) *psiz = sb.st_size;
    return b;
}

static const char* next_key(const char *keylist)
{
    for(;*keylist && *keylist != KEY_SP; keylist++);
    if(*keylist) keylist++;
    return keylist;
}

static size_t compare_key(const char *key,size_t keylen,const char *keylist)
{
    for(;keylen && *keylist && *key==*keylist;keylen--,key++,keylist++) ;
    if(!keylen) if(*keylist && *keylist!=KEY_SP) return 1;
    return keylen;
}

size_t buf_int(const char *b,size_t len,char beginchar,char endchar,int64_t *pi)
{
    const char *p = b;
    const char *psave;

    if(2 > len) return 0; /* buffer too small */

    if( beginchar ){
        if(*p != beginchar) return 0;
        p++; len--;
    }

    for(psave = p; len && isdigit(*p); p++,len--) ;

    if(!len || MAX_INT_SIZ < (p - psave) || *p != endchar) return 0;

    if( pi ){
        if( beginchar ) *pi = strtoll(b + 1,(char**) 0,10);
        else  *pi= strtoll(b,(char**) 0,10);
    }
    return (size_t)( p - b + 1 );
}

size_t buf_str(const char *b,size_t len,const char **pstr,size_t* slen)
{
    size_t rl;
    int64_t sl;

    rl = buf_int(b,len,0,':',&sl);

    if( !rl ) return 0;

    if(len < rl + sl) return 0;
    if(pstr) *pstr = b + rl;
    if(slen) *slen = sl;

    return( rl + sl );
}

size_t decode_int(const char *b,size_t len)
{
    return(buf_int(b,len,'i','e',(int64_t*) 0));
}

size_t decode_query(const char *b,size_t len,const char *keylist,const char **ps,int64_t *pi,int method)
{
    size_t pos;
    char kl[KEYNAME_LISTSIZ];
    strcpy(kl,keylist);
    pos = decode_rev(b, len, kl);
    if( !pos ) return 0;
    switch(method){
        //case QUERY_STR: return(buf_str(b + pos,len - pos, ps, pi));
    case QUERY_STR:
        {
            size_t ret,tmp;
            ret=buf_str(b + pos,len - pos, ps, &tmp);
            *pi=(int64_t)tmp;
            return ret;
        }
    case QUERY_INT: return(buf_int(b + pos,len - pos, 'i', 'e', pi));
    case QUERY_POS:
        if(pi) *pi = decode_rev(b + pos, len - pos, (const char*) 0);
        return pos;
    default: return 0;
    }
}

size_t decode_rev(const char *b,size_t len,const char *keylist)
{
    if( !b ) return 0;
    switch( *b ){
    case 'i': return decode_int(b,len);
    case 'd': return decode_dict(b,len,keylist);
    case 'l': return decode_list(b,len,keylist);
    default: return decode_str(b,len);
    }
}

size_t decode_str(const char *b,size_t len)
{
    return (buf_str(b,len,(const char**) 0,(size_t*) 0));
}

size_t decode_list(const char *b,size_t len,const char *keylist)
{
    size_t ll,rl;
    ll = 0;
    if(2 > len || *b != 'l') return 0;
    len--; ll++;
    for(;len && *(b + ll) != 'e';){
        rl = decode_rev(b + ll,len,keylist);
        if( !rl ) return 0;

        ll += rl; len -= rl;
    }
    if( !len ) return 0;
    return ll + 1;  /* add last char 'e' */
}

size_t decode_dict(const char *b,size_t len,const char *keylist)
{
    size_t rl,dl,nl;
    const char *pkey;
    dl = 0;
    if(2 > len || *b != 'd') return 0;

    dl++; len--;
    for(;len && *(b + dl) != 'e';){
        rl = buf_str(b + dl,len,&pkey,&nl);

        if( !rl || KEYNAME_SIZ < nl) return 0;
        dl += rl;
        len -= rl;

        if(keylist && compare_key(pkey,nl,keylist) == 0){
            pkey = next_key(keylist);
            if(! *pkey ) return dl;
            rl = decode_dict(b + dl,len, pkey);
            if( !rl ) return 0;
            return dl + rl;
        }

        rl = decode_rev(b + dl,len,(const char*) 0);
        if( !rl ) return 0;

        dl += rl;len -= rl;
    }
    if( !len || keylist) return 0;
    return dl + 1;        /* add the last char 'e' */
}

char *changeb(char* b)
{
	if(strchr(b,10) != NULL)
	{
	  b = strchr(b,10)+1;
	}
//fprintf(stderr,"tmp=%s\n",filename);
return b;

}

int get_file_infohash(const char* metainfo_fname, char *infohash)
{
    unsigned char *ptr = m_shake_buffer;
    char *b;
    const char *s;
    size_t flen;
    int64_t q, r;
    // tmp test
    int i;
    int j=0;
	//2012.06.13 eric added {
    int push_pop = 0;
    long begin,end;
	//2012.06.13 eric added }

    b = _file2mem(metainfo_fname,&flen);
    if ( !b ) return -1;
//fprintf(stderr,"b = %s\n",b);
/*//2011.07.22 magic added for torrent . contains \0{
	char a[1];
	memset(a, '\0', sizeof(a));
	sprintf(a,"%c",*(b+2));
	if(!strcmp(a,"d")){
		b=changeb(b);
	}else{

	}
//2011.07.22 magic}*/

	/*if(!strcmp(getenv("HTTP_USER_AGENT"),"ASUSDMUtility/2.0")){ //2011.06.29 magic added for DM2.0 Utility . contains \0
	//fprintf(stderr,"ASUSDMUtility/2.0\n");
	b=changeb(b);
	}*/

    if( !(r = meta_pos("info")) ) return -1;
   // if( !(q = decode_dict(b + r, flen - r, (char*)0)) ) return -2;
    begin = r;
    for(;r<flen;)
    {
        if(b[r] == 'd')
        {
            push_pop++;
            r++;
        }
        else if(b[r] == 'l')
        {
            push_pop++;
            r++;
        }
        else if(b[r] == 'i')
        {
            r++;
            if(r == flen)
                return -1;
            while(b[r] != 'e')
            {
                if((r+1) == flen)
                {
                    fprintf(stderr,"here\n");
                    return -1;
                }
                else
                    r++;
            }
            r++;
        }
        else if((b[r] >= '0')&&(b[r] <= '9'))
        {
            int number = 0;
            while((b[r] >= '0')&&(b[r] <= '9'))
            {
                number = number*10+(b[r]-'0');
                r++;
            }
            r++;
            r = r+number;
        }
        else if(b[r] == 'e')
        {
            push_pop--;
            if(push_pop == 0)
            {
                end = r;
                break;
            }
            else
                r++;
        }
        else return -1;
    }
    if(r == flen)
        return -1;
    q = end-begin+1;
    //Sha1(b + r, q, m_shake_buffer + 28);
    Sha1(b + begin, q, m_shake_buffer + 28); //2012.06.13 eric 

    for(i=28;i<48;i++)
        sprintf(infohash+(2*j++),"%02X",m_shake_buffer[i] & 0xff);

    return 0;
}

//Allen 20090722
int get_file_name(const char* metainfo_fname, char *name)
{
    char *b;
    const char *s;
    int64_t q;
    size_t flen;

    b = _file2mem(metainfo_fname,&flen);
    if ( !b ) return 0;
/*//2011.11.24 magic added for torrent . contains \0{
	char a[1];
	memset(a, '\0', sizeof(a));
	sprintf(a,"%c",*(b+2));
	if(!strcmp(a,"d")){
		b=changeb(b);
	}else{

	}
//2011.11.24 magic}*/

    if( decode_query(b, flen, "info|name.utf-8",&s,&q,QUERY_STR) && q < 256){
        memset(name,0,sizeof(name));
        memcpy(name,s,q);
        name[q]='\0';
        return 1;
    }
    else if( decode_query(b, flen, "info|name",&s,&q,QUERY_STR) && q < 256){
        memset(name,0,sizeof(name));
        memcpy(name,s,q);
        name[q]='\0';
        return 1;
    }

    return 0;
}

int get_files_path(const char* metainfo_fname, char *path)
{
    char *b;
    const char *s;
    int64_t q;
    size_t flen;

    b = _file2mem(metainfo_fname,&flen);
    if(!b)
        return 0;
    if( decode_query(b, flen, "path.utf-8",&s,&q,QUERY_STR) && q < 256){
        memset(path,0,sizeof(path));
        memcpy(path,s,q);
        path[q]='\0';
        //fprintf(stderr,"\nname=%s\n",path);
        return 1;
    }
    else if( decode_query(b, flen, "path",&s,&q,QUERY_STR) && q < 256){
        memset(path,0,sizeof(path));
        memcpy(path,s,q);
        path[q]='\0';
        //fprintf(stderr,"\nname=%s\n",path);
        return 1;
    }
    return 0;
}

int get_files_length_path(const char* metafile_name)
{
    char *metafile_content = NULL;//torrent file content

    long i;
    unsigned int filesize;
    FILE *fp = fopen(metafile_name,"rb");
    if(fp == NULL)
    {
        fprintf(stderr,"file open failed\n");
        return -1;
    }

    fseek(fp,0,SEEK_END);
    filesize = ftell(fp);
    if(filesize == -1)
    {
        fprintf(stderr,"file fseek failed\n");
        return -1;
    }
    metafile_content = (char *)malloc(filesize+1);
    if(metafile_content == NULL)
    {
        fprintf(stderr,"mem malloc failed\n");
        return -1;
    }

    fseek(fp,0,SEEK_SET);
    for(i=0;i<filesize;i++)
        metafile_content[i] = fgetc(fp);
    metafile_content[i] = '\0';

    fclose(fp);

    unsigned long long length;
    int count;

    Filess *node = NULL;
    Filess *p = NULL;
    int filesnum = 0;

    if(multi_file != 1)
        return 0;

    for(i=0;i<filesize-8;i++)
    {
        if(memcmp(&metafile_content[i],"6:length",8) == 0)
        {
            i = i+8;
            i++;
            length = 0;
            while(metafile_content[i] != 'e')
            {
                length = length*10 +(metafile_content[i]-'0');
                i++;
            }
            node = (Filess *)malloc(sizeof(Filess));
            //fprintf(stderr,"length=%lld\n",length);
            node->length = length;
            node->id = filesnum;
            node->next = NULL;
            if(filess_head == NULL)
                filess_head = node;
            else
            {
                p = filess_head;
                while(p->next != NULL)  p = p->next;
                p->next = node;
            }
            filesnum++;
        }
        if(memcmp(&metafile_content[i],"10:path.utf-8",13) == 0)
        {
            i = i+13;
            i++;
            count = 0;
            while(metafile_content[i] != ':')
            {
                count = count *10 +(metafile_content[i]-'0');
                i++;
            }
            i++;
            p = filess_head;
            while(p->next != NULL) p = p->next;
            memcpy(p->path,&metafile_content[i],count);
            *(p->path + count) = '\0';
            i = i + count;
            if(metafile_content[i] == 'e')
                memcpy(p->name,"none",5);
            else if(isdigit(metafile_content[i]))
            {
                count = 0;
                while(metafile_content[i] != ':')
                {
                    count = count *10 +(metafile_content[i]-'0');
                    i++;
                }
                i++;
                memcpy(p->name,&metafile_content[i],count);
                *(p->name+count) = '\0';
            }

            //fprintf(stderr,"path.utf-8 is %s\n",p->path);
            //fprintf(stderr,"name.utf-8 is %s\n",p->name);
        }
        else if(memcmp(&metafile_content[i],"4:path",6) == 0)
        {
            i = i+6;
            i++;
            count = 0;
            while(metafile_content[i] != ':')
            {
                count = count *10 +(metafile_content[i]-'0');
                i++;
            }
            i++;
            p = filess_head;
            while(p->next != NULL) p = p->next;
            memcpy(p->path,&metafile_content[i],count);
            *(p->path+count) = '\0';
            i = i + count;
            if(metafile_content[i] == 'e')
                memcpy(p->name,"none",5);
            else if(isdigit(metafile_content[i]))
            {
                count = 0;
                while(metafile_content[i] != ':')
                {
                    count = count *10 +(metafile_content[i]-'0');
                    i++;
                }
                i++;
                memcpy(p->name,&metafile_content[i],count);
                *(p->name+count) = '\0';
            }
            //fprintf(stderr,"path is %s\n",p->path);
            //fprintf(stderr,"name is %s\n",p->name);
        }
    }
    /*p = filess_head;
    while(p)
    {
        fprintf(stderr,"\n*******id=%d\n",p->id);
        fprintf(stderr,"length=%ld\n",p->length);
        fprintf(stderr,"name=%s\n",p->name);
        fprintf(stderr,"path=%s\n",p->path);
        p = p->next;
    }*/
    //fprintf(stderr,"\n############filesnum=%d\n",filesnum);
    if(metafile_content != NULL)
        free(metafile_content);
    return 0;
}

//eric 20120613
int get_file_length(const char* metainfo_fname, int64_t *total_len)
{
    char *metabuf = NULL;
    size_t metabuf_len;
    FILE *fp;
    long j;
    fp = fopen(metainfo_fname,"rb");
    if(fp == NULL)
    {
        fprintf(stderr,"error to open file\n");
        return -1;
    }
    //get torrent file size
    fseek(fp, 0 ,SEEK_END);
    metabuf_len = ftell(fp);
    if(metabuf_len == -1)
    {
        fprintf(stderr,"ftell file is failed\n");
        return -1;
    }
    //get torrent file content
    metabuf = (char *)malloc(metabuf_len+1);
    if(metabuf == NULL)
    {
        fprintf(stderr,"malloc file mem is failed\n");
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    for(j = 0;j < metabuf_len;j++)
    {
        metabuf[j] = fgetc(fp);
    }
    metabuf[j] = '\0';
    fclose(fp);
    //fprintf(stderr, "metabuf=%s\n", metabuf);

    if ( !metabuf ) return 0;

/*//2011.11.24 magic added for torrent . contains \0{
        char a[1];
        memset(a, '\0', sizeof(a));
        sprintf(a,"%c",*(metabuf+2));
        if(!strcmp(a,"d")){
                metabuf=changeb(metabuf);
        }else{

        }
//2011.11.24 magic}*/

    multi_file=0;//if DL file is multi_files is 1,else is 0
    /*if(strstr(metabuf,"5:files")) multi_file = 1;
    else multi_file = 0;*/
    //2012.07.03 eric modi{
    for(j=0;j<metabuf_len-7;j++)
    {
    if(memcmp(&metabuf[j],"5:files",7) == 0){
	 multi_file = 1;
	break;
	}
    else {
	multi_file = 0;
	}
    }
    //2012.07.03 eric modi}
    //fprintf(stderr, "multi_file=%d\n",multi_file);

    if(multi_file == 1)
    {
        long i;
        Files *files_head = NULL;//for muti_file
        unsigned long long length;
        Files *node = NULL;
        Files *p = NULL;
        if(files_head == NULL)
        {
            for(i=0;i<metabuf_len-8;i++)
            {
                if(memcmp(&metabuf[i],"6:length",8) == 0)
                {
                    i=i+8;
                    i++;
                    length = 0;
                    while(metabuf[i] != 'e')
                    {
                        length = length*10 + (metabuf[i]-'0');
                        i++;
                    }
                    node = (Files *)malloc(sizeof(Files));
                    node->length = length;
                    //fprintf(stderr,"length=%lld\n",length);
                    node->next = NULL;
                    if(files_head == NULL)
                        files_head = node;
                    else
                    {
                        p = files_head;
                        while(p->next != NULL) p=p->next;
                        p->next=node;
                    }
                }
            }
        }
        Files *q;
        q = files_head;
        while(q != NULL)
        {
            *total_len += q->length;
            q = q->next;
        }
        free(node);
    }
    else
    {
        long l;
        /*char *poss;
        poss=strstr(metabuf,"6:length");
        if(poss == NULL)
            fprintf(stderr,"have not find\n");
        l=strlen(metabuf)-strlen(poss);
        l=l+8;
        l++;//'i'*/
        for(l=0;l<metabuf_len-8;l++)
        {
            if(memcmp(&metabuf[l],"6:length",8) == 0)
            {
                //fprintf(stderr,"metabuf111=%s\n",metabuf+l);
                l=l+8;
                l++;
                break;
            }
        }
        while(metabuf[l] != 'e')
        {
            *total_len = *total_len*10 + (metabuf[l]-'0');
            l++;
        }
    }
    //fprintf(stderr, "total_len=%lld\n",*total_len);
    free(metabuf);

    return 0;
}
