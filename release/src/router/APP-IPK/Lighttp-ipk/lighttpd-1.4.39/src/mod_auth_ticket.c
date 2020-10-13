//
// mod_auth_ticket - Authentication based on signed cookie
//
// This module protects webpage from clients without valid
// cookie. By redirecting not-yet-valid clients to certain
// "logon page", you can protect any webapp without adding
// any auth code to webapp itself.
//
// Unlike mod_authcookie for Apache, this DOES NOT work
// with mod_auth_* modules due to lighttpd limitation (there's
// no way to turn 401 response into page redirection).
// This module solely relies on external "logon page" for
// authentication, and expect it to provide a valid cookie as a
// ticket for authenticated access.
//

#include <ctype.h>

#include "plugin.h"
#include "log.h"
#include "response.h"
#include "md5.h"

#include "mod_auth_ticket.h"
#include "base.h"
#include "dm_func.h"   //leo

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#define VER_ID(major, minor, relno) ((major) << 16 | (minor) << 8 | (relno))

// Compatibility macros to use this module with older lighttpd versions.
// With svn commit r2788, MD5* APIs were renamed to li_MD5*.
#if LIGHTTPD_VERSION_ID < VER_ID(1, 4, 29)
#define li_MD5_CTX    MD5_CTX
#define li_MD5_Init   MD5_Init
#define li_MD5_Update MD5_Update
#define li_MD5_Final  MD5_Final
#endif


#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];
#ifdef USE_OPENSSL
#define IN const
#else
#define IN
#endif
#define OUT

static void CvtHex(const HASH Bin, char Hex[33]) {
    li_tohex(Hex, (const char*) Bin, 16);
}

#include "base64.h"
#include "server.h"
#include "http_auth.h"
//#include "http_auth_digest.h"
#define LOG(level, ...)                                           \
if (pc->loglevel >= level) {                                  \
                                                              log_error_write(srv, __FILE__, __LINE__, __VA_ARGS__);    \
                                                          }

#define FATAL(...) LOG(0, __VA_ARGS__)
#define ERROR(...) LOG(1, __VA_ARGS__)
#define WARN(...)  LOG(2, __VA_ARGS__)
#define INFO(...)  LOG(3, __VA_ARGS__)
#define DEBUG(...) LOG(4, __VA_ARGS__)

#define HEADER(con, key)                                                \
(data_string *)array_get_element((con)->request.headers, (key))

#define MD5_LEN 16
#define URLTIMEOUT (10*60)

#define MY_KEY "sharedsecret.passwd"
        buffer *direct_url_string=NULL;//neal add default NULL

char productname[64];
// 20160608 leo added for lock username and password {
static int Username_Pw_DM=0;
static int Username_Pw_MS=0;
long int time_DM,time_MS,time_c;
static int flag_up_DM=0;
static int flag_up_MS=0;
time_t last_access_time_DM = 0;
time_t last_access_time_MS = 0;
unsigned int login_ip_DM = 0;
unsigned int login_ip_MS = 0;
int nvramver;
char directurl_post[256];
char dec_pd[256];
#ifdef DM_MIPSBIG
static int mips_type = -1;
#endif
/**********************************************************************
 * data strutures
 **********************************************************************/

// module configuration
typedef struct {
    int loglevel;
    buffer *name;    // cookie name to extract auth info
    int override;    // how to handle incoming Auth header
    buffer *authurl; // page to go when unauthorized
    buffer *key;     // key for cookie verification
    int timeout;     // life duration of last-stage auth token
    buffer *options; // options for last-stage auth token cookie
} plugin_config;

// top-level module structure
typedef struct {
    PLUGIN_DATA;

    plugin_config **config;
    plugin_config   conf;

    array *users;
    array *users2;
} plugin_data;

/**********************************************************************
 * supporting functions
 **********************************************************************/

//
// helper to generate "configuration in current context".
//
static plugin_config *
        merge_config(server *srv, connection *con, plugin_data *pd) {
#define PATCH(x) pd->conf.x = pc->x
#define MATCH(k) if (buffer_is_equal_string(du->key, CONST_STR_LEN(k)))
#define MERGE(k, x) MATCH(k) PATCH(x)

    size_t i, j;
    plugin_config *pc = pd->config[0]; // start from global context

    // load initial config in global context
    PATCH(loglevel);
    PATCH(name);
    PATCH(override);
    PATCH(authurl);
    PATCH(key);
    PATCH(timeout);
    PATCH(options);

    // merge config from sub-contexts
    for (i = 1; i < srv->config_context->used; i++) {
        data_config *dc = (data_config *)srv->config_context->data[i];

        // condition didn't match
        if (! config_check_cond(srv, con, dc)) continue;

        // merge config
        pc = pd->config[i];
        for (j = 0; j < dc->value->used; j++) {
            data_unset *du = dc->value->data[j];

            // describe your merge-policy here...
            MERGE("auth-ticket.loglevel", loglevel);
            MERGE("auth-ticket.name", name);
            MERGE("auth-ticket.override", override);
            MERGE("auth-ticket.authurl", authurl);
            MERGE("auth-ticket.key", key);
            MERGE("auth-ticket.timeout", timeout);
            MERGE("auth-ticket.options", options);
        }
    }
    return &(pd->conf);
#undef PATCH
#undef MATCH
#undef MERGE
}

//
// Generates appropriate response depending on policy.
//
static handler_t
        endauth(server *srv, connection *con, plugin_config *pc) {

    if(strstr(con->request.uri->ptr,".cgi"))
    {
        // prepare response
        if(strstr(con->request.uri->ptr,"mediaserverui")){
            if (!con->file_finished) {
                buffer_reset(con->physical.path);
                buffer *b;
                b = chunkqueue_get_append_buffer(con->write_queue);
                char dirceturl[256];
                memset(dirceturl,0,sizeof(dirceturl));
                sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=1&productname=",productname,"&url=");
                buffer_append_string(b, dirceturl);
                response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
            }
        }
        con->http_status =598;
        con->mode = DIRECT;
        con->file_finished = 1;
        return HANDLER_FINISHED;
    }
    // pass through if no redirect target is specified
    if (buffer_is_empty(pc->authurl)) {
        return HANDLER_GO_ON;
    }

    // prepare redirection header
    buffer *url = buffer_init_buffer(pc->authurl);
    buffer_append_string(url, "?flag=1");
    buffer_append_string(url, "&productname=");
    buffer_append_string(url, productname);
    buffer_append_string(url, strchr(url->ptr, '?') ? "&url=" : "?url=");

    buffer_append_string(url,con->request.uri->ptr);

    response_header_insert(srv, con,
                           CONST_STR_LEN("Location"), CONST_BUF_LEN(url));
    buffer_free(url);

    // prepare response
    con->http_status = 307;
    con->mode = DIRECT;
    con->file_finished = 1;

    return HANDLER_FINISHED;
}

// generate hex-encoded random string
static int
        gen_random(buffer *b, int len) {
    buffer_string_prepare_copy(b, len);
    while (len--) {
        char c = int2hex(rand() >> 24);
        //buffer_append_memory(b, &c, 1);
	buffer_append_string_len(b, &c, 1);
    }
    return 0;
}

/*
 ***nvarm_encrypt_support
 ***return 1: nvram encrypt, 0: nvram no encrypt
 */
int nvram_encrypt_support()
{
  int nvram_encrypt=0;
  int cfgmgr_ver=0;
#ifdef DM_MIPSBIG
  if(access("/userfs/bin/tcapi",0) == 0){
      cfgmgr_ver=tcapi_get_int("SysInfo_Entry","cfgmgr_ver");
  } else {
     nvram_encrypt=nvram_get_int("nvram_encrypt");
  }
#else
  nvram_encrypt=nvram_get_int("nvram_encrypt");
#endif
  if(nvram_encrypt==1 || cfgmgr_ver>1){
      return 1;
  }else{
      return 0;
  }
}



// XOR-based decryption
// This is not used in this module - it is only provided as an
// example of supported encryption.
static int __attribute__((unused))
        encrypt(buffer *buf, uint8_t *key, int keylen) {
    unsigned int i;

    for (i = 0; i < buf->used; i++) {
        buf->ptr[i] ^= (i > 0 ? buf->ptr[i - 1] : 0) ^ key[i % keylen];
    }
    return 0;
}

//
// update header using (verified) authentication info.
//
static void
		update_header(server *srv, connection *con,
                      plugin_data *pd, plugin_config *pc, buffer *authinfo) {

    buffer *field, *token;

    // insert auth header
    field = buffer_init_string("Basic ");
    buffer_append_string_buffer(field, authinfo);
    array_set_key_value(con->request.headers,
                        CONST_STR_LEN("Authorization"), CONST_BUF_LEN(field));

    // generate random token and relate it with authinfo
    gen_random(token = buffer_init(), MD5_LEN * 2); // length in hex string
    //DEBUG("sb", "pairing authinfo with token:", token);
    buffer_copy_int(field, time(NULL));
    buffer_append_string(field, ":");
	buffer_append_string_buffer(field, authinfo);
    //DEBUG("sb", "pd->user:", field);

    if(strstr(con->request.uri->ptr,"mediaserverui"))
        array_set_key_value(pd->users, CONST_BUF_LEN(token), CONST_BUF_LEN(field));
    else
		array_set_key_value(pd->users2, CONST_BUF_LEN(token), CONST_BUF_LEN(field));

    // insert opaque auth token
    buffer_copy_buffer(field, pc->name);
    buffer_append_string(field, "=asus_app_token:");
    buffer_append_string_buffer(field, token);
    buffer_append_string(field, "; ");

	if(strncmp(direct_url_string->ptr,"/mediaserverui/", 15)==0)
    {
		buffer_append_string(field, "path=/mediaserverui/; httponly;");
		login_ip_MS = con->dst_addr.ipv4.sin_addr.s_addr;
		last_access_time_MS = uptime();
    }
	else if (strncmp(direct_url_string->ptr,"/downloadmaster/", 16)==0)
    {
		buffer_append_string(field, "path=/downloadmaster/; httponly;");
		login_ip_DM = con->dst_addr.ipv4.sin_addr.s_addr;
		last_access_time_DM = uptime();
    }

    response_header_append(srv, con,
                           CONST_STR_LEN("Set-Cookie"), CONST_BUF_LEN(field));

    buffer_free(field);
    buffer_free(token);
}

//20150902  leo added for socket {
/*
int PW_new_socket()
{
    int sfd;
    int result = UNKNOW;
    char command[24];
    memset(command, '\0', sizeof(command));

    sprintf(command, "%s", "password-renew-now");

    struct sockaddr_in btaddr;
    memset(&btaddr, '\0', sizeof(btaddr));
    btaddr.sin_family = AF_INET;
    btaddr.sin_port = htons(BT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
    result = ACK_SUCESS;


    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        result = ACK_FAIL;
        return result;
    }

    if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
        result = ACK_FAIL;
        return result;
    }
	if(write(sfd, command, strlen(command)) != (ssize_t)strlen(command))
    {
        result = ACK_FAIL;
        return result;
    }
    close(sfd);
    //sleep(1);
    return result;
}
*/
//url to get which app access
int check_can_login(server *srv, connection *con, plugin_data *pd) {
	long current_time = uptime();
	unsigned int ip = con->dst_addr.ipv4.sin_addr.s_addr;

	if(strstr(con->request.uri->ptr, "downloadmaster")) {
		//to make another to login
		if((unsigned long)(current_time-last_access_time_DM) > (10*60)){
			//remove the token saved in user
			array_reset(pd->users2);
			return HANDLER_GO_ON;
		}
		if(login_ip_DM != 0 && login_ip_DM != ip) {
			char dirceturl_tmp[128];
			struct in_addr login_ip_addr;
			char *login_ip_str;
			memset(dirceturl_tmp,0,sizeof(dirceturl_tmp));
			login_ip_addr.s_addr = login_ip_DM;
			login_ip_str = inet_ntoa(login_ip_addr);
			sprintf(dirceturl_tmp,"%s%s","/log.asp?flag=1&ip=",login_ip_str);
			response_header_insert(srv, con,
								   CONST_STR_LEN("Location"),dirceturl_tmp,strlen(dirceturl_tmp));
			// prepare response
			con->http_status = 307;
			con->mode = DIRECT;
			con->file_finished = 1;
			return HANDLER_FINISHED;
		}
	} else if (strstr(con->request.uri->ptr, "mediaserverui")) {
		if((unsigned long)(current_time-last_access_time_MS) > (10*60)){
			//remove the token saved in user
			array_reset(pd->users);
			return HANDLER_GO_ON;
		}
		if(login_ip_MS != 0 && login_ip_MS != ip) {
			char dirceturl_tmp[128];
			struct in_addr login_ip_addr;
			char *login_ip_str;
			memset(dirceturl_tmp,0,sizeof(dirceturl_tmp));
			login_ip_addr.s_addr = login_ip_MS;
			login_ip_str = inet_ntoa(login_ip_addr);
			sprintf(dirceturl_tmp,"%s%s","/log.asp?flag=1&ip=",login_ip_str);
			response_header_insert(srv, con,
								   CONST_STR_LEN("Location"),dirceturl_tmp,strlen(dirceturl_tmp));
			// prepare response
			con->http_status = 307;
			con->mode = DIRECT;
			con->file_finished = 1;
			return HANDLER_FINISHED;
		}
	}
	return HANDLER_GO_ON;

}
static int
url_ascii_to_char(const char *output, const char *input, int outsize){
    char *src = (char *)input;
    char *dst = (char *)output;
    char *end = (char *)output+outsize-1;
    char char_array[3];
    unsigned int char_code;

    if(src == NULL || dst == NULL || outsize <= 0)
        return 0;

    for(; *src && dst < end; ++src, ++dst){
        if(*src != '%'){
            *dst = *src;
        } else {
            ++src;
            if(!(*src))
                break;
            memset(char_array, 0, 3);
            strncpy(char_array, src, 2);
            ++src;

            char_code = strtol(char_array, NULL, 16);

            *dst = (char)char_code;
        }
    }

    if(dst <= end)
        *dst = '\0';

    return (dst-output);
}
//
// Handle token given in cookie.
//
// Expected Cookie Format:
//   <name>=token:<random-token-to-be-verified>
//
static handler_t
        handle_token(server *srv, connection *con,
                     plugin_data *pd, plugin_config *pc, char *token) {

    data_string *entry;
    if(strstr(con->request.uri->ptr,"mediaserverui"))
        entry =(data_string *)array_get_element(pd->users, token);
    else
        entry =(data_string *)array_get_element(pd->users2, token);
    //char accessip[32];
	//int ip_Changed = 0;

    // Check for existence
    if (! entry) return endauth(srv, con, pc);

    char *authinfo = strchr(entry->value->ptr, ':');
    if (! authinfo) return endauth(srv, con, pc);
  /*  char buf_username[128];
    char buf_password[128];

    FILE *fp;
    fp=fopen("/tmp/web_info","r+");
    memset(buf_username,0,sizeof(buf_username));
    memset(buf_password,0,sizeof(buf_password));
    if(fp != NULL)
    {
        fgets(buf_username, 128, fp);
        fgets(buf_password, 128, fp);

        if(buf_username[strlen(buf_username)-1]=='\n')
        {
            buf_username[strlen(buf_username)-1]='\0';
        }
        if(buf_password[strlen(buf_password)-1]=='\n')
        {
            buf_password[strlen(buf_password)-1]='\0';
        }

    }
    fclose(fp);
    */
    char *dut_username=NULL;
    char *dut_password=NULL;
    ////////////////{{{
#ifdef DM_MIPSBIG
      dut_username=(char *)malloc(32);
      dut_password=(char *)malloc(128);
      memset(dut_username,0,32);
      memset(dut_password,0,128);
      if(access("/userfs/bin/tcapi",0) == 0){
      dut_username=tcapi_get_by_popen("Account_Entry0","username");
      dut_password=tcapi_get_by_popen("Account_Entry0","web_passwd");
      }else{
          dut_username=nvram_get_by_popen("http_username");
          dut_password=nvram_get_by_popen("http_passwd");
      }
#else
      dut_username=(char *)malloc(32);
      dut_password=(char *)malloc(128);
      memset(dut_username,0,32);
      memset(dut_password,0,128);
      dut_username=nvram_get_by_popen("http_username");
      dut_password=nvram_get_by_popen("http_passwd");
#endif
    /////////////////}}}
    // All passed. Inject as BasicAuth header
    buffer *field = buffer_init();
    buffer_append_string(field, authinfo+1);
    array_set_key_value(con->request.headers,
						CONST_STR_LEN("Authorization"), CONST_BUF_LEN(field));

	char *pw = strchr(field->ptr, ':');
	*pw = '\0';
    //username and password show in different style between mipsbig and other
    /*char passwd_tmp[128];
    char username_tmp[128];
    memset(passwd_tmp,   0, sizeof(passwd_tmp));
    memset(username_tmp, 0, sizeof(username_tmp));
	char_to_ascii_safe(passwd_tmp,   pw+1,      128);

    char_to_ascii_safe(username_tmp, field->ptr, 128);
    */
    nvramver=nvram_encrypt_support();
    //20170622 changed for shadow {
    if(access("/etc/shadow",0)==0){
    int value_u=compare_passwd_in_shadow(field->ptr,pw+1);
    if(!value_u){
        buffer_free(field);
        /*if(pids("dm2_transmission-daemon")!=0)  //dm2_transmission_daemon is running
        {
            PW_new_socket();
        }
        else if(pids("dm2_transmission-daemon")==0) //dm2_transmission_daemon not running
        {
            FILE *fp_tr;
            char changname[256];
            char changpasswd[256];
            char buf_un[32];
            char buf_pd[32];
            system("/opt/etc/asus_script/dm2_transmission_user_changed");
            while(access("/tmp/dm2_check_user",0)== 0)
            {
                sleep(1);
            }
            if(access("/tmp/dm2_check_user",0)!= 0){
                fp_tr=fopen("/tmp/transmission_get_up","r+");
                memset(buf_un,0,sizeof(buf_un));
                memset(buf_pd,0,sizeof(buf_pd));
                if(fp_tr != NULL)
                {
                    fgets(buf_un,32,fp_tr);
                    if(buf_un[strlen(buf_un)-1]=='\n')
                    {
                        buf_un[strlen(buf_un)-1]='\0';
                    }
                    fgets(buf_pd,32,fp_tr);
                    if(buf_pd[strlen(buf_pd)-1]=='\n')
                    {
                        buf_pd[strlen(buf_pd)-1]='\0';
                    }
                }
                fclose(fp_tr);
            }
            memset(changname,0,sizeof(changname));
            sprintf(changname,"sed -i '49s/^.*$/    \"rpc-username\": \"%s\",/' %s/Download2/config/settings.json",buf_un,Base_dir);
            system(changname);
            memset(changpasswd,0,sizeof(changpasswd));
            sprintf(changpasswd,"sed -i '46s/^.*$/    \"rpc-password\": \"%s\",/' %s/Download2/config/settings.json",buf_pd,Base_dir);
            system(changpasswd);
        }*/
        return endauth(srv, con, pc);
      }
    }else{
       if(nvramver==0){
           if((strcmp(field->ptr, dut_username) )|| (strcmp(pw+1,dut_password)))
           {
               buffer_free(field);
               /*if(pids("dm2_transmission-daemon")!=0)  //dm2_transmission_daemon is running
               {
                   PW_new_socket();
               }
               else if(pids("dm2_transmission-daemon")==0) //dm2_transmission_daemon not running
               {
                   FILE *fp_tr;
                   char changname[256];
                   char changpasswd[256];
                   char buf_un[32];
                   char buf_pd[32];
                   system("/opt/etc/asus_script/dm2_transmission_user_changed");
                   while(access("/tmp/dm2_check_user",0)== 0)
                   {
                       sleep(1);
                   }
                   if(access("/tmp/dm2_check_user",0)!= 0){
                       fp_tr=fopen("/tmp/transmission_get_up","r+");
                       memset(buf_un,0,sizeof(buf_un));
                       memset(buf_pd,0,sizeof(buf_pd));
                       if(fp_tr != NULL)
                       {
                           fgets(buf_un,32,fp_tr);
                           if(buf_un[strlen(buf_un)-1]=='\n')
                           {
                               buf_un[strlen(buf_un)-1]='\0';
                           }
                           fgets(buf_pd,32,fp_tr);
                           if(buf_pd[strlen(buf_pd)-1]=='\n')
                           {
                               buf_pd[strlen(buf_pd)-1]='\0';
                           }


                       }
                       fclose(fp_tr);
                   }
                   memset(changname,0,sizeof(changname));
                   sprintf(changname,"sed -i '49s/^.*$/    \"rpc-username\": \"%s\",/' %s/Download2/config/settings.json",buf_un,Base_dir);
                   system(changname);
                   memset(changpasswd,0,sizeof(changpasswd));
                   sprintf(changpasswd,"sed -i '46s/^.*$/    \"rpc-password\": \"%s\",/' %s/Download2/config/settings.json",buf_pd,Base_dir);
                   system(changpasswd);
               }*/
               return endauth(srv, con, pc);
           }
       }
       else if(nvramver==1) {
           char dec_password[256];
           memset(dec_password,0,sizeof(dec_password));
           pw_dec(dut_password,dec_password, sizeof(dec_password));
           if((strcmp(field->ptr, dut_username)) ||(strcmp(pw+1, dec_password)))
           {
               buffer_free(field);
              /* if(pids("dm2_transmission-daemon")!=0)  //dm2_transmission_daemon is running
               {
                   PW_new_socket();
               }
               else if(pids("dm2_transmission-daemon")==0) //dm2_transmission_daemon not running
               {
                   FILE *fp_tr;
                   char changname[256];
                   char changpasswd[256];
                   char buf_un[32];
                   char buf_pd[32];
                   system("/opt/etc/asus_script/dm2_transmission_user_changed");
                   while(access("/tmp/dm2_check_user",0)== 0)
                   {
                       sleep(1);
                   }
                   if(access("/tmp/dm2_check_user",0)!= 0){
                       fp_tr=fopen("/tmp/transmission_get_up","r+");
                       memset(buf_un,0,sizeof(buf_un));
                       memset(buf_pd,0,sizeof(buf_pd));
                       if(fp_tr != NULL)
                       {
                           fgets(buf_un,32,fp_tr);
                           if(buf_un[strlen(buf_un)-1]=='\n')
                           {
                               buf_un[strlen(buf_un)-1]='\0';
                           }
                           fgets(buf_pd,32,fp_tr);
                           if(buf_pd[strlen(buf_pd)-1]=='\n')
                           {
                               buf_pd[strlen(buf_pd)-1]='\0';
                           }


                       }
                       fclose(fp_tr);
                   }
                   memset(changname,0,sizeof(changname));
                   sprintf(changname,"sed -i '49s/^.*$/    \"rpc-username\": \"%s\",/' %s/Download2/config/settings.json",buf_un,Base_dir);
                   system(changname);
                   memset(changpasswd,0,sizeof(changpasswd));
                   sprintf(changpasswd,"sed -i '46s/^.*$/    \"rpc-password\": \"%s\",/' %s/Download2/config/settings.json",buf_pd,Base_dir);
                   system(changpasswd);
               }*/
               return endauth(srv, con, pc);
           }
       }
    }
    //20170622 changed for shadow }
    buffer_free(field);

	//update time for each access
	if(strstr(con->request.uri->ptr,"mediaserverui")){
		last_access_time_MS = uptime();
	} else if(strstr(con->request.uri->ptr,"downloadmaster")){
		last_access_time_DM = uptime();
	}

    return HANDLER_GO_ON;
}

char*
        MD5_string(uint8_t *hash,char *string)
{
    li_MD5_CTX md5;

    li_MD5_Init(&md5);
    li_MD5_Update(&md5, string, strlen(string));
    li_MD5_Final(hash,&md5);

    return NULL;
}

//
// Check for redirected auth request in cookie.
//
// Expected Cookie Format:
//   <name>=crypt:<hash>:<data>
//
//   hash    = hex(MD5(key + timesegment + data))
//   data    = hex(encrypt(MD5(timesegment + key), payload))
//   payload = base64(username + ":" + password)
//
static handler_t
        handle_crypt(server *srv, connection *con,
                     plugin_data *pd, plugin_config *pc, char *line) {

	if (! line)
		return endauth(srv, con, pc);
    //////////////{{{{
        char directurl[256];
        memset(directurl,0,sizeof(directurl));
        snprintf(directurl,sizeof(directurl),"\"%s\";\n",directurl_post);
        buffer *buf = buffer_init();
        buffer_append_string(buf,line);
        update_header(srv, con, pd, pc, buf);
        buffer_free(buf);
        response_header_insert(srv, con, CONST_STR_LEN("Cache-Control"), CONST_STR_LEN("no-store"));
        response_header_insert(srv, con, CONST_STR_LEN("Cache-Control"), CONST_STR_LEN("no-cache"));
        buffer *p;
         buffer_reset(con->physical.path);
         con->file_finished = 1;
         p=chunkqueue_get_append_buffer(con->write_queue);

         //chunkqueue_append_mem(con->write_queue, CONST_STR_LEN(
         buffer_copy_string_len(p, CONST_STR_LEN(
                                    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                                    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                    "<html xmlns:v>\n"
                                    "<head>\n"
                                    "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=EmulateIE8\" />\n"
                                    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
                                    "<meta http-equiv=\"Expires\" content=\"-1\" />\n"
                                    "<meta HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\">\n"
                                    "<meta http-equiv=\"Pragma\" content=\"no-cache\" />\n"
                                    "<title>Login Jump</title>\n"
                                    ));

         buffer_append_string_len(p, CONST_STR_LEN(
                                      "<script type=\"text/javascript\" src=\"jquery.js\"></script>\n"
                                      "</head>\n"
                                      "<body>\n"
                                      "<script>\n"
                                      "var httpTag 		= 'https:' == document.location.protocol ? false : true;\n"
                                      "var directurl_host 	= document.location.host;\n"
                                      "var url 			="
                 ));

         buffer_append_string_len(p, directurl,strlen(directurl));

         buffer_append_string_len(p, CONST_STR_LEN(
                                       "if(httpTag)\n"
                                          "self.location = \"http://\" +directurl_host	+ url;\n"
                                      "else\n"
                                          "self.location = \"https://\" +directurl_host	+ url;\n"

                                      "</script>\n"
                                      "</body>\n"
                                      "</html>\n"

                 ));
         response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
     /////////////////}}}}}

    // update header using decrypted authinfo
//return HANDLER_GO_ON;
   // response_header_insert(srv, con,
    //                       CONST_STR_LEN("Location"),directurl_post,strlen(directurl_post));
    con->http_status = 200;
    con->mode = DIRECT;
    con->file_finished = 1;
    return HANDLER_FINISHED;
}

/**********************************************************************
 * module interface
 **********************************************************************/

INIT_FUNC(module_init) {
    plugin_data *pd;

    pd = calloc(1, sizeof(*pd));
    pd->users = array_init();
    pd->users2 = array_init();//2016.06.06 sherry add
	int fd, len, i=0;
    char ch, tmp[256], name[256], content[256];
    memset(tmp, 0, sizeof(tmp));
    memset(name, 0, sizeof(name));
    memset(content, 0, sizeof(content));
    memset(productname, '\0', sizeof(productname));
#ifdef DM_MIPSBIG
    mips_type = check_mips_type();
#endif
    if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0)
    {
        fprintf(stderr,"\nread conf error!\n");
    }
    else
    {

        while((len = read(fd, &ch, 1)) > 0)
        {
            if(ch == '=')
            {
                strcpy(name, tmp);
                memset(tmp, 0, sizeof(tmp));
                i = 0;
                continue;
            }
            else if(ch == '\n')
            {
                strcpy(content, tmp);
                memset(tmp, 0, sizeof(tmp));
                i = 0;
                if(!strcmp(name, "PRODUCTID"))
                {
                    sprintf(productname, "%s", content);
                }
                continue;
            }


            memcpy(tmp+i, &ch, 1);
            i++;
        }
        close(fd);
    }

    return pd;
}

FREE_FUNC(module_free) {
    plugin_data *pd = p_d;

    if (! pd) return HANDLER_GO_ON;

    // Free plugin data
    array_free(pd->users);
	array_free(pd->users2);

    // Free configuration data.
    // This must be done for each context.
    if (pd->config) {
        size_t i;
        for (i = 0; i < srv->config_context->used; i++) {
            plugin_config *pc = pd->config[i];
            if (! pc) continue;

            // free configuration
            buffer_free(pc->name);
            buffer_free(pc->authurl);
            buffer_free(pc->key);

            free(pc);
        }
        free(pd->config);
    }
    free(pd);

    return HANDLER_GO_ON;
}

unsigned char *li_base64_encode(const unsigned char *str)
{
    static const char base64_table[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
  };
    static const char base64_pad = '=';
    const unsigned char *current = str;
	int length = strlen(str);
    unsigned char *p;
    unsigned char *result;

    if ((length + 2) < 0 || ((length + 2) / 3) >= (1 << (sizeof(int) * 8 - 2))) {
        return NULL;
    }

    result = (unsigned char *)malloc(((length + 2) / 3) * 4 * sizeof(char) + 1);
    p = result;

    while (length > 2) {
        *p++ = base64_table[current[0] >> 2];
        *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
        *p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
        *p++ = base64_table[current[2] & 0x3f];

        current += 3;
        length -= 3;
    }

    if (length != 0) {
        *p++ = base64_table[current[0] >> 2];
        if (length > 1) {
            *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
            *p++ = base64_table[(current[1] & 0x0f) << 2];
            *p++ = base64_pad;
        } else {
            *p++ = base64_table[(current[0] & 0x03) << 4];
            *p++ = base64_pad;
            *p++ = base64_pad;
        }
    }
    *p = '\0';
    return result;
}



/*  Function:   check the file whether on the filepath and exclude the directory
*/
int
        check_if_file_exist(const char *filepath)
{

    struct stat st;
    return (stat(filepath, &st) == 0) && (!S_ISDIR(st.st_mode));
}
int parse_postdata_from_chunkqueue(server *srv, connection *con, chunkqueue *cq, char **ret_data) {

    int res = -1;

    chunk *c;
    char* result_data = NULL;

    UNUSED(srv);
    UNUSED(con);

    for (c = cq->first; cq->bytes_out != cq->bytes_in; c = cq->first) {
        size_t weWant = cq->bytes_out - cq->bytes_in;
        size_t weHave;

        switch(c->type) {
            case MEM_CHUNK:
                weHave = c->mem->used - 1 - c->offset;

                if (weHave > weWant) weHave = weWant;

                result_data = (char*)malloc(sizeof(char)*(weHave+1));
                memset(result_data, 0, sizeof(char)*(weHave+1));
                strcpy(result_data, c->mem->ptr + c->offset);

                c->offset += weHave;
                cq->bytes_out += weHave;

                res = 0;

                break;

            default:
                break;
        }

        chunkqueue_remove_finished_chunks(cq);
    }

    *ret_data = result_data;

    return res;
}
/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
    {
    d = b64_decode_table[(int)*cp];
    if ( d != -1 )
        {
        switch ( phase )
        {
        case 0:
        ++phase;
        break;
        case 1:
        c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
        if ( space_idx < size )
            space[space_idx++] = c;
        ++phase;
        break;
        case 2:
        c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
        if ( space_idx < size )
            space[space_idx++] = c;
        ++phase;
        break;
        case 3:
        c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
        if ( space_idx < size )
            space[space_idx++] = c;
        phase = 0;
        break;
        }
        prev_d = d;
        }
    }
    return space_idx;
}
//
// authorization handler
//
URIHANDLER_FUNC(module_uri_handler) {
    char *post_data;
    if((con->request.http_method == HTTP_METHOD_POST)&&(strstr(con->request.uri->ptr, "dm_uploadbt.cgi")==NULL)){
    if( parse_postdata_from_chunkqueue(srv, con, con->request_content_queue, &post_data) == 0 ){
        strcat(con->request.uri->ptr,"?");
        strcat(con->request.uri->ptr,post_data);
     }
    else{
        //fprintf(stderr,"\n------ Get post_data error ------ \n");
    }
    }
    if(con->request.uri->ptr == NULL){
        return HANDLER_ERROR;
    }
    if( strstr(con->request.uri->ptr,"mediasever_path=%3C%2Fmnt") || strstr(con->request.uri->ptr,"%3CAPV") ){
        return HANDLER_GO_ON;
    }
    else if(strstr(con->request.uri->ptr,">")||strstr(con->request.uri->ptr,"<")||strstr(con->request.uri->ptr,"%3E")||strstr(con->request.uri->ptr,"%3C")||strstr(con->request.uri->ptr,"%0A")||strstr(con->request.uri->ptr,"%0D")){
            con->http_status = 404;
        return HANDLER_FINISHED;
    }
    //leo added for lock username {
    if(strstr(con->request.uri->ptr,"mediaserver"))
    {
        memset(appname,0,sizeof(appname));
        snprintf(appname,12,"%s","mediaserver");
        if (flag_up_MS==1)
        {
            time_t t_p=time(NULL);
            if((t_p-time_MS)>60)
            {

                Username_Pw_MS=0;
                flag_up_MS=0;
                time_MS=0;
                if(access("/tmp/username_pw_MS.txt",0)==0)
                {
                    system("rm -rf /tmp/username_pw_MS.txt");
                }
            }
            else
            {
                if(strstr(con->request.uri->ptr,"images") || strstr(con->request.uri->ptr,".js"))
                {

                    return HANDLER_GO_ON;
                }
                char dirceturl_k[128];
                memset(dirceturl_k,0,sizeof(dirceturl_k));
                if(strstr(direct_url_string->ptr,"mediaserverui"))
                    sprintf(dirceturl_k,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
				else
                    sprintf(dirceturl_k,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                response_header_insert(srv, con,
                                       CONST_STR_LEN("Location"),dirceturl_k,strlen(dirceturl_k));
                con->http_status = 401;
                con->mode = DIRECT;
                con->file_finished = 1;
                return HANDLER_FINISHED;
            }
        }
    }
    else  if (flag_up_DM==1)
    {
        memset(appname,0,sizeof(appname));
        snprintf(appname,15,"%s","downloadmaster");
        time_t t_p=time(NULL);
        if((t_p-time_DM)>60)
        {
            Username_Pw_DM=0;
            flag_up_DM=0;
            time_DM=0;
            if(access("/tmp/username_pw_DM.txt",0)==0)
            {
                system("rm -rf /tmp/username_pw_DM.txt");
            }
        }
        else
        {

            if(strstr(con->request.uri->ptr,"images") || strstr(con->request.uri->ptr,".js") || strstr(con->request.uri->ptr,".ico"))
            {
                return HANDLER_GO_ON;
            }


            char dirceturl_k[128];
            memset(dirceturl_k,0,sizeof(dirceturl_k));
            if(strstr(direct_url_string->ptr,"mediaserverui"))
                sprintf(dirceturl_k,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
			else
                sprintf(dirceturl_k,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
            response_header_insert(srv, con,
                                   CONST_STR_LEN("Location"),dirceturl_k,strlen(dirceturl_k));
            con->http_status = 401;
            con->mode = DIRECT;
            con->file_finished = 1;
            return HANDLER_FINISHED;
        }
    }
    else
    {
        memset(appname,0,sizeof(appname));
        snprintf(appname,15,"%s","downloadmaster");
    }

    //leo added for lock username }
    plugin_data   *pd = p_d;
    plugin_config *pc = merge_config(srv, con, pd);
    data_string *ds;
    char buf[1024]; // cookie content
    char key[32];   // <AuthName> key
    char *cs;       // pointer to (some part of) <AuthName> key
    // skip if not enabled
    if (buffer_is_empty(pc->name)) return HANDLER_GO_ON;
    if (access("/tmp/have_dm2",0) != 0 && strstr(con->request.uri->ptr, "/downloadmaster")!=NULL && strstr(con->request.uri->ptr, ".asp")!=NULL){
        con->http_status = 599;
        con->mode = DIRECT;
        con->file_finished = 1;
        return HANDLER_FINISHED;
    }
    if (access("/tmp/have_ms" ,0) != 0 && strstr(con->request.uri->ptr, "/mediaserverui" ) != NULL)
    {
        con->http_status = 404;
        return HANDLER_FINISHED;
    }
    if(!strcmp(con->request.uri->ptr,"/Main_Login.asp")){
        char dirceturl_tmp[128];
        memset(dirceturl_tmp,0,sizeof(dirceturl_tmp));
        //neal modify begin {

        if(access("/tmp/have_dm2",0) == 0) {
            sprintf(dirceturl_tmp,"%s%s%s","/Main_Login.asp?flag=1&productname=",productname,"&url=/downloadmaster/task.asp");
        } else if(access("/tmp/have_ms",0) == 0) {
            sprintf(dirceturl_tmp,"%s%s%s","/Main_Login.asp?flag=1&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
        } else {
            con->http_status = 404;
            return HANDLER_FINISHED;
        }


        response_header_insert(srv, con,
                               CONST_STR_LEN("Location"),dirceturl_tmp,strlen(dirceturl_tmp));
        con->http_status = 307;
        con->mode = DIRECT;
        con->file_finished = 1;
        return HANDLER_FINISHED;
    } else if(!strcmp(con->request.uri->ptr,"/")) {
        if(access("/tmp/have_dm2",0)==0) {
            return HANDLER_GO_ON;
        } else if(access("/tmp/have_ms",0)==0) {
            char dirceturl[128];
            memset(dirceturl,0,sizeof(dirceturl));
            sprintf(dirceturl,"%s","/mediaserverui/mediaserver.asp");
            response_header_insert(srv, con,
                                   CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
            con->http_status    = 307;
            con->mode           = DIRECT;
            con->file_finished  = 1;
            return HANDLER_FINISHED;
        } else {
            con->http_status = 404;
            return HANDLER_FINISHED;
        }
        //neal 20160908 modify end }
	} else if(strstr(con->request.uri->ptr,"Main_Login.asp")){
		//TODO: the address for ipv6
		return check_can_login(srv, con, pd);

	} else if (strstr(con->request.uri->ptr,"log.asp") || strstr(con->request.uri->ptr,"images") || strstr(con->request.uri->ptr,".js") || strstr(con->request.uri->ptr,"get_language"))
    {//2016.06.12 sherry add uri=/ go on
        return HANDLER_GO_ON;
    }

    if(strstr(con->request.uri->ptr,"Logout.asp")){
        // pass through if no redirect target is specified
        if (buffer_is_empty(pc->authurl)) {
            //DEBUG("s", "endauth - continuing");
            return HANDLER_GO_ON;
        }
        //DEBUG("sb", "endauth - redirecting:", pc->authurl);

        buffer *field = buffer_init();
        //2016.06.06 sherry modify{
        buffer_copy_buffer(field, pc->name);
        buffer_append_string(field, "=");
        buffer_append_string(field, "; ");
        //buffer_append_string_buffer(field, pc->options);
        if(strstr(con->request.uri->ptr,"mediaserverui")){
            buffer_append_string(field, "path=/mediaserverui/; httponly;");
			login_ip_MS = 0;
			last_access_time_MS = 0;
        }
        else{
            buffer_append_string(field, "path=/downloadmaster/; httponly;");
			login_ip_DM = 0;
			last_access_time_DM = 0;
        }
        response_header_append(srv, con,
                               CONST_STR_LEN("Set-Cookie"), CONST_BUF_LEN(field));
        buffer_free(field);

        // prepare redirection header
        buffer *url = buffer_init_buffer(pc->authurl);
        buffer_append_string(url, "?flag=8");
        buffer_append_string(url, "&productname=");
        buffer_append_string(url, productname);
        buffer_append_string(url, strchr(url->ptr, '?') ? "&url=" : "?url=");
        //DEBUG("sb","url :",url);
        char *nv,*nvp,*b=NULL, *mytmp=NULL;
        nv = nvp = strdup(con->uri.query->ptr);
        if(nv){
            while((b=strsep(&nvp,"&")) != NULL){
                //DEBUG("ss","b :",b);
                if(!strncmp(b,"directurl=",10)){

                    mytmp = b+10;
                    buffer_append_string(url, mytmp);
                    //end 2016.06.08 sherry modify
                }
            }
            free(nv);
        }
        //2016.06.07 sherry add
        if(!mytmp){
            if(strstr(con->request.uri->ptr,"mediaserverui"))
                buffer_append_string(url, "/mediaserverui/mediaserver.asp");
            else if(strstr(con->request.uri->ptr,"downloadmaster"))
                buffer_append_string(url, "/downloadmaster/task.asp");
        }
        //DEBUG("sb", "Location url: ",url);
        response_header_insert(srv, con,
                               CONST_STR_LEN("Location"), CONST_BUF_LEN(url));
        buffer_free(url);

        // prepare response
        con->http_status = 307;
        con->mode = DIRECT;
        con->file_finished = 1;

        return HANDLER_FINISHED;
    }

    if(strstr(con->request.uri->ptr,"check.asp")){
		int ret = check_can_login(srv, con, pd);
		if( ret != HANDLER_GO_ON)
			return ret;
        char *auth_login_u;
        char *auth_login_p;
        int l;
        char login_authinfo[256];
        char login_authpasw[256];
        char check_user_tmp[256];
        char check_passwd_tmp[256];
        char enc_passwd[256];
        char *dut_username=NULL;
        char *dut_password=NULL;
        memset(check_user_tmp, 0, sizeof(check_user_tmp));
        memset(check_passwd_tmp, 0, sizeof(check_passwd_tmp));
        memset(enc_passwd, 0, sizeof(enc_passwd));
        memset(directurl_post, 0, sizeof(directurl_post));
        memset(login_authinfo, 0, sizeof(login_authinfo));
        memset(login_authpasw, 0, sizeof(login_authpasw));
        direct_url_string = buffer_init();
        char *nv,*nvp,*b;
        //nv = nvp = strdup(con->uri.query->ptr);
        nv = nvp = strdup(con->request.uri->ptr);

        if(nv){
            while((b=strsep(&nvp,"&")) != NULL){
                if(!strncmp(b,"login_username=",15)){
                    url_ascii_to_char(login_authinfo, b+15, 256);
                }
                else if(!strncmp(b,"login_passwd=",13)){
                    url_ascii_to_char(login_authpasw, b+13, 256);
                }
                else if(!strncmp(b,"directurl=",10)){
                    buffer_copy_string(direct_url_string,b+10);
                    buffer_urldecode_path(direct_url_string);
                    url_ascii_to_char(directurl_post, b+10, 256);
                }
            }
            free(nv);
        }
        /* Decode it. */
       if(strstr(con->request.request->ptr,"ASUSDMUtility")||(con->request.http_method == HTTP_METHOD_POST)){
        auth_login_u=strdup(login_authinfo);
        auth_login_p=strdup(login_authpasw);
        l = b64_decode( &(auth_login_u[0]), (unsigned char*) check_user_tmp, sizeof(check_user_tmp) );
        check_user_tmp[l] = '\0';
        l = b64_decode( &(auth_login_p[0]), (unsigned char*) check_passwd_tmp, sizeof(check_passwd_tmp) );
        check_passwd_tmp[l] = '\0';
        free(auth_login_u);
        free(auth_login_p);
       }else{
          snprintf(check_user_tmp,sizeof(check_user_tmp),"%s",login_authinfo);
          snprintf(check_passwd_tmp,sizeof(check_passwd_tmp),"%s",login_authpasw);
           }

#ifdef DM_MIPSBIG
          dut_username=(char *)malloc(32);
          dut_password=(char *)malloc(128);
          memset(dut_username,0,32);
          memset(dut_password,0,128);
          if(access("/userfs/bin/tcapi",0) == 0){
          dut_username=tcapi_get_by_popen("Account_Entry0","username");
          dut_password=tcapi_get_by_popen("Account_Entry0","web_passwd");
          }else{
              dut_username=nvram_get_by_popen("http_username");
              dut_password=nvram_get_by_popen("http_passwd");
          }
#else
          dut_username=(char *)malloc(32);
          dut_password=(char *)malloc(128);
          memset(dut_username,0,32);
          memset(dut_password,0,128);
          dut_username=nvram_get_by_popen("http_username");
          dut_password=nvram_get_by_popen("http_passwd");
#endif
        /////////////////}}}
        ///20160622 libpasswd for shadow{
        if(access("/etc/shadow",0)==0){
        // etc/shadow is exist
            int value_u=compare_passwd_in_shadow(check_user_tmp,check_passwd_tmp);
            if(value_u){
                if(strstr(con->request.uri->ptr,"downloadmaster"))
                {
                    Username_Pw_DM=0;
                }
                if(strstr(con->request.uri->ptr,"mediaserver"))
                {
                    Username_Pw_MS=0;
                }
    // Add for check the direct url weather exit on the server
                if(direct_url_string->used>1)
                {
                    char filename[128];
                    memset(filename,0,sizeof(filename));
                    snprintf(filename,sizeof(filename),"/opt/etc/downloadmaster%s",direct_url_string->ptr);
                    if(check_if_file_exist(filename))
                    {
                        char cookie_string[256];
                        snprintf(cookie_string,sizeof(cookie_string),"%s:%s",check_user_tmp,check_passwd_tmp);
                        return handle_crypt(srv, con, pd, pc, cookie_string);
                    }
                }
                if(access("/tmp/have_dm2",0) == 0) {
                    buffer_copy_string(direct_url_string,"/downloadmaster/task.asp");
                } else if(access("/tmp/have_ms",0) == 0) {
                    buffer_copy_string(direct_url_string,"/mediaserverui/mediaserver.asp");
                } else {
                    con->http_status = 404;
                    return HANDLER_FINISHED;
                }
                char cookie_string[256];
                snprintf(cookie_string, sizeof(cookie_string), "%s:%s",check_user_tmp,check_passwd_tmp);
                return handle_crypt(srv, con, pd, pc, cookie_string);
            }else{
                if(strstr(con->request.uri->ptr,"downloadmaster"))
                {
                    Username_Pw_DM++;
                }
                if(strstr(con->request.uri->ptr,"mediaserver"))
                {
                    Username_Pw_MS++;
                }
                char dirceturl[128];
                char tr[128];
                memset(dirceturl,0,sizeof(dirceturl));
                memset(tr,0,sizeof(tr));

                if(Username_Pw_DM>=5)
                {
                    Username_Pw_DM=0;
                    flag_up_DM=1;
                    time_t t_l=time(NULL);
                    time_DM=t_l;
                    sprintf(tr,"%s %ld %s","echo",time_DM," > /tmp/username_pw_DM.txt");
                    system(tr);
                    if(direct_url_string->used > 1) {
                        if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                            sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                    } else {
                        sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                    }
                    response_header_insert(srv, con,
                                           CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                    con->http_status = 401;
                    con->mode = DIRECT;
                    con->file_finished = 1;
                    return HANDLER_FINISHED;
                }
                else if(Username_Pw_MS>=5)
                {
                    Username_Pw_MS=0;
                    flag_up_MS=1;
                    time_t t_l=time(NULL);
                    time_MS=t_l;
                    sprintf(tr,"%s %ld %s","echo",time_MS," > /tmp/username_pw_MS.txt");
                    system(tr);
                    if(direct_url_string->used > 1) {
                        if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                            sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                    } else {
                        sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                    }
                    response_header_insert(srv, con,
                                           CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                    con->http_status = 401;
                    con->mode = DIRECT;
                    con->file_finished = 1;
                    return HANDLER_FINISHED;
                } else {
                    if(direct_url_string->used >1 ) {
                        if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                            sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                    } else {
                        sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/");
                    }
                    response_header_insert(srv, con,
                                           CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                    con->http_status = 401;
                    con->mode = DIRECT;
                    con->file_finished = 1;
                    return HANDLER_FINISHED;
                }
            }
        }else{
            nvramver=nvram_encrypt_support();
       if(nvramver==1){
               //nvram is encrypted
           char dec_passwd[128];
           memset(dec_passwd,0,128);
           pw_dec(dut_password,dec_passwd, sizeof(dec_passwd));
           decode_url(check_user_tmp,1);
            /*   char command_name[128];
               memset(command_name,'\0',sizeof(command_name));
               sprintf(command_name,"%s","/tmp/APPS/Lighttpd/Script/asus_http_usercheck user-renew-nvram-en");
               system(command_name);

               char http_username[1][512];
               char username_check[256];
               if(access("/tmp/APPS/Lighttpd/Config/asus_lighttpdpassword",0)== 0){

                   memset(http_username, '\0', sizeof(http_username));
                   memset(username_check, '\0', sizeof(username_check));
                   FILE *fp;
                   if((fp=fopen("/tmp/APPS/Lighttpd/Config/asus_lighttpdpassword","r"))!=NULL){
                       if(fscanf(fp,"%[^:]%*s",username_check)>0){
                           sprintf(username_check,"%s",username_check);
                       }
                       fclose(fp);
                   }
               }

               char buf_passwd[256];// max password is 15
            */
           /* buf_passwd is ascii
            * check_passwd_tmp is char
            */
             /*  char_to_ascii_safe(buf_passwd, check_passwd_tmp, 256);
               char command[512];
               memset(command,'\0',sizeof(command));
               sprintf(command,"%s \"%s\"","/tmp/APPS/Lighttpd/Script/asus_http_check passwd-renew-nvram-en",buf_passwd);
               system(command);

               if(access("/tmp/http_info_changed",0)== 0){
                   memset(http_pw, '\0', sizeof(http_pw));
                   memset(passwd_check, '\0', sizeof(passwd_check));
                   FILE *fp;
                   if((fp=fopen("/tmp/http_info","r"))!=NULL){
                       if(fscanf(fp,"%[^\n]%*s",passwd_check)>0){
                           sprintf(passwd_check,"%s",passwd_check);
                       }
                       fclose(fp);
                   }

                   memset(http_pw_check,'\0',sizeof(http_pw_check));
                   sprintf(http_pw_check,"%s",passwd_check);
                   decode_url(http_pw_check,0); //2014.02.18 magic modify
                   system("rm -f /tmp/http_info_changed");
               }
               char dec_passwd[64];
               char http_pw_check_en[512];
               memset(dec_passwd,0,sizeof(dec_passwd));
               memset(http_pw_check_en,0,sizeof(http_pw_check_en));
               pw_dec(http_pw_check,dec_passwd);
               li_MD5_CTX Md5Ctx,Md5Ctx1,Md5Ctx2;
               HASH HA1,HA2,HA3;
               char passwd1[256];
               char passwd2[256];

               memset(passwd1,'\0',sizeof(passwd1));
               memset(passwd2,'\0',sizeof(passwd2));
               li_MD5_Init(&Md5Ctx);
               li_MD5_Update(&Md5Ctx, (unsigned char *)check_passwd_tmp, strlen(check_passwd_tmp));
               li_MD5_Final(HA1, &Md5Ctx);
               CvtHex(HA1, passwd1);

               li_MD5_Init(&Md5Ctx1);
               li_MD5_Update(&Md5Ctx1, (unsigned char *)buf_passwd, strlen(buf_passwd));
               li_MD5_Final(HA2, &Md5Ctx1);
               CvtHex(HA2, passwd2);

               li_MD5_Init(&Md5Ctx2);
               li_MD5_Update(&Md5Ctx2, (unsigned char *)dec_passwd, strlen(dec_passwd));
               li_MD5_Final(HA3, &Md5Ctx2);
               CvtHex(HA3, http_pw_check_en);
               char buf_username[256];
               char_to_ascii_safe(buf_username, check_user_tmp, 256);
               // mipsbig don't need to change to ascii
               */
               if ((!strcmp(dut_username, check_user_tmp) ) && ( !strcmp(dec_passwd,check_passwd_tmp))) {
                   if(strstr(con->request.uri->ptr,"downloadmaster"))
                   {
                       Username_Pw_DM=0;
                   }
                   if(strstr(con->request.uri->ptr,"mediaserver"))
                   {
                       Username_Pw_MS=0;
                   }
       // Add for check the direct url weather exit on the server
                   if(direct_url_string->used>1)
                   {
                       char filename[128];
                       memset(filename,0,sizeof(filename));
                       snprintf(filename,sizeof(filename),"/opt/etc/downloadmaster%s",direct_url_string->ptr);
                       if(check_if_file_exist(filename))
                       {
                           char cookie_string[256];
                           snprintf(cookie_string,sizeof(cookie_string),"%s:%s",check_user_tmp,check_passwd_tmp);
                           return handle_crypt(srv, con, pd, pc, cookie_string);
                       }
                   }
                   if(access("/tmp/have_dm2",0) == 0) {
                       buffer_copy_string(direct_url_string,"/downloadmaster/task.asp");
                   } else if(access("/tmp/have_ms",0) == 0) {
                       buffer_copy_string(direct_url_string,"/mediaserverui/mediaserver.asp");
                   } else {
                       con->http_status = 404;
                       return HANDLER_FINISHED;
                   }
                   char cookie_string[256];
                   snprintf(cookie_string, sizeof(cookie_string), "%s:%s",check_user_tmp,check_passwd_tmp);
                   return handle_crypt(srv, con, pd, pc, cookie_string);
               }
               else{

                   // 20160608 leo addd for lock username and password {
                   if(strstr(con->request.uri->ptr,"downloadmaster"))
                   {

                       Username_Pw_DM++;
                   }
                   if(strstr(con->request.uri->ptr,"mediaserver"))
                   {
                       Username_Pw_MS++;

                   }
                   char dirceturl[128];
                   char tr[128];
                   memset(dirceturl,0,sizeof(dirceturl));
                   memset(tr,0,sizeof(tr));

                   if(Username_Pw_DM>=5)
                   {
                       Username_Pw_DM=0;
                       flag_up_DM=1;
                       time_t t_l=time(NULL);
                       time_DM=t_l;
                       sprintf(tr,"%s %ld %s","echo",time_DM," > /tmp/username_pw_DM.txt");
                       system(tr);
                       if(direct_url_string->used > 1) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
                   else if(Username_Pw_MS>=5)
                   {
                       Username_Pw_MS=0;
                       flag_up_MS=1;
                       time_t t_l=time(NULL);
                       time_MS=t_l;
                       sprintf(tr,"%s %ld %s","echo",time_MS," > /tmp/username_pw_MS.txt");
                       system(tr);
                       if(direct_url_string->used > 1) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
                   // 20160608 leo addd for lock username and password }
                   else
                   {
                       if(direct_url_string->used >1 ) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
               }
           }
           else {
             //nvram not encrypted
              decode_url(check_user_tmp,1);
               /*char command_name[128];
               memset(command_name,'\0',sizeof(command_name));
               sprintf(command_name,"%s","/tmp/APPS/Lighttpd/Script/asus_http_usercheck user-renew");
               system(command_name);

               char http_username[1][512];
               char username_check[256];
               if(access("/tmp/APPS/Lighttpd/Config/asus_lighttpdpassword",0)== 0){

                   memset(http_username, '\0', sizeof(http_username));
                   memset(username_check, '\0', sizeof(username_check));
                   FILE *fp;
                   if((fp=fopen("/tmp/APPS/Lighttpd/Config/asus_lighttpdpassword","r"))!=NULL){
                       if(fscanf(fp,"%[^:]%*s",username_check)>0){
                           sprintf(username_check,"%s",username_check);
                       }
                       fclose(fp);
                   }
               }

               char buf_passwd[256];// max password is 15
*/
           /* buf_passwd is ascii
            * check_passwd_tmp is char
            */
              /*
               char_to_ascii_safe(buf_passwd, check_passwd_tmp, 256);

               char command[512];
               memset(command,'\0',sizeof(command));
               sprintf(command,"%s \"%s\"","/tmp/APPS/Lighttpd/Script/asus_http_check passwd-renew",buf_passwd);
               system(command);

               if(access("/tmp/http_info_changed",0)== 0){
                   memset(http_pw, '\0', sizeof(http_pw));
                   memset(passwd_check, '\0', sizeof(passwd_check));
                   FILE *fp;
                   if((fp=fopen("/tmp/http_info","r"))!=NULL){
                       if(fscanf(fp,"%[^\n]%*s",passwd_check)>0){
                           sprintf(passwd_check,"%s",passwd_check);
                       }
                       fclose(fp);
                   }

                   memset(http_pw_check,'\0',sizeof(http_pw_check));
                   sprintf(http_pw_check,"%s",passwd_check);
                   decode_url(http_pw_check,0); //2014.02.18 magic modify
                   system("rm -f /tmp/http_info_changed");
               }
               li_MD5_CTX Md5Ctx,Md5Ctx1;
               HASH HA1,HA2;
               char passwd1[256];
               char passwd2[256];

               memset(passwd1,'\0',sizeof(passwd1));
               memset(passwd2,'\0',sizeof(passwd2));
               li_MD5_Init(&Md5Ctx);
               li_MD5_Update(&Md5Ctx, (unsigned char *)check_passwd_tmp, strlen(check_passwd_tmp));
               li_MD5_Final(HA1, &Md5Ctx);

               CvtHex(HA1, passwd1);

               li_MD5_Init(&Md5Ctx1);
               li_MD5_Update(&Md5Ctx1, (unsigned char *)buf_passwd, strlen(buf_passwd));
               li_MD5_Final(HA2, &Md5Ctx1);

               CvtHex(HA2, passwd2);

               char buf_username[256];
               char_to_ascii_safe(buf_username, check_user_tmp, 256);
               // mipsbig don't need to change to ascii
               */
               if ((!strcmp(dut_username, check_user_tmp)) && ( !strcmp(dut_password,check_passwd_tmp))) {
                   if(strstr(con->request.uri->ptr,"downloadmaster"))
                   {
                       Username_Pw_DM=0;
                   }
                   if(strstr(con->request.uri->ptr,"mediaserver"))
                   {
                       Username_Pw_MS=0;
                   }
       // Add for check the direct url weather exit on the server
                   if(direct_url_string->used>1)
                   {
                       char filename[128];
                       memset(filename,0,sizeof(filename));
                       snprintf(filename,sizeof(filename),"/opt/etc/downloadmaster%s",direct_url_string->ptr);
                       if(check_if_file_exist(filename))
                       {
                           char cookie_string[256];
                           snprintf(cookie_string,sizeof(cookie_string),"%s:%s",check_user_tmp,check_passwd_tmp);
                           return handle_crypt(srv, con, pd, pc, cookie_string);
                       }
                   }
                   if(access("/tmp/have_dm2",0) == 0) {
                       buffer_copy_string(direct_url_string,"/downloadmaster/task.asp");
                   } else if(access("/tmp/have_ms",0) == 0) {
                       buffer_copy_string(direct_url_string,"/mediaserverui/mediaserver.asp");
                   } else {
                       con->http_status = 404;
                       return HANDLER_FINISHED;
                   }
                   char cookie_string[256];
                   snprintf(cookie_string, sizeof(cookie_string), "%s:%s",check_user_tmp,check_passwd_tmp);
                   return handle_crypt(srv, con, pd, pc, cookie_string);
               }
               else{

                   // 20160608 leo addd for lock username and password {
                   if(strstr(con->request.uri->ptr,"downloadmaster"))
                   {

                       Username_Pw_DM++;
                   }
                   if(strstr(con->request.uri->ptr,"mediaserver"))
                   {
                       Username_Pw_MS++;

                   }
                   char dirceturl[128];
                   char tr[128];
                   memset(dirceturl,0,sizeof(dirceturl));
                   memset(tr,0,sizeof(tr));

                   if(Username_Pw_DM>=5)
                   {
                       Username_Pw_DM=0;
                       flag_up_DM=1;
                       time_t t_l=time(NULL);
                       time_DM=t_l;
                       sprintf(tr,"%s %ld %s","echo",time_DM," > /tmp/username_pw_DM.txt");
                       system(tr);
                       if(direct_url_string->used > 1) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
                   else if(Username_Pw_MS>=5)
                   {
                       Username_Pw_MS=0;
                       flag_up_MS=1;
                       time_t t_l=time(NULL);
                       time_MS=t_l;
                       sprintf(tr,"%s %ld %s","echo",time_MS," > /tmp/username_pw_MS.txt");
                       system(tr);
                       if(direct_url_string->used > 1) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=7&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
                   // 20160608 leo addd for lock username and password }
                   else
                   {
                       if(direct_url_string->used >1 ) {
                           if(strstr(direct_url_string->ptr,"mediaserverui"))//neal 20160908 modify
                               sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/mediaserverui/mediaserver.asp");
                       } else {
                           sprintf(dirceturl,"%s%s%s","/Main_Login.asp?flag=3&productname=",productname,"&url=/");
                       }
                       response_header_insert(srv, con,
                                              CONST_STR_LEN("Location"),dirceturl,strlen(dirceturl));
                       con->http_status = 401;
                       con->mode = DIRECT;
                       con->file_finished = 1;
                       return HANDLER_FINISHED;
                   }
               }
           }
        }

        ///20160622 libpasswd for shadow}
        return HANDLER_GO_ON;
	}
    // decide how to handle incoming Auth header
    if ((ds = HEADER(con, "Authorization")) != NULL) {
        switch (pc->override) {
        case 0: return HANDLER_GO_ON;   // just use it if supplied
        case 1: break;                  // use CookieAuth if exists
        case 2:
        default: buffer_reset(ds->key); // use CookieAuth only
        }
    }
	time_t current_time = uptime();
	if(strstr(con->request.uri->ptr, "downloadmaster")) {
		if((unsigned long)(current_time-last_access_time_DM) > (10*60)){
			array_reset(pd->users2);
			return endauth(srv, con, pc);
		}
	} else if(strstr(con->request.uri->ptr, "mediaserverui")) {
		if((unsigned long)(current_time-last_access_time_MS) > (10*60)){
			array_reset(pd->users);
			return endauth(srv, con, pc);
		}
	}

    // check for cookie
    if ((ds = HEADER(con, "Cookie")) == NULL) {
        return endauth(srv, con, pc);
    }
    // prepare cstring for processing
    memset(key, 0, sizeof(key));
    memset(buf, 0, sizeof(buf));
    strncpy(key, pc->name->ptr,  min(sizeof(key) - 1, pc->name->used));
    strncpy(buf, ds->value->ptr, min(sizeof(buf) - 1, ds->value->used));

    // check for "<AuthName>=" entry in a cookie
    for (cs = buf; (cs = strstr(cs, key)) != NULL; ) {

        // check if found entry matches exactly for "KEY=" part.
        cs += pc->name->used - 1;  // jump to the end of "KEY" part
        while (isspace(*cs)) cs++; // whitespace can be skipped

        // break forward if this was an exact match
        if (*cs++ == '=') {
            char *eot = strchr(cs, ';');
            if (eot) *eot = '\0';
            break;
        }
    }
    if (! cs)
    {
        return endauth(srv, con, pc); // not found - rejecting
    }
    // unescape payload
    buffer *tmp = buffer_init_string(cs);
    buffer_urldecode_path(tmp);
    memset(buf, 0, sizeof(buf));
    strncpy(buf, tmp->ptr, min(sizeof(buf) - 1, tmp->used));
    buffer_free(tmp);
    cs = buf;
    // Allow access if client already has an "authorized" token.
    if (strncmp(cs, "asus_app_token:", 15) == 0) {
        return handle_token(srv, con, pd, pc, cs + 15);
    }

    // Verify "non-authorized" CookieAuth request in encrypted format.
    // Once verified, give out authorized token ("token:..." cookie).
    return endauth(srv, con, pc);
}

SETDEFAULTS_FUNC(module_set_defaults) {
    //fprintf(stderr,"\nmodule_set_defaults\n");
    plugin_data *pd = p_d;
    size_t i;

	config_values_t cv[] = {
		{ "auth-ticket.loglevel",
		  NULL, T_CONFIG_INT,    T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.name",
		  NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.override",
		  NULL, T_CONFIG_INT,    T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.authurl",
		  NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.key",
		  NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.timeout",
		  NULL, T_CONFIG_INT,    T_CONFIG_SCOPE_CONNECTION },
		{ "auth-ticket.options",
		  NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION },
		{ NULL, NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

    pd->config = calloc(1,
                        srv->config_context->used * sizeof(specific_config *));

    for (i = 0; i < srv->config_context->used; i++) {
	data_config const* config = (data_config const*)srv->config_context->data[i];
        plugin_config *pc;

        pc = pd->config[i] = calloc(1, sizeof(plugin_config));
        pc->loglevel = 1;
        pc->name     = buffer_init();
        pc->override = 2;
        pc->authurl  = buffer_init();
        pc->key      = buffer_init();
        pc->timeout  = 86400;
        pc->options  = buffer_init();

        cv[0].destination = &(pc->loglevel);
        cv[1].destination = pc->name;
        cv[2].destination = &(pc->override);
        cv[3].destination = pc->authurl;
        cv[4].destination = pc->key;
        cv[5].destination = &(pc->timeout);
        cv[6].destination = pc->options;

	if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
            return HANDLER_ERROR;
        }
    }
    return HANDLER_GO_ON;
}

int
        mod_auth_ticket_plugin_init(plugin *p) {
    p->version          = LIGHTTPD_VERSION_ID;
    p->name             = buffer_init_string("auth_ticket");
    p->init             = module_init;
    p->set_defaults     = module_set_defaults;
    p->cleanup          = module_free;
    p->handle_uri_clean = module_uri_handler;
    p->data             = NULL;

    return 0;
}
