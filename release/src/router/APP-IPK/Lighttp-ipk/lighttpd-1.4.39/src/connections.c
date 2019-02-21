#include "buffer.h"
#include "server.h"
#include "log.h"
#include "connections.h"
#include "fdevent.h"

#include "request.h"
#include "response.h"
#include "network.h"
#include "http_chunk.h"
#include "stat_cache.h"
#include "joblist.h"

#include "plugin.h"

#include "inet_ntop_cache.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#ifdef USE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#include "sys-socket.h"

#include "dm_func.h"

typedef struct {
    PLUGIN_DATA;
} plugin_data;

static connection *connections_get_new_connection(server *srv) {
    connections *conns = srv->conns;
    size_t i;

    if (conns->size == 0) {
        conns->size = 128;
        conns->ptr = NULL;
        conns->ptr = malloc(sizeof(*conns->ptr) * conns->size);
        for (i = 0; i < conns->size; i++) {
            conns->ptr[i] = connection_init(srv);
        }
    } else if (conns->size == conns->used) {
        conns->size += 128;
        conns->ptr = realloc(conns->ptr, sizeof(*conns->ptr) * conns->size);

        for (i = conns->used; i < conns->size; i++) {
            conns->ptr[i] = connection_init(srv);
        }
    }

    connection_reset(srv, conns->ptr[conns->used]);
#if 0
    fprintf(stderr, "%s.%d: add: ", __FILE__, __LINE__);
    for (i = 0; i < conns->used + 1; i++) {
        fprintf(stderr, "%d ", conns->ptr[i]->fd);
    }
    fprintf(stderr, "\n");
#endif

    conns->ptr[conns->used]->ndx = conns->used;
    return conns->ptr[conns->used++];
}

static int connection_del(server *srv, connection *con) {
    size_t i;
    connections *conns = srv->conns;
    connection *temp;

    if (con == NULL) return -1;

    if (-1 == con->ndx) return -1;

    buffer_reset(con->uri.authority);
    buffer_reset(con->uri.path);
    buffer_reset(con->uri.query);
    buffer_reset(con->request.orig_uri);

    i = con->ndx;

    /* not last element */

    if (i != conns->used - 1) {
        temp = conns->ptr[i];
        conns->ptr[i] = conns->ptr[conns->used - 1];
        conns->ptr[conns->used - 1] = temp;

        conns->ptr[i]->ndx = i;
        conns->ptr[conns->used - 1]->ndx = -1;
    }

    conns->used--;

    con->ndx = -1;
#if 0
    fprintf(stderr, "%s.%d: del: (%d)", __FILE__, __LINE__, conns->used);
    for (i = 0; i < conns->used; i++) {
        fprintf(stderr, "%d ", conns->ptr[i]->fd);
    }
    fprintf(stderr, "\n");
#endif
    return 0;
}

int connection_close(server *srv, connection *con) {
#ifdef USE_OPENSSL
    server_socket *srv_sock = con->srv_socket;
#endif

#ifdef USE_OPENSSL
    if (srv_sock->is_ssl) {
        if (con->ssl) SSL_free(con->ssl);
        con->ssl = NULL;
    }
#endif

    fdevent_event_del(srv->ev, &(con->fde_ndx), con->fd);
    fdevent_unregister(srv->ev, con->fd);
#ifdef __WIN32
    if (closesocket(con->fd)) {
        log_error_write(srv, __FILE__, __LINE__, "sds",
                        "(warning) close:", con->fd, strerror(errno));
    }
#else
    if (close(con->fd)) {
        log_error_write(srv, __FILE__, __LINE__, "sds",
                        "(warning) close:", con->fd, strerror(errno));
    }
#endif

    srv->cur_fds--;
#if 0
    log_error_write(srv, __FILE__, __LINE__, "sd",
                    "closed()", con->fd);
#endif

    connection_del(srv, con);
    connection_set_state(srv, con, CON_STATE_CONNECT);

    return 0;
}

#if 0
static void dump_packet(const unsigned char *data, size_t len) {
    size_t i, j;

    if (len == 0) return;

    for (i = 0; i < len; i++) {
        if (i % 16 == 0) fprintf(stderr, "  ");

        fprintf(stderr, "%02x ", data[i]);

        if ((i + 1) % 16 == 0) {
            fprintf(stderr, "  ");
            for (j = 0; j <= i % 16; j++) {
                unsigned char c;

                if (i-15+j >= len) break;

                c = data[i-15+j];

                fprintf(stderr, "%c", c > 32 && c < 128 ? c : '.');
            }

            fprintf(stderr, "\n");
        }
    }

    if (len % 16 != 0) {
        for (j = i % 16; j < 16; j++) {
            fprintf(stderr, "   ");
        }

        fprintf(stderr, "  ");
        for (j = i & ~0xf; j < len; j++) {
            unsigned char c;

            c = data[j];
            fprintf(stderr, "%c", c > 32 && c < 128 ? c : '.');
        }
        fprintf(stderr, "\n");
    }
}
#endif

static int connection_handle_read_ssl(server *srv, connection *con) {
#ifdef USE_OPENSSL
    int r, ssl_err, len, count = 0;
    char *mem = NULL;
    size_t mem_len = 0;

    if (!con->srv_socket->is_ssl) return -1;

    ERR_clear_error();
    do {
        chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, SSL_pending(con->ssl));
#if 0
        /* overwrite everything with 0 */
        memset(mem, 0, mem_len);
#endif

        len = SSL_read(con->ssl, mem, mem_len);
        chunkqueue_use_memory(con->read_queue, len > 0 ? len : 0);

        if (con->renegotiations > 1 && con->conf.ssl_disable_client_renegotiation) {
            log_error_write(srv, __FILE__, __LINE__, "s", "SSL: renegotiation initiated by client, killing connection");
            connection_set_state(srv, con, CON_STATE_ERROR);
            return -1;
        }

        if (len > 0) {
            con->bytes_read += len;
            count += len;
        }
    } while (len == (ssize_t) mem_len && count < MAX_READ_LIMIT);


    if (len < 0) {
        int oerrno = errno;
        switch ((r = SSL_get_error(con->ssl, len))) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            con->is_readable = 0;

            /* the manual says we have to call SSL_read with the same arguments next time.
             * we ignore this restriction; no one has complained about it in 1.5 yet, so it probably works anyway.
             */

            return 0;
        case SSL_ERROR_SYSCALL:
            /**
             * man SSL_get_error()
             *
             * SSL_ERROR_SYSCALL
             *   Some I/O error occurred.  The OpenSSL error queue may contain more
             *   information on the error.  If the error queue is empty (i.e.
             *   ERR_get_error() returns 0), ret can be used to find out more about
             *   the error: If ret == 0, an EOF was observed that violates the
             *   protocol.  If ret == -1, the underlying BIO reported an I/O error
             *   (for socket I/O on Unix systems, consult errno for details).
             *
             */
            while((ssl_err = ERR_get_error())) {
                /* get all errors from the error-queue */
                log_error_write(srv, __FILE__, __LINE__, "sds", "SSL:",
                                r, ERR_error_string(ssl_err, NULL));
            }

            switch(oerrno) {
            default:
                log_error_write(srv, __FILE__, __LINE__, "sddds", "SSL:",
                                len, r, oerrno,
                                strerror(oerrno));
                break;
            }

            break;
        case SSL_ERROR_ZERO_RETURN:
            /* clean shutdown on the remote side */

            if (r == 0) {
                /* FIXME: later */
            }

            /* fall thourgh */
        default:
            while((ssl_err = ERR_get_error())) {
                switch (ERR_GET_REASON(ssl_err)) {
                case SSL_R_SSL_HANDSHAKE_FAILURE:
                case SSL_R_TLSV1_ALERT_UNKNOWN_CA:
                case SSL_R_SSLV3_ALERT_CERTIFICATE_UNKNOWN:
                case SSL_R_SSLV3_ALERT_BAD_CERTIFICATE:
                    if (!con->conf.log_ssl_noise) continue;
                    break;
                default:
                    break;
                }
                /* get all errors from the error-queue */
                log_error_write(srv, __FILE__, __LINE__, "sds", "SSL:",
                                r, ERR_error_string(ssl_err, NULL));
            }
            break;
        }

        connection_set_state(srv, con, CON_STATE_ERROR);

        return -1;
    } else if (len == 0) {
        con->is_readable = 0;
        /* the other end close the connection -> KEEP-ALIVE */

        return -2;
    } else {
        joblist_append(srv, con);
    }

    return 0;
#else
    UNUSED(srv);
    UNUSED(con);
    return -1;
#endif
}

/* 0: everything ok, -1: error, -2: con closed */
static int connection_handle_read(server *srv, connection *con) {
    int len;
    char *mem = NULL;
    size_t mem_len = 0;
    int toread;

    if (con->srv_socket->is_ssl) {
        return connection_handle_read_ssl(srv, con);
    }

    /* default size for chunks is 4kb; only use bigger chunks if FIONREAD tells
     *  us more than 4kb is available
     * if FIONREAD doesn't signal a big chunk we fill the previous buffer
     *  if it has >= 1kb free
     */
#if defined(__WIN32)
    chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, 4096);

    len = recv(con->fd, mem, mem_len, 0);
#else /* __WIN32 */
    if (ioctl(con->fd, FIONREAD, &toread) || toread == 0 || toread <= 4*1024) {
        toread = 4096;
    }
    else if (toread > MAX_READ_LIMIT) {
        toread = MAX_READ_LIMIT;
    }
    chunkqueue_get_memory(con->read_queue, &mem, &mem_len, 0, toread);

    len = read(con->fd, mem, mem_len);
#endif /* __WIN32 */

    chunkqueue_use_memory(con->read_queue, len > 0 ? len : 0);

    if (len < 0) {
        con->is_readable = 0;

#if defined(__WIN32)
        {
            int lastError = WSAGetLastError();
            switch (lastError) {
            case EAGAIN:
                return 0;
            case EINTR:
                /* we have been interrupted before we could read */
                con->is_readable = 1;
                return 0;
            case ECONNRESET:
                /* suppress logging for this error, expected for keep-alive */
                break;
            default:
                log_error_write(srv, __FILE__, __LINE__, "sd", "connection closed - recv failed: ", lastError);
                break;
            }
        }
#else /* __WIN32 */
        switch (errno) {
        case EAGAIN:
            return 0;
        case EINTR:
            /* we have been interrupted before we could read */
            con->is_readable = 1;
            return 0;
        case ECONNRESET:
            /* suppress logging for this error, expected for keep-alive */
            break;
        default:
            log_error_write(srv, __FILE__, __LINE__, "ssd", "connection closed - read failed: ", strerror(errno), errno);
            break;
        }
#endif /* __WIN32 */

        connection_set_state(srv, con, CON_STATE_ERROR);

        return -1;
    } else if (len == 0) {
        con->is_readable = 0;
        /* the other end close the connection -> KEEP-ALIVE */

        /* pipelining */

        return -2;
    } else if (len != (ssize_t) mem_len) {
        /* we got less then expected, wait for the next fd-event */

        con->is_readable = 0;
    }

    con->bytes_read += len;
#if 0
    dump_packet(b->ptr, len);
#endif

    return 0;
}

static int connection_handle_write_prepare(server *srv, connection *con) {
    char product_name[64];
    char flag[64];    // 20160608 leo added for 401 lock
    char remain_time[64];// 20160608 leo added for 401 lock
    char appname_url[10];
    /*
     *Main_login.asp get multi language
     */
    char multi_lang_tmp[10];
    char multi_lang[15];
    int fd, len, i=0;
    char ch, tmp[256], name[256], content[256];
    memset(tmp, 0, sizeof(tmp));
    memset(name, 0, sizeof(name));
    memset(content, 0, sizeof(content));
    memset(multi_lang_tmp, 0, sizeof(multi_lang_tmp));
    memset(multi_lang, 0, sizeof(multi_lang));
    if (access("/tmp/APPS/DM2/Config/dm2_general.conf",0) == 0)
    {
         if((fd = open("/tmp/APPS/DM2/Config/dm2_general.conf", O_RDONLY | O_NONBLOCK)) > 0)
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
                      if(strcmp(name, "LANGUAGE") == 0)
                                              {
                                                  snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s",content);

                                              }
                             continue;
                        }
                                              memcpy(tmp+i, &ch, 1);
                                              i++;

         }
              close(fd);
         }else
         {
             //fprintf(stderr,"\nopen dm2_general.conf error");
         }

    }

    else {
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","EN");// for mediaserver
       // fprintf(stderr,"\n dm2_general not exist");
    }

    if(strcmp(multi_lang_tmp,"EN") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","0");
    }
        else if(strcmp(multi_lang_tmp,"TW") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","1");
    }
        else if(strcmp(multi_lang_tmp,"CN") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","2");
    }
        else if(strcmp(multi_lang_tmp,"RU") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","3");
    }
        else if(strcmp(multi_lang_tmp,"FR") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","4");
    }
        else if(strcmp(multi_lang_tmp,"DE") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","5");
    }
        else if(strcmp(multi_lang_tmp,"BR") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","6");
    }
        else if(strcmp(multi_lang_tmp,"CZ") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","7");
    }
        else if(strcmp(multi_lang_tmp,"DA") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","8");
    }
        else if(strcmp(multi_lang_tmp,"FI") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","9");
    }
        else if(strcmp(multi_lang_tmp,"MS") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","10");
    }
        else if(strcmp(multi_lang_tmp,"NO") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","11");
    }
        else if(strcmp(multi_lang_tmp,"PL") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","12");
    }
        else if(strcmp(multi_lang_tmp,"SV") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","13");
    }
        else if(strcmp(multi_lang_tmp,"TH") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","14");
    }
        else if(strcmp(multi_lang_tmp,"TR") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","15");
    }
        else if(strcmp(multi_lang_tmp,"JP") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","16");
    }
        else if(strcmp(multi_lang_tmp,"IT") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","17");
    }
        else if(strcmp(multi_lang_tmp,"HU") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","18");
    }
        else if(strcmp(multi_lang_tmp,"RO") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","19");
    }
        else if(strcmp(multi_lang_tmp,"UK") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","20");
    }
        else if(strcmp(multi_lang_tmp,"ES") == 0)
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","21");
    }
        else
    {
        memset(multi_lang_tmp,0,sizeof(multi_lang_tmp));
        snprintf(multi_lang_tmp,sizeof(multi_lang_tmp),"%s","0");
    }

         snprintf(multi_lang,sizeof(multi_lang),"\'%s\';\n",multi_lang_tmp);
    if (con->mode == DIRECT) {
        /* static files */
        switch(con->request.http_method) {
        case HTTP_METHOD_GET:
        case HTTP_METHOD_POST:
        case HTTP_METHOD_HEAD:
            break;
        case HTTP_METHOD_OPTIONS:
            /*
             * 400 is coming from the request-parser BEFORE uri.path is set
             * 403 is from the response handler when noone else catched it
             *
             * */
            if ((!con->http_status || con->http_status == 200) && !buffer_string_is_empty(con->uri.path) &&
                con->uri.path->ptr[0] != '*') {
                response_header_insert(srv, con, CONST_STR_LEN("Allow"), CONST_STR_LEN("OPTIONS, GET, HEAD, POST"));

                con->response.transfer_encoding &= ~HTTP_TRANSFER_ENCODING_CHUNKED;
                con->parsed_response &= ~HTTP_CONTENT_LENGTH;

                con->http_status = 200;
                con->file_finished = 1;

                chunkqueue_reset(con->write_queue);
            }
            break;
        default:
            if (0 == con->http_status) {
                con->http_status = 501;
            }
            break;
        }
    }

    if (con->http_status == 0) {
        con->http_status = 403;
    }

    switch(con->http_status) {
    case 204: /* class: header only */
    case 205:
    case 304:
        /* disable chunked encoding again as we have no body */
        con->response.transfer_encoding &= ~HTTP_TRANSFER_ENCODING_CHUNKED;
        con->parsed_response &= ~HTTP_CONTENT_LENGTH;
        chunkqueue_reset(con->write_queue);

        con->file_finished = 1;
        break;
    case 600:
        con->file_finished = 0;

        buffer_reset(con->physical.path);

        /* try to send static errorfile */
        if (!buffer_is_empty(con->conf.errorfile_prefix)) {
            stat_cache_entry *sce = NULL;

            buffer_copy_buffer(con->physical.path, con->conf.errorfile_prefix);
            buffer_append_int(con->physical.path, con->http_status);
            buffer_append_string_len(con->physical.path, CONST_STR_LEN(".html"));

            if (HANDLER_ERROR != stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
                con->file_finished = 1;

                http_chunk_append_file(srv, con, con->physical.path, 0, sce->st.st_size);
                response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(sce->content_type));
            }
        }
        if (!con->file_finished) {
            buffer *b;

            buffer_reset(con->physical.path);

            con->file_finished = 1;
            //b = chunkqueue_get_append_buffer(con->write_queue);
            struct in_addr login_ip_addr;
            login_ip_addr.s_addr = login_ip;
            char login_ip_str[16];
            memset(login_ip_str, 0, 16);
            strcpy(login_ip_str, inet_ntoa(login_ip_addr));
            /* build default error-page */
            // buffer_copy_string_len(b, CONST_STR_LEN(
            chunkqueue_append_mem(con->write_queue, CONST_STR_LEN(
                    "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
                    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
                    "         \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
                    " <head>\n"
                    "  <title>"));

            buffer_append_string_len(b, CONST_STR_LEN("ASUS Downloadmaster</title>\n"
                                                      "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>\n"
                                                      "<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>\n"
                                                      "<meta HTTP-EQUIV='Expires' CONTENT='-1'>\n"
                                                      "<link rel='shortcut icon' href='/downloadmaster/images/favicon.png'>\n"
                                                      "<link rel='icon' href='/downloadmaster/images/favicon.png'>\n"
                                                      "<link href='/downloadmaster/other.css'  rel='stylesheet' type='text/css'>\n"
                                                      "<style type='text/css'>\n"
                                                      "body {\n"
                                                      "background-image: url(/downloadmaster/images/bg.gif);\n"
                                                      "margin:50px auto;\n"
                                                      "}\n"
                                                      "</style>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/jquery.js'></script>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/jquery-scrolltotop.min.js'></script>\n"
                                                      "<script>\n"
                                                      "var $j = jQuery.noConflict();\n"
                                                      "var multi_INT = 0;\n"
                                                      "var url = '/downloadmaster/dm_apply.cgi';\n"
                                                      "var action_mode = 'initial';\n"
                                                      "var type = 'General';\n"
                                                      "url += '?action_mode=' + action_mode + '&download_type=' +type+ '&t=' +Math.random();\n"
                                                      "$j.ajax({url: url,\n"
                                                      "async: false,\n"
                                                      "success: function(data){initial_multi_INT_status(data)}\n"
                                                      "});\n"
                                                      "function initial_multi_INT_status(data){\n"
                                                      "var array = new Array();\n"
                                                      "eval('array='+data);\n"
                                                      "var lang = array[14];\n"
                                                      "if(lang == 'EN')\n"
                                                      "multi_INT = 0;\n"
                                                      "else if(lang == 'TW')\n"
                                                      "multi_INT = 1;\n"
                                                      "else if(lang == 'CN')\n"
                                                      "multi_INT = 2;\n"
                                                      "else if(lang == 'RU')\n"
                                                      "multi_INT = 3;\n"
                                                      "else if(lang == 'FR')\n"
                                                      "multi_INT = 4;\n"
                                                      "else if(lang == 'DE')\n"
                                                      "multi_INT = 5;\n"
                                                      "else if(lang == 'BR')\n"
                                                      "multi_INT = 6;\n"
                                                      "else if(lang == 'CZ')\n"
                                                      "multi_INT = 7;\n"
                                                      "else if(lang == 'DA')\n"
                                                      "multi_INT = 8;\n"
                                                      "else if(lang == 'FI')\n"
                                                      "multi_INT = 9;\n"
                                                      "else if(lang == 'MS')\n"
                                                      "multi_INT = 10;\n"
                                                      "else if(lang == 'NO')\n"
                                                      "multi_INT = 11;\n"
                                                      "else if(lang == 'PL')\n"
                                                      "multi_INT = 12;\n"
                                                      "else if(lang == 'SV')\n"
                                                      "multi_INT = 13;\n"
                                                      "else if(lang == 'TH')\n"
                                                      "multi_INT = 14;\n"
                                                      "else if(lang == 'TR')\n"
                                                      "multi_INT = 15;\n"
                                                      "else if(lang == 'JP')\n"
                                                      "multi_INT = 16;\n"
                                                      "else if(lang == 'IT')\n"
                                                      "multi_INT = 17;\n"
                                                      "else if(lang == 'HU')\n"
                                                      "multi_INT = 18;\n"
                                                      "else if(lang == 'RO')\n"
                                                      "multi_INT = 19;\n"
                                                      "else if(lang == 'UK')\n"
                                                      "multi_INT = 20;\n"
                                                      "else if(lang == 'ES')\n"
                                                      "multi_INT = 21;\n"
                                                      "else\n"
                                                      "multi_INT = 0;\n"
                                                      "}\n"
                                                      "</script>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/multiLanguage_all.js'></script>\n"
                                                      " </head>\n"
                                                      " <body>\n"
                                                      "  <form name='formname' method='POST'>\n"
                                                      "<table width='500' border='0' align='center' cellpadding='10' cellspacing='0' class='erTable'>\n"
                                                      "<thead>\n"
                                                      "<tr>\n"
                                                      "<td height='52' background='/downloadmaster/images/er_top.gif'></td>\n"
                                                      "</tr>\n"
                                                      "</thead>\n"
                                                      "<tr>\n"
                                                      "<th align='left' valign='top' background='/downloadmaster/images/er_bg.gif'>\n"
                                                      "<div class='drword'>\n"
                                                      "<p><span id='login_hint1'></span><span id='logined_ip_str'>\n"));
            buffer_append_string(b, login_ip_str);
            buffer_append_string_len(b, CONST_STR_LEN("</span></p>\n"
                                                      "<p><span id='login_hint2'></span></p>\n"
                                                      "</div>\n"
                                                      //"<div class='drImg'><img src='images/DrsurfImg.gif'></div>\n"
                                                      "<div style='height:70px; '></div>\n"
                                                      "</th>\n"
                                                      "</tr>\n"
                                                      "<tr>\n"
                                                      "<td height='22' background='/downloadmaster/images/er_bottom.gif'><span></span></td>\n"
                                                      "</tr>\n"
                                                      "</table>\n"
                                                      "</form>\n"
                                                      "<script>\n"
                                                      "$j('#login_hint1').html(multiLanguage_all_array[multi_INT][9]);\n"
                                                      "$j('#login_hint2').html(multiLanguage_all_array[multi_INT][10]);\n"
                                                      "</script>\n"
                                                      ));

            buffer_append_string_len(b, CONST_STR_LEN(
                    " </body>\n"
                    "</html>\n"
                    ));

            response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
        }
        break;
        case 599:
        con->file_finished = 0;

        buffer_reset(con->physical.path);

        /* try to send static errorfile */
        if (!buffer_is_empty(con->conf.errorfile_prefix)) {
            stat_cache_entry *sce = NULL;

            buffer_copy_buffer(con->physical.path, con->conf.errorfile_prefix);
            buffer_append_int(con->physical.path, con->http_status);
            buffer_append_string_len(con->physical.path, CONST_STR_LEN(".html"));

            if (HANDLER_ERROR != stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
                con->file_finished = 1;

                http_chunk_append_file(srv, con, con->physical.path, 0, sce->st.st_size);
                response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(sce->content_type));
            }
        }
        if (!con->file_finished) {
            buffer *b;

            buffer_reset(con->physical.path);

            con->file_finished = 1;
            //2016.8.31tina modify{
            b = chunkqueue_get_append_buffer(con->write_queue);
            /* build default error-page */

            //chunkqueue_append_mem(con->write_queue, CONST_STR_LEN(
            buffer_copy_string_len(b, CONST_STR_LEN(
                    //} end tina
                    "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
                    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
                    "         \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
                    " <head>\n"
                    "  <title>"));

            buffer_append_string_len(b, CONST_STR_LEN("ASUS Downloadmaster</title>\n"
                                                      "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>\n"
                                                      "<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>\n"
                                                      "<meta HTTP-EQUIV='Expires' CONTENT='-1'>\n"
                                                      "<link rel='shortcut icon' href='/downloadmaster/images/favicon.png'>\n"
                                                      "<link rel='icon' href='/downloadmaster/images/favicon.png'>\n"
                                                      "<link href='/downloadmaster/other.css'  rel='stylesheet' type='text/css'>\n"
                                                      "<style type='text/css'>\n"
                                                      "body {\n"
                                                      "background-image: url(/downloadmaster/images/bg.gif);\n"
                                                      "margin:50px auto;\n"
                                                      "}\n"
                                                      "</style>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/jquery.js'></script>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/jquery-scrolltotop.min.js'></script>\n"
                                                      "<script>\n"
                                                      "var $j = jQuery.noConflict();\n"
                                                      "var multi_INT = 0;\n"
                                                      "var url = '/downloadmaster/dm_apply.cgi';\n"
                                                      "var action_mode = 'initial';\n"
                                                      "var type = 'General';\n"
                                                      "url += '?action_mode=' + action_mode + '&download_type=' +type+ '&t=' +Math.random();\n"
                                                      "$j.ajax({url: url,\n"
                                                      "async: false,\n"
                                                      "success: function(data){initial_multi_INT_status(data)}\n"
                                                      "});\n"
                                                      "function initial_multi_INT_status(data){\n"
                                                      "var array = new Array();\n"
                                                      "eval('array='+data);\n"
                                                      "var lang = array[14];\n"
                                                      "if(lang == 'EN')\n"
                                                      "multi_INT = 0;\n"
                                                      "else if(lang == 'TW')\n"
                                                      "multi_INT = 1;\n"
                                                      "else if(lang == 'CN')\n"
                                                      "multi_INT = 2;\n"
                                                      "else if(lang == 'RU')\n"
                                                      "multi_INT = 3;\n"
                                                      "else if(lang == 'FR')\n"
                                                      "multi_INT = 4;\n"
                                                      "else if(lang == 'DE')\n"
                                                      "multi_INT = 5;\n"
                                                      "else if(lang == 'BR')\n"
                                                      "multi_INT = 6;\n"
                                                      "else if(lang == 'CZ')\n"
                                                      "multi_INT = 7;\n"
                                                      "else if(lang == 'DA')\n"
                                                      "multi_INT = 8;\n"
                                                      "else if(lang == 'FI')\n"
                                                      "multi_INT = 9;\n"
                                                      "else if(lang == 'MS')\n"
                                                      "multi_INT = 10;\n"
                                                      "else if(lang == 'NO')\n"
                                                      "multi_INT = 11;\n"
                                                      "else if(lang == 'PL')\n"
                                                      "multi_INT = 12;\n"
                                                      "else if(lang == 'SV')\n"
                                                      "multi_INT = 13;\n"
                                                      "else if(lang == 'TH')\n"
                                                      "multi_INT = 14;\n"
                                                      "else if(lang == 'TR')\n"
                                                      "multi_INT = 15;\n"
                                                      "else if(lang == 'JP')\n"
                                                      "multi_INT = 16;\n"
                                                      "else if(lang == 'IT')\n"
                                                      "multi_INT = 17;\n"
                                                      "else if(lang == 'HU')\n"
                                                      "multi_INT = 18;\n"
                                                      "else if(lang == 'RO')\n"
                                                      "multi_INT = 19;\n"
                                                      "else if(lang == 'UK')\n"
                                                      "multi_INT = 20;\n"
                                                      "else if(lang == 'ES')\n"
                                                      "multi_INT = 21;\n"
                                                      "else\n"
                                                      "multi_INT = 0;\n"
                                                      "}\n"
                                                      "</script>\n"
                                                      "<script type='text/javascript' src='/downloadmaster/multiLanguage_all.js'></script>\n"
                                                      " </head>\n"
                                                      " <body>\n"
                                                      "  <form name='formname' method='POST'>\n"
                                                      "<table width='500' border='0' align='center' cellpadding='10' cellspacing='0' class='erTable'>\n"
                                                      "<thead>\n"
                                                      "<tr>\n"
                                                      "<td height='52' background='/downloadmaster/images/er_top.gif'></td>\n"
                                                      "</tr>\n"
                                                      "</thead>\n"
                                                      "<tr>\n"
                                                      "<th align='left' valign='top' background='/downloadmaster/images/er_bg.gif'>\n"
                                                      "<div class='drword'>\n"
                                                      "<p><span id='dm_status'></span><span id='dm_status_str'>\n"
                                                      "</span></p>\n"
                                                      "</div>\n"
                                                      //"<div class='drImg'><img src='images/DrsurfImg.gif'></div>\n"
                                                      "<div style='height:70px; '>\n"
                                                      "<p><span id='dm_status'></span>\n"
                                                      "</div>\n"
                                                      "</th>\n"
                                                      "</tr>\n"
                                                      "<tr>\n"
                                                      "<td height='22' background='/downloadmaster/images/er_bottom.gif'><span></span></td>\n"
                                                      "</tr>\n"
                                                      "</table>\n"
                                                      "</form>\n"
                                                      "<script>\n"
                                                      "$j('#dm_status').html(multiLanguage_all_array[multi_INT][16]);\n"
                                                      "</script>\n"
                                                      ));

            buffer_append_string_len(b, CONST_STR_LEN(
                    " </body>\n"
                    "</html>\n"
                    ));

            response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
        }
        break;

        ///20160216 leo added for return 401 {{{

         case 401:
        memset(product_name,0,sizeof(product_name));
        sprintf(product_name,"\'%s\';\n",router_name);
        long  int lock_time=0;
        long  int last_remain_time=0;
        time_t t_r=time(NULL);
        memset(flag,0,sizeof(flag));
         memset(appname_url,0,sizeof(appname_url));
        memset(remain_time,0,sizeof(remain_time));
        buffer *b;
        buffer_reset(con->physical.path);
        if(strstr(appname,"mediaserver"))
        {
         snprintf(appname_url,7,"\'%s\';\n","MS");
        if(access("/tmp/username_pw_MS.txt",0)==0)
        {

            sprintf(flag,"\'%s\';\n","7");
            FILE *fp=fopen("/tmp/username_pw_MS.txt","r+");
            fscanf(fp,"%ld",&lock_time);
            last_remain_time=60-(t_r-lock_time);
            //last_remain_time=60;
            sprintf(remain_time,"\'%ld\';\n",last_remain_time);
            fclose(fp);
        }
        else
        {
            sprintf(flag,"\'%s\';","3");
            sprintf(remain_time,"\'%ld\';\n",last_remain_time);
        }
        }
        else
        {
            snprintf(appname_url,7,"\'%s\';\n","DM");
            if(access("/tmp/username_pw_DM.txt",0)==0)
            {

                sprintf(flag,"\'%s\';\n","7");
                FILE *fp=fopen("/tmp/username_pw_DM.txt","r+");
                fscanf(fp,"%ld",&lock_time);
                last_remain_time=60-(t_r-lock_time);
                //last_remain_time=60;
                sprintf(remain_time,"\'%ld\';\n",last_remain_time);
                fclose(fp);
            }
            else
            {
                sprintf(flag,"\'%s\';","3");
                sprintf(remain_time,"\'%ld\';\n",last_remain_time);
            }
         }
        con->file_finished = 1;
        b=chunkqueue_get_append_buffer(con->write_queue);
        /* build default error-page */
        //chunkqueue_append_mem(con->write_queue, CONST_STR_LEN(
        buffer_copy_string_len(b, CONST_STR_LEN(
                "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                "<html xmlns:v>\n"
                "<head>\n"
                "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=Edge\"/>\n"
                "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
                "<meta HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n"
                "<meta HTTP-EQUIV=\"Expires\" CONTENT=\"-1\">\n"
                "<link rel=\"shortcut icon\" href=\"images/favicon.png\">\n"
                "<link rel=\"icon\" href=\"images/favicon.png\">\n"
                "<title>"));
        buffer_append_string_len(b, CONST_STR_LEN(
                "ASUS Login</title>\n"
                "<style>\n"
                ".content{\n"
                "width:580px;\n"
                "height:526px;\n"
                "margin: 20px auto 40px auto;\n"
                "background:rgba(40,52,55,0.1);\n"
                "}\n"
                ".wrapper{\n"
                "background:url(images/New_ui/login_bg.png) #283437 no-repeat;\n"
                "background-size: 1280px 1076px;\n"
                "background-position: center 0%;\n"
                "margin: 0px;\n"
                "background:#283437\9;\n"
                "}\n"
                ".title_name {\n"
                "font-family:Arial;\n"
                "font-size: 40pt;\n"
                "color:#93d2d9;\n"
                "}\n"
                                     ".div_td{\n"
                                     "display:table-cell;\n"
                                     "}\n"
                                     ".img_gap{\n"
                                     "padding-right:30px;\n"
                                     "vertical-align:middle;\n"
                                     "}\n"
                                     ".login_img{\n"
                                     "width:43px;\n"
                                     "height:43px;\n"
                                     "background-image: url('images/New_ui/icon_titleName.png');\n"
                                     "background-repeat: no-repeat;\n"
                                     "}\n"
                                     ".nologin{\n"
                                    " margin:10px 0px 0px 78px;\n"
                                     "background-color:rgba(255,255,255,0.2);\n"
                                     "padding:20px;\n"
                                     "line-height:36px;\n"
                                     "border-radius: 5px;\n"
                                    " width: 480px;\n"
                                    " border: 0;\n"
                                     "color:#FFF;\n"
                                    " color:#FFF\9; /* IE6 IE7 IE8 */\n"
                                    " font-size:26pt;\n"
                                    " }\n"
                                    " .div_table{\n"
                                    " display:table;\n"
                                    " }\n"
                                    " .main_field_gap{\n"
                                    " margin:100px auto 0;\n"
                                    " }\n"
                                    " .nologin{\n"
                                    " margin-left:10px;\n"
                                    " padding:10px;\n"
                                    " line-height:50px;\n"
                                     "font-size:20pt;\n"
                                    " }\n"
                                     ".main_field_gap{\n"
                                    " width:80%;\n"
                                     "margin:30px 0 0 15px;\n"
                                     "}\n"
                                     ".div_tr{\n"
                                     "display:table-row;\n"
                                     "}\n"
                ".app_name {\n"
                "font-family:Arial;\n"
                "font-size: 30pt;\n"
                "color:#93d2d9;\n"
                "}\n"
                ".div_tr{\n"
                "display:table-row;\n"
                "}\n"
                ".div_td{\n"
                "display:table-cell;\n"
                "}\n"
                ".img_gap{\n"
                "padding-right:30px;\n"
                "vertical-align:middle;\n"
                "}\n"
                ".div_table{\n"
                "display:table;\n"
                "}\n"
                ".prod_madelName{\n"
                "font-family: Arial;\n"
                "font-size: 26pt;\n"
                "color:#fff;\n"
                "margin-top: 10px;\n"
                "}\n"
                ".p1{\n"
                "font-family: Arial;\n"
                "font-size: 16pt;\n"
                "color:#fff;\n"
                "}\n"
                ".error_hint{\n"
                "color: rgb(255, 204, 0);\n"
                "margin:10px 0px -10px 78px;\n"
                "font-size: 18px;\n"
                "}\n"
                ".error_hint1{\n"
                "margin:60px 0px -10px 78px;\n"
                "font-size: 24px;\n"
                "line-height:32px;\n"
                "width: 580px;\n"
                "}\n"
                ".button{\n"
                "background:rgba(255,255,255,0.1);\n"
                "border: solid 1px #6e8385;\n"
                "border-radius: 4px ;\n"
                "transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;\n"
                "height: 68px;\n"
                "width: 300px;\n"
                "font-family: Arial;\n"
                "font-size: 28pt;\n"
                "color:#fff;\n"
                "color:#000\\9; /* IE6 IE7 IE8 */\n"
                "text-align:center;\n"
                "Vertical-align:center\n"
                "}\n"
                ".button_text{\n"
                "font-family: Arial;\n"
                "font-size: 28pt;\n"
                "color:#fff;\n"
                "text-align:center;\n"
                "Vertical-align:center\n"
                "}\n"
                ".form_input{\n"
                "background-color:rgba(255,255,255,0.2);\n"
                "border-radius: 4px;\n"
                "padding:26px 22px;\n"
                "width: 480px;\n"
                "border: 0;\n"
                "height:25px;\n"
                "color:#fff;\n"
                "color:#000\\9; /* IE6 IE7 IE8 */\n"
                "font-size:28px\n"
                "}\n"
                ".form_input_text{\n"
                "font-family: Arial;\n"
                "font-size: 28pt;\n"
                "color:#a9a9a9;\n"
                "}\n"
                ".p2{\n"
                "font-family: Arial;\n"
                "font-size: 18pt;\n"
                "color:#28fff7;\n"
                "}\n"
                ));
        buffer_append_string_len(b, CONST_STR_LEN(
                "</style>\n"
                "<script type=\"text/javascript\" src=\"jquery.js?ver=104\"></script>\n"
                                     "<script type=\"text/javascript\" src=\"multiLanguage_all.js?ver=104\"></script>\n"
                "<script>\n"
                "var $j=jQuery.noConflict();\n"
                "var remaining_time="
                ));

        buffer_append_string_len(b, remain_time,strlen(remain_time));
        buffer_append_string_len(b, CONST_STR_LEN(
                "var appname="
                ));
        buffer_append_string_len(b, appname_url,strlen(appname_url));
        buffer_append_string_len(b, CONST_STR_LEN(
                "var multi_INT="
                ));
        buffer_append_string_len(b, multi_lang,strlen(multi_lang));
        buffer_append_string_len(b, CONST_STR_LEN(
                "var countdownid, rtime_obj;\n"
                "var dir_url=decodeURIComponent(location.search)\n"
                "if( dir_url == \"\"){\n"
                "if( appname == \"DM\"){\n"
                "var directurl1_l=decodeURIComponent(\"?flag=&foilautofill=&directurl=%2Fdownloadmaster%2Ftask.asp&login_username=&login_passwd=\").split(\"url=\")[1]\n"
                "var directurl_l = directurl1_l.split(\"&\")[0]\n"
                "var directurl1=decodeURIComponent(\"?flag=&foilautofill=&directurl=%2Fdownloadmaster%2Ftask.asp&login_username=&login_passwd=\").split(\"url=\")[1]\n"
                "var directurl = directurl1.split(\"&\")[0]\n"
                "}\n"
                "else {\n"
                "var directurl1_l=decodeURIComponent(\"?flag=&foilautofill=&directurl=%2Fmediaserverui%2Fmediaserver.asp&login_username=&login_passwd=\").split(\"url=\")[1]\n"
                "var directurl_l = directurl1_l.split(\"&\")[0]\n"
                "var directurl1=decodeURIComponent(\"?flag=&foilautofill=&directurl=%2Fmediaserverui%2Fmediaserver.asp&login_username=&login_passwd=\").split(\"url=\")[1]\n"
                "var directurl = directurl1.split(\"&\")[0]\n"
                "}\n"
                "}\n"
                "else {\n"
                "var directurl1_l=decodeURIComponent(location.search).split(\"url=\")[1]\n"
                "var directurl_l = directurl1_l.split(\"&\")[0]\n"
                "var directurl1=decodeURIComponent(location.search).split(\"url=\")[1]\n"
                "var directurl = directurl1.split(\"&\")[0]\n"
                "}\n"
                "var flag="
                ));

        buffer_append_string_len(b, flag,strlen(flag));
        buffer_append_string_len(b, CONST_STR_LEN(
                "var product_name="
                ));
        buffer_append_string_len(b, product_name,strlen(product_name));
        buffer_append_string_len(b, CONST_STR_LEN(
                "function initial(){\n"
                "document.form.login_usern.focus();\n"
                "showAPPname();\n"
                "if(flag != \"\" && flag != \"1\"){\n"
                "document.getElementById(\"error_status_field\").style.display =\"\";\n"
                "if(flag == 3){\n"
                "document.getElementById(\"error_status_field\").innerHTML =\"* Invalid username or password\";\n"
                "document.getElementById(\"error_status_field\").className = \"error_hint\";\n"
                "}\n"
                "else if(flag == 7){\n"
                "document.getElementById(\"error_status_field\").innerHTML =\"You have entered an incorrect username or password 5 times. Please try again after \"+\"<span id='rtime'></span>\"+\" seconds.\";\n"
                "document.getElementById(\"error_status_field\").className = \"error_hint error_hint1\";\n"
                "document.getElementsByClassName('form_input')[0].style.display = \"none\";\n"
                "document.getElementsByClassName('form_input')[1].style.display = \"none\";\n"
                "disable_button(1);\n"
                "rtime_obj=document.getElementById(\"rtime\");\n"
                "rtime_obj.innerHTML=remaining_time;\n"
                "countdownid = window.setInterval(countdownfunc,1000);\n"
    "}\n"
    "else if(flag == 8){\n"
    "document.getElementById(\"login_filed\").style.display =\"none\";\n"
    "document.getElementById(\"logout_field\").style.display =\"\";\n"
    "}else\n"
                "document.getElementById(\"error_status_field\").style.display =\"none\";\n"
                "}\n"
                "}\n"
                "function trim(val){\n"
                "val = val+'';\n"
                "for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);\n"
                "for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);\n"
                "return val.substring(startIndex,endIndex+1);\n"
                "}\n"
                "function countdownfunc(){\n"
                "rtime_obj.innerHTML=remaining_time;\n"
                "if (remaining_time==0){\n"
                "clearInterval(countdownid);\n"
                "setTimeout(\"top.location.href='/Main_Login.asp'+'?flag=1'+'&productname='+product_name+'&url='+directurl_l;\", 2000);\n"
                "}\n"
                "remaining_time--;\n"
                "}\n"
                "function disable_input(val){\n"
                "var disable_input_x = document.getElementsByClassName('form_input');\n"
                "for(i=0;i<disable_input_x.length;i++){\n"
                "if(val == 0)\n"
                "disable_input_x[i].disabled = true;\n"
                "else\n"
                "disable_input_x[i].style.display = \"none\";\n"
                "}\n"
                "}\n"
                "function disable_button(val){\n"
                "if(val == 0)\n"
                "document.getElementsByClassName('button')[0].disabled = true;\n"
                "else\n"
                "document.getElementsByClassName('button')[0].style.display = \"none\";\n"
                "}\n"
                "function login(){\n"
                "if(!window.btoa){\n"
                "window.btoa = function(input){\n"
                "var keyStr = \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\";\n"
                "var output = \"\";\n"
                "var chr1, chr2, chr3, enc1, enc2, enc3, enc4;\n"
                "var i = 0;\n"
                "var utf8_encode = function(string) {\n"
                "string = string.replace(/\\r\\n/g,\"\\n\");\n"
                "var utftext = \"\";\n"
                "for (var n = 0; n < string.length; n++) {\n"
                "var c = string.charCodeAt(n);\n"
                "if (c < 128) {\n"
                "utftext += String.fromCharCode(c);\n"
                "}\n"
                "else if((c > 127) && (c < 2048)) {\n"
                "utftext += String.fromCharCode((c >> 6) | 192);\n"
                "utftext += String.fromCharCode((c & 63) | 128);\n"
                "}\n"
                "else {\n"
                "utftext += String.fromCharCode((c >> 12) | 224);\n"
                "utftext += String.fromCharCode(((c >> 6) & 63) | 128);\n"
                "utftext += String.fromCharCode((c & 63) | 128);\n"
                "}\n"
                "}\n"
                "return utftext;\n"
                "};\n"
                "input = utf8_encode(input);\n"
                "while (i < input.length) {\n"
                "chr1 = input.charCodeAt(i++);\n"
                "chr2 = input.charCodeAt(i++);\n"
                "chr3 = input.charCodeAt(i++);\n"
                "enc1 = chr1 >> 2;\n"
                "enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);\n"
                "enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);\n"
                "enc4 = chr3 & 63;\n"
                "if (isNaN(chr2)) {\n"
                "enc3 = enc4 = 64;\n"
                "}\n"
                "else if (isNaN(chr3)) {\n"
                "enc4 = 64;\n"
                "}\n"
                "output = output +\n"
                "keyStr.charAt(enc1) + keyStr.charAt(enc2) +\n"
                "keyStr.charAt(enc3) + keyStr.charAt(enc4);\n"
                "}\n"
                "return output;\n"
                "};\n"
                "}\n"
                "document.form.login_usern.value = trim(document.form.login_usern.value);\n"
                "document.form.login_username.value = btoa(document.form.login_usern.value);\n"
                "document.form.login_passwd.value = btoa(document.form.login_passw.value);\n"
                "document.form.login_usern.disabled = true;\n"
                "document.form.login_passw.disabled = true;\n"
                "document.form.submit();\n"
                "}\n"
                "function showAPPname() {\n"
                "if(directurl.indexOf(\"downloadmaster\")>0) {\n"
                "document.getElementById(\"app_name\").innerHTML =\"Download Master\";\n"
                " } else if (directurl.indexOf(\"mediaserverui\")>0) {\n"
                "document.getElementById(\"app_name\").innerHTML =\"Media Server\";"
                "\t}\n"
                "}\n"
                 ));

        buffer_append_string_len(b, CONST_STR_LEN(
                "</script>\n"
                "</head>\n"
                "<body class=\"wrapper\" onload=\"initial();\">\n"
                "<iframe name=\"hidden_frame\" id=\"hidden_frame\" width=\"0\" height=\"0\" frameborder=\"0\"></iframe>\n"
                "<form method=\"post\" name=\"form\" action=\"/check.asp\" >\n"
                "<input type=\"hidden\" name=\"flag\" value=\"\">\n"
                "<input type=\"hidden\" name=\"login_username\" value=\"\">\n"
                "<input type=\"hidden\" name=\"login_passwd\" value=\"\">\n"
                "<input name=\"foilautofill\" style=\"display: none;\" type=\"password\">\n"
        "<script>\n"
                "document.write('<input type=\"hidden\" name=\"directurl\" value='+directurl+'>');\n"
        "</script>\n"
                                     "<div class=\"div_table main_field_gap\">\n"
                                     "<div class=\"div_tr\">\n"
                                     "<div class=\"title_name\" style=\"margin-left:322px;margin-top:60px;\">\n"
                                     "<div class=\"div_td img_gap\">\n"
                                    " <div class=\"login_img\"></div>\n"
                                    " </div>\n"
                                     "<div class=\"div_td\">SIGN IN</div>\n"
                                     "</div>\n"
                                     "<div class=\"app_name\" style=\"margin-left:400px;margin-top:10px;\" id=\"app_name\"></div>\n"
                                    "  <div class=\"prod_madelName\" style=\"margin-left:400px;margin-top:10px;\"><span id=\"multi_1\"></span></div>\n"
                                     "<div id=\"login_filed\">\n"
                                    " <div class=\"p1\" style=\"margin-left:400px;margin-top:10px;\"><span id=\"multi_sign\" >Sign in with your ASUS router account</span></div>\n"
                                     "<div style=\"margin-left:400px;margin-top:20px;\">\n"
                                    " <input type=\"text\" id=\"login_usern\" name=\"login_usern\" tabindex=\"1\" class=\"form_input\" maxlength=\"20\" value=\"\" autocapitalization=\"off\" autocomplete=\"off\" placeholder=\"Username\">\n"
                                    " </div>\n"
                                    " <div style=\"margin-left:400px;margin-top:30px;\">\n"
                                   "  <input type=\"password\" id=\"login_passw\" autocapitalization=\"off\" autocomplete=\"off\" value=\"\" name=\"login_passw\" tabindex=\"2\" class=\"form_input\" maxlength=\"16\" onkeyup=\"\" onpaste=\"return false;\"/ onBlur=\"\" placeholder=\"Password\">\n"
                                    " </div>\n"
                                     "<div style=\"color: rgb(255, 204, 0); margin-left:400px;margin-top:10px; display:none;\" id=\"error_status_field\" class=\"formfontdesc\"></div>\n"
                                    " <div>\n"
                                    " <!--<input type=\"submit\" class=\"button\" onclick=\"login();\" value=\"<#76#>\">-->\n"
                                    " <input type=\"submit\" id=\"button_sign\" onclick=\"login();\" class=\"button\" style=\"margin-left:622px;margin-top:40px;\" value=\"Sign in\">\n"
                                    " </div>\n"
                                    " </div>\n"
                                    " <div id=\"logout_field\" style=\"display:none;\">\n"
                                    " <div class=\"nologin\"  style=\"margin-left:438px;margin-top:30px;\" align=\"left\"><span id=\"logout_id\"></span></div>\n"
                                    " </div>\n"
                                     "</div>\n"
                                    " </div>\n"
                                    " </form>\n"
                                    " <script>\n"
                                    " $j(\"#multi_1\").html(product_name);\n"
                                    " document.getElementById(\"login_usern\").setAttribute(\"placeholder\",multiLanguage_all_array[multi_INT][18]);\n"
                                    " document.getElementById(\"login_passw\").setAttribute(\"placeholder\",multiLanguage_all_array[multi_INT][19]);\n"
                                    " $j(\"#multi_sign\").html(multiLanguage_all_array[multi_INT][20]);\n"
                                    " document.getElementById(\"button_sign\").setAttribute(\"value\",multiLanguage_all_array[multi_INT][21]);\n"
                                    "$j(\"#logout_id\").html(multiLanguage_all_array[multi_INT][8]);\n"
                "</script>\n"
                "</body>\n"
                "</html>\n"
                ));
        response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
        break;

        ///20160216 leo added for return 401 }}}
        //20160428 sherry added for cgi redirect{
    case 598:

        break;
        //20160428 sherry added for cgi redirect }
    default: /* class: header + body */
            if (con->mode != DIRECT) break;

            /* only custom body for 4xx and 5xx */
            if (con->http_status < 400 || con->http_status >= 600) break;

            con->file_finished = 0;

            buffer_reset(con->physical.path);

            /* try to send static errorfile */
            if (!buffer_string_is_empty(con->conf.errorfile_prefix)) {
                stat_cache_entry *sce = NULL;

                buffer_copy_buffer(con->physical.path, con->conf.errorfile_prefix);
                buffer_append_int(con->physical.path, con->http_status);
                buffer_append_string_len(con->physical.path, CONST_STR_LEN(".html"));

                if (HANDLER_ERROR != stat_cache_get_entry(srv, con, con->physical.path, &sce)) {
                    con->file_finished = 1;

                    http_chunk_append_file(srv, con, con->physical.path, 0, sce->st.st_size);
                    response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(sce->content_type));
                }
            }

            if (!con->file_finished) {
                buffer *b;

                buffer_reset(con->physical.path);

                con->file_finished = 1;
                b = buffer_init();

                /* build default error-page */
                buffer_copy_string_len(b, CONST_STR_LEN(
                        "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
                        "         \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
                        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
                        " <head>\n"
                        "  <title>"));
                buffer_append_int(b, con->http_status);
                buffer_append_string_len(b, CONST_STR_LEN(" - "));
                buffer_append_string(b, get_http_status_name(con->http_status));

                buffer_append_string_len(b, CONST_STR_LEN(
                        "</title>\n"
                        " </head>\n"
                        " <body>\n"
                        "  <h1>"));
                buffer_append_int(b, con->http_status);
                buffer_append_string_len(b, CONST_STR_LEN(" - "));
                buffer_append_string(b, get_http_status_name(con->http_status));

                buffer_append_string_len(b, CONST_STR_LEN("</h1>\n"
                                                          " </body>\n"
                                                          "</html>\n"
                                                          ));

                http_chunk_append_buffer(srv, con, b);
                buffer_free(b);
                http_chunk_close(srv, con);

                response_header_overwrite(srv, con, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
            }
            break;
    }

    if (con->file_finished) {
        /* we have all the content and chunked encoding is not used, set a content-length */

        if ((!(con->parsed_response & HTTP_CONTENT_LENGTH)) &&
            (con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) == 0) {
            off_t qlen = chunkqueue_length(con->write_queue);

            /**
             * The Content-Length header only can be sent if we have content:
             * - HEAD doesn't have a content-body (but have a content-length)
             * - 1xx, 204 and 304 don't have a content-body (RFC 2616 Section 4.3)
             *
             * Otherwise generate a Content-Length header as chunked encoding is not
             * available
             */
            if ((con->http_status >= 100 && con->http_status < 200) ||
                con->http_status == 204 ||
                con->http_status == 304) {
                data_string *ds;
                /* no Content-Body, no Content-Length */
                if (NULL != (ds = (data_string*) array_get_element(con->response.headers, "Content-Length"))) {
                    buffer_reset(ds->value); /* Headers with empty values are ignored for output */
                }
            } else if (qlen > 0 || con->request.http_method != HTTP_METHOD_HEAD) {
                /* qlen = 0 is important for Redirects (301, ...) as they MAY have
                 * a content. Browsers are waiting for a Content otherwise
                 */
                buffer_copy_int(srv->tmp_buf, qlen);

                response_header_overwrite(srv, con, CONST_STR_LEN("Content-Length"), CONST_BUF_LEN(srv->tmp_buf));
            }
        }
    } else {
        /**
         * the file isn't finished yet, but we have all headers
         *
         * to get keep-alive we either need:
         * - Content-Length: ... (HTTP/1.0 and HTTP/1.0) or
         * - Transfer-Encoding: chunked (HTTP/1.1)
         */

        if (((con->parsed_response & HTTP_CONTENT_LENGTH) == 0) &&
            ((con->response.transfer_encoding & HTTP_TRANSFER_ENCODING_CHUNKED) == 0)) {
            con->keep_alive = 0;
        }

        /**
         * if the backend sent a Connection: close, follow the wish
         *
         * NOTE: if the backend sent Connection: Keep-Alive, but no Content-Length, we
         * will close the connection. That's fine. We can always decide the close
         * the connection
         *
         * FIXME: to be nice we should remove the Connection: ...
         */
        if (con->parsed_response & HTTP_CONNECTION) {
            /* a subrequest disable keep-alive although the client wanted it */
            if (con->keep_alive && !con->response.keep_alive) {
                con->keep_alive = 0;
            }
        }
    }

    if (con->request.http_method == HTTP_METHOD_HEAD) {
        /**
         * a HEAD request has the same as a GET
         * without the content
         */
        con->file_finished = 1;

        chunkqueue_reset(con->write_queue);
        con->response.transfer_encoding &= ~HTTP_TRANSFER_ENCODING_CHUNKED;
    }

    http_response_write_header(srv, con);

    return 0;
}

static int connection_handle_write(server *srv, connection *con) {
    switch(network_write_chunkqueue(srv, con, con->write_queue, MAX_WRITE_LIMIT)) {
    case 0:
        con->write_request_ts = srv->cur_ts;
        if (con->file_finished) {
            connection_set_state(srv, con, CON_STATE_RESPONSE_END);
            joblist_append(srv, con);
        }
        break;
    case -1: /* error on our side */
            log_error_write(srv, __FILE__, __LINE__, "sd",
                            "connection closed: write failed on fd", con->fd);
            connection_set_state(srv, con, CON_STATE_ERROR);
            joblist_append(srv, con);
            break;
    case -2: /* remote close */
            connection_set_state(srv, con, CON_STATE_ERROR);
            joblist_append(srv, con);
            break;
    case 1:
            con->write_request_ts = srv->cur_ts;
            con->is_writable = 0;

            /* not finished yet -> WRITE */
            break;
    }

    return 0;
}



connection *connection_init(server *srv) {
    connection *con;

    UNUSED(srv);

    con = calloc(1, sizeof(*con));

    con->fd = 0;
    con->ndx = -1;
    con->fde_ndx = -1;
    con->bytes_written = 0;
    con->bytes_read = 0;
    con->bytes_header = 0;
    con->loops_per_request = 0;

#define CLEAN(x) \
    con->x = buffer_init();

    CLEAN(request.uri);
    CLEAN(request.request_line);
    CLEAN(request.request);
    CLEAN(request.pathinfo);

    CLEAN(request.orig_uri);

    CLEAN(uri.scheme);
    CLEAN(uri.authority);
    CLEAN(uri.path);
    CLEAN(uri.path_raw);
    CLEAN(uri.query);

    CLEAN(physical.doc_root);
    CLEAN(physical.path);
    CLEAN(physical.basedir);
    CLEAN(physical.rel_path);
    CLEAN(physical.etag);
    CLEAN(parse_request);

    CLEAN(server_name);
    CLEAN(error_handler);
    CLEAN(dst_addr_buf);
#if defined USE_OPENSSL && ! defined OPENSSL_NO_TLSEXT
    CLEAN(tlsext_server_name);
#endif

#undef CLEAN
    con->write_queue = chunkqueue_init();
    con->read_queue = chunkqueue_init();
    con->request_content_queue = chunkqueue_init();
    chunkqueue_set_tempdirs(
            con->request_content_queue,
            srv->srvconf.upload_tempdirs,
            srv->srvconf.upload_temp_file_size);

    con->request.headers      = array_init();
    con->response.headers     = array_init();
    con->environment     = array_init();

    /* init plugin specific connection structures */

    con->plugin_ctx = calloc(1, (srv->plugins.used + 1) * sizeof(void *));

    con->cond_cache = calloc(srv->config_context->used, sizeof(cond_cache_t));
    config_setup_connection(srv, con);

    return con;
}

void connections_free(server *srv) {
    connections *conns = srv->conns;
    size_t i;

    for (i = 0; i < conns->size; i++) {
        connection *con = conns->ptr[i];

        connection_reset(srv, con);

        chunkqueue_free(con->write_queue);
        chunkqueue_free(con->read_queue);
        chunkqueue_free(con->request_content_queue);
        array_free(con->request.headers);
        array_free(con->response.headers);
        array_free(con->environment);

#define CLEAN(x) \
    buffer_free(con->x);

        CLEAN(request.uri);
        CLEAN(request.request_line);
        CLEAN(request.request);
        CLEAN(request.pathinfo);

        CLEAN(request.orig_uri);

        CLEAN(uri.scheme);
        CLEAN(uri.authority);
        CLEAN(uri.path);
        CLEAN(uri.path_raw);
        CLEAN(uri.query);

        CLEAN(physical.doc_root);
        CLEAN(physical.path);
        CLEAN(physical.basedir);
        CLEAN(physical.etag);
        CLEAN(physical.rel_path);
        CLEAN(parse_request);

        CLEAN(server_name);
        CLEAN(error_handler);
        CLEAN(dst_addr_buf);
#if defined USE_OPENSSL && ! defined OPENSSL_NO_TLSEXT
        CLEAN(tlsext_server_name);
#endif
#undef CLEAN
        free(con->plugin_ctx);
        free(con->cond_cache);

        free(con);
    }

    free(conns->ptr);
}


int connection_reset(server *srv, connection *con) {
    size_t i;

    plugins_call_connection_reset(srv, con);

    con->is_readable = 1;
    con->is_writable = 1;
    con->http_status = 0;
    con->file_finished = 0;
    con->file_started = 0;
    con->got_response = 0;

    con->parsed_response = 0;

    con->bytes_written = 0;
    con->bytes_written_cur_second = 0;
    con->bytes_read = 0;
    con->bytes_header = 0;
    con->loops_per_request = 0;

    con->request.http_method = HTTP_METHOD_UNSET;
    con->request.http_version = HTTP_VERSION_UNSET;

    con->request.http_if_modified_since = NULL;
    con->request.http_if_none_match = NULL;

    con->response.keep_alive = 0;
    con->response.content_length = -1;
    con->response.transfer_encoding = 0;

    con->mode = DIRECT;

#define CLEAN(x) \
    if (con->x) buffer_reset(con->x);

    CLEAN(request.uri);
    CLEAN(request.request_line);
    CLEAN(request.pathinfo);
    CLEAN(request.request);

    /* CLEAN(request.orig_uri); */

    CLEAN(uri.scheme);
    /* CLEAN(uri.authority); */
    /* CLEAN(uri.path); */
    CLEAN(uri.path_raw);
    /* CLEAN(uri.query); */

    CLEAN(physical.doc_root);
    CLEAN(physical.path);
    CLEAN(physical.basedir);
    CLEAN(physical.rel_path);
    CLEAN(physical.etag);

    CLEAN(parse_request);

    CLEAN(server_name);
    CLEAN(error_handler);
#if defined USE_OPENSSL && ! defined OPENSSL_NO_TLSEXT
    CLEAN(tlsext_server_name);
#endif
#undef CLEAN

#define CLEAN(x) \
    if (con->x) con->x->used = 0;

#undef CLEAN

#define CLEAN(x) \
    con->request.x = NULL;

    CLEAN(http_host);
    CLEAN(http_range);
    CLEAN(http_content_type);
#undef CLEAN
    con->request.content_length = 0;

    array_reset(con->request.headers);
    array_reset(con->response.headers);
    array_reset(con->environment);

    chunkqueue_reset(con->write_queue);
    chunkqueue_reset(con->request_content_queue);

    /* the plugins should cleanup themself */
    for (i = 0; i < srv->plugins.used; i++) {
        plugin *p = ((plugin **)(srv->plugins.ptr))[i];
        plugin_data *pd = p->data;

        if (!pd) continue;

        if (con->plugin_ctx[pd->id] != NULL) {
            log_error_write(srv, __FILE__, __LINE__, "sb", "missing cleanup in", p->name);
        }

        con->plugin_ctx[pd->id] = NULL;
    }

    /* The cond_cache gets reset in response.c */
    /* config_cond_cache_reset(srv, con); */

    con->header_len = 0;
    con->in_error_handler = 0;

    config_setup_connection(srv, con);

    return 0;
}

/**
 * handle all header and content read
 *
 * we get called by the state-engine and by the fdevent-handler
 */
static int connection_handle_read_state(server *srv, connection *con)  {
    connection_state_t ostate = con->state;
    chunk *c, *last_chunk;
    off_t last_offset;
    chunkqueue *cq = con->read_queue;
    chunkqueue *dst_cq = con->request_content_queue;
    int is_closed = 0; /* the connection got closed, if we don't have a complete header, -> error */

    if (con->is_readable) {
        con->read_idle_ts = srv->cur_ts;

        switch(connection_handle_read(srv, con)) {
        case -1:
            return -1;
        case -2:
            is_closed = 1;
            break;
        default:
            break;
        }
    }

    chunkqueue_remove_finished_chunks(cq);

    /* we might have got several packets at once
     */

    switch(ostate) {
    case CON_STATE_READ:
        /* if there is a \r\n\r\n in the chunkqueue
         *
         * scan the chunk-queue twice
         * 1. to find the \r\n\r\n
         * 2. to copy the header-packet
         *
         */

        last_chunk = NULL;
        last_offset = 0;

        for (c = cq->first; c; c = c->next) {
            size_t i;
            size_t len = buffer_string_length(c->mem) - c->offset;
            const char *b = c->mem->ptr + c->offset;

            for (i = 0; i < len; ++i) {
                char ch = b[i];

                if ('\r' == ch) {
                    /* chec if \n\r\n follows */
                    size_t j = i+1;
                    chunk *cc = c;
                    const char header_end[] = "\r\n\r\n";
                    int header_end_match_pos = 1;

                    for ( ; cc; cc = cc->next, j = 0 ) {
                        size_t bblen = buffer_string_length(cc->mem) - cc->offset;
                        const char *bb = cc->mem->ptr + cc->offset;

                        for ( ; j < bblen; j++) {
                            ch = bb[j];

                            if (ch == header_end[header_end_match_pos]) {
                                header_end_match_pos++;
                                if (4 == header_end_match_pos) {
                                    last_chunk = cc;
                                    last_offset = j+1;
                                    goto found_header_end;
                                }
                            } else {
                                goto reset_search;
                            }
                        }
                    }
                }
                reset_search: ;
                }
        }
        found_header_end:

        /* found */
        if (last_chunk) {
            buffer_reset(con->request.request);

            for (c = cq->first; c; c = c->next) {
                size_t len = buffer_string_length(c->mem) - c->offset;

                if (c == last_chunk) {
                    len = last_offset;
                }

                buffer_append_string_len(con->request.request, c->mem->ptr + c->offset, len);
                c->offset += len;
                cq->bytes_out += len;

                if (c == last_chunk) break;
            }

            connection_set_state(srv, con, CON_STATE_REQUEST_END);
        } else if (chunkqueue_length(cq) > 64 * 1024) {
            log_error_write(srv, __FILE__, __LINE__, "s", "oversized request-header -> sending Status 414");

            con->http_status = 414; /* Request-URI too large */
            con->keep_alive = 0;
            connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);
        }
        break;
    case CON_STATE_READ_POST:
        if (con->request.content_length <= 64*1024) {
            /* don't buffer request bodies <= 64k on disk */
            chunkqueue_steal(dst_cq, cq, con->request.content_length - dst_cq->bytes_in);
        }
        else if (0 != chunkqueue_steal_with_tempfiles(srv, dst_cq, cq, con->request.content_length - dst_cq->bytes_in )) {
            con->http_status = 413; /* Request-Entity too large */
            con->keep_alive = 0;
            connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);
        }

        /* Content is ready */
        if (dst_cq->bytes_in == (off_t)con->request.content_length) {
            connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);
        }

        break;
    default: break;
    }

    /* the connection got closed and we didn't got enough data to leave one of the READ states
     * the only way is to leave here */
    if (is_closed && ostate == con->state) {
        connection_set_state(srv, con, CON_STATE_ERROR);
    }

    chunkqueue_remove_finished_chunks(cq);

    return 0;
}

static handler_t connection_handle_fdevent(server *srv, void *context, int revents) {
    connection *con = context;

    joblist_append(srv, con);

    if (con->srv_socket->is_ssl) {
        /* ssl may read and write for both reads and writes */
        if (revents & (FDEVENT_IN | FDEVENT_OUT)) {
            con->is_readable = 1;
            con->is_writable = 1;
        }
    } else {
        if (revents & FDEVENT_IN) {
            con->is_readable = 1;
        }
        if (revents & FDEVENT_OUT) {
            con->is_writable = 1;
            /* we don't need the event twice */
        }
    }


    if (revents & ~(FDEVENT_IN | FDEVENT_OUT)) {
        /* looks like an error */

        /* FIXME: revents = 0x19 still means that we should read from the queue */
        if (revents & FDEVENT_HUP) {
            if (con->state == CON_STATE_CLOSE) {
                con->close_timeout_ts = srv->cur_ts - (HTTP_LINGER_TIMEOUT+1);
            } else {
                /* sigio reports the wrong event here
                 *
                 * there was no HUP at all
                 */
#ifdef USE_LINUX_SIGIO
                if (srv->ev->in_sigio == 1) {
                    log_error_write(srv, __FILE__, __LINE__, "sd",
                                    "connection closed: poll() -> HUP", con->fd);
                } else {
                    connection_set_state(srv, con, CON_STATE_ERROR);
                }
#else
                connection_set_state(srv, con, CON_STATE_ERROR);
#endif

            }
        } else if (revents & FDEVENT_ERR) {
            /* error, connection reset, whatever... we don't want to spam the logfile */
#if 0
            log_error_write(srv, __FILE__, __LINE__, "sd",
                            "connection closed: poll() -> ERR", con->fd);
#endif
            connection_set_state(srv, con, CON_STATE_ERROR);
        } else {
            log_error_write(srv, __FILE__, __LINE__, "sd",
                            "connection closed: poll() -> ???", revents);
        }
    }

    if (con->state == CON_STATE_READ ||
        con->state == CON_STATE_READ_POST) {
        connection_handle_read_state(srv, con);
    }

    if (con->state == CON_STATE_WRITE &&
        !chunkqueue_is_empty(con->write_queue) &&
        con->is_writable) {

        if (-1 == connection_handle_write(srv, con)) {
            connection_set_state(srv, con, CON_STATE_ERROR);

            log_error_write(srv, __FILE__, __LINE__, "ds",
                            con->fd,
                            "handle write failed.");
        }
    }

    if (con->state == CON_STATE_CLOSE) {
        /* flush the read buffers */
        int len;
        char buf[1024];

        len = read(con->fd, buf, sizeof(buf));
        if (len == 0 || (len < 0 && errno != EAGAIN && errno != EINTR) ) {
            con->close_timeout_ts = srv->cur_ts - (HTTP_LINGER_TIMEOUT+1);
        }
    }

    return HANDLER_FINISHED;
}


connection *connection_accept(server *srv, server_socket *srv_socket) {
    /* accept everything */

    /* search an empty place */
    int cnt;
    sock_addr cnt_addr;
    socklen_t cnt_len;
    /* accept it and register the fd */

    /**
     * check if we can still open a new connections
     *
     * see #1216
     */

    if (srv->conns->used >= srv->max_conns) {
        return NULL;
    }

    cnt_len = sizeof(cnt_addr);

    if (-1 == (cnt = accept(srv_socket->fd, (struct sockaddr *) &cnt_addr, &cnt_len))) {
        switch (errno) {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        case EINTR:
            /* we were stopped _before_ we had a connection */
        case ECONNABORTED: /* this is a FreeBSD thingy */
            /* we were stopped _after_ we had a connection */
            break;
        case EMFILE:
            /* out of fds */
            break;
        default:
            log_error_write(srv, __FILE__, __LINE__, "ssd", "accept failed:", strerror(errno), errno);
        }
        return NULL;
    } else {
        connection *con;

        srv->cur_fds++;

        /* ok, we have the connection, register it */
#if 0
        log_error_write(srv, __FILE__, __LINE__, "sd",
                        "appected()", cnt);
#endif
        srv->con_opened++;

        con = connections_get_new_connection(srv);

        con->fd = cnt;
        con->fde_ndx = -1;
#if 0
        gettimeofday(&(con->start_tv), NULL);
#endif
        fdevent_register(srv->ev, con->fd, connection_handle_fdevent, con);

        connection_set_state(srv, con, CON_STATE_REQUEST_START);

        con->connection_start = srv->cur_ts;
        con->dst_addr = cnt_addr;
        buffer_copy_string(con->dst_addr_buf, inet_ntop_cache_get_ip(srv, &(con->dst_addr)));
        con->srv_socket = srv_socket;

        if (-1 == (fdevent_fcntl_set(srv->ev, con->fd))) {
            log_error_write(srv, __FILE__, __LINE__, "ss", "fcntl failed: ", strerror(errno));
            return NULL;
        }
#ifdef USE_OPENSSL
        /* connect FD to SSL */
        if (srv_socket->is_ssl) {
            if (NULL == (con->ssl = SSL_new(srv_socket->ssl_ctx))) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "SSL:",
                                ERR_error_string(ERR_get_error(), NULL));

                return NULL;
            }

            con->renegotiations = 0;
            SSL_set_app_data(con->ssl, con);
            SSL_set_accept_state(con->ssl);

            if (1 != (SSL_set_fd(con->ssl, cnt))) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "SSL:",
                                ERR_error_string(ERR_get_error(), NULL));
                return NULL;
            }
        }
#endif
        return con;
    }
}


int connection_state_machine(server *srv, connection *con) {
    int done = 0, r;
#ifdef USE_OPENSSL
    server_socket *srv_sock = con->srv_socket;
#endif

    if (srv->srvconf.log_state_handling) {
        log_error_write(srv, __FILE__, __LINE__, "sds",
                        "state at start",
                        con->fd,
                        connection_get_state(con->state));
    }

    while (done == 0) {
        size_t ostate = con->state;

        switch (con->state) {
        case CON_STATE_REQUEST_START: /* transient */
            if (srv->srvconf.log_state_handling) {
                log_error_write(srv, __FILE__, __LINE__, "sds",
                                "state for fd", con->fd, connection_get_state(con->state));
            }

            con->request_start = srv->cur_ts;
            con->read_idle_ts = srv->cur_ts;

            con->request_count++;
            con->loops_per_request = 0;

            connection_set_state(srv, con, CON_STATE_READ);

            break;
        case CON_STATE_REQUEST_END: /* transient */
                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    buffer_reset(con->uri.authority);
                    buffer_reset(con->uri.path);
                    buffer_reset(con->uri.query);
                    buffer_reset(con->request.orig_uri);

                    if (http_request_parse(srv, con)) {
                        /* we have to read some data from the POST request */

                        connection_set_state(srv, con, CON_STATE_READ_POST);

                        break;
                    }

                    connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);

                    break;
        case CON_STATE_HANDLE_REQUEST:
                    /*
             * the request is parsed
             *
             * decided what to do with the request
             * -
             *
             *
             */

                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    switch (r = http_response_prepare(srv, con)) {
                    case HANDLER_FINISHED:
                        if (con->mode == DIRECT) {
                            if (con->http_status == 404 ||
                                con->http_status == 403) {
                                /* 404 error-handler */

                                if (con->in_error_handler == 0 &&
                                    (!buffer_string_is_empty(con->conf.error_handler) ||
                                     !buffer_string_is_empty(con->error_handler))) {
                                    /* call error-handler */

                                    con->error_handler_saved_status = con->http_status;
                                    con->http_status = 0;

                                    if (buffer_string_is_empty(con->error_handler)) {
                                        buffer_copy_buffer(con->request.uri, con->conf.error_handler);
                                    } else {
                                        buffer_copy_buffer(con->request.uri, con->error_handler);
                                    }
                                    buffer_reset(con->physical.path);

                                    con->in_error_handler = 1;

                                    connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);

                                    done = -1;
                                    break;
                                } else if (con->in_error_handler) {
                                    /* error-handler is a 404 */

                                    con->http_status = con->error_handler_saved_status;
                                }
                            } else if (con->in_error_handler) {
                                /* error-handler is back and has generated content */
                                /* if Status: was set, take it otherwise use 200 */
                            }
                        }
                        if (con->http_status == 0) con->http_status = 200;

                        /* we have something to send, go on */
                        connection_set_state(srv, con, CON_STATE_RESPONSE_START);
                        break;
            case HANDLER_WAIT_FOR_FD:
                        srv->want_fds++;

                        fdwaitqueue_append(srv, con);

                        connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);

                        break;
            case HANDLER_COMEBACK:
                        done = -1;
                        /* fallthrough */
            case HANDLER_WAIT_FOR_EVENT:
                        /* come back here */
                        connection_set_state(srv, con, CON_STATE_HANDLE_REQUEST);

                        break;
            case HANDLER_ERROR:
                        /* something went wrong */
                        connection_set_state(srv, con, CON_STATE_ERROR);
                        break;
            default:
                        log_error_write(srv, __FILE__, __LINE__, "sdd", "unknown ret-value: ", con->fd, r);
                        break;
                    }

                    break;
        case CON_STATE_RESPONSE_START:
                    /*
             * the decision is done
             * - create the HTTP-Response-Header
             *
             */

                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    if (-1 == connection_handle_write_prepare(srv, con)) {
                        connection_set_state(srv, con, CON_STATE_ERROR);

                        break;
                    }

                    connection_set_state(srv, con, CON_STATE_WRITE);
                    break;
        case CON_STATE_RESPONSE_END: /* transient */
                    /* log the request */

                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    plugins_call_handle_request_done(srv, con);

                    srv->con_written++;

                    if (con->keep_alive) {
                        connection_set_state(srv, con, CON_STATE_REQUEST_START);

#if 0
                        con->request_start = srv->cur_ts;
                        con->read_idle_ts = srv->cur_ts;
#endif
                    } else {
                        switch(r = plugins_call_handle_connection_close(srv, con)) {
                        case HANDLER_GO_ON:
                        case HANDLER_FINISHED:
                            break;
                        default:
                            log_error_write(srv, __FILE__, __LINE__, "sd", "unhandling return value", r);
                            break;
                        }

#ifdef USE_OPENSSL
                        if (srv_sock->is_ssl) {
                            switch (SSL_shutdown(con->ssl)) {
                            case 1:
                                /* done */
                                break;
                            case 0:
                                /* wait for fd-event
                         *
                         * FIXME: wait for fdevent and call SSL_shutdown again
                         *
                         */

                                break;
                            default:
                                log_error_write(srv, __FILE__, __LINE__, "ss", "SSL:",
                                                ERR_error_string(ERR_get_error(), NULL));
                            }
                        }
#endif
                        if ((0 == shutdown(con->fd, SHUT_WR))) {
                            con->close_timeout_ts = srv->cur_ts;
                            connection_set_state(srv, con, CON_STATE_CLOSE);
                        } else {
                            connection_close(srv, con);
                        }

                        srv->con_closed++;
                    }

                    connection_reset(srv, con);

                    break;
        case CON_STATE_CONNECT:
                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    chunkqueue_reset(con->read_queue);

                    con->request_count = 0;

                    break;
        case CON_STATE_CLOSE:
                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    /* we have to do the linger_on_close stuff regardless
             * of con->keep_alive; even non-keepalive sockets may
             * still have unread data, and closing before reading
             * it will make the client not see all our output.
             */
                    {
                        int len;
                        char buf[1024];

                        len = read(con->fd, buf, sizeof(buf));
                        if (len == 0 || (len < 0 && errno != EAGAIN && errno != EINTR) ) {
                            con->close_timeout_ts = srv->cur_ts - (HTTP_LINGER_TIMEOUT+1);
                        }
                    }

                    if (srv->cur_ts - con->close_timeout_ts > HTTP_LINGER_TIMEOUT) {
                        connection_close(srv, con);

                        if (srv->srvconf.log_state_handling) {
                            log_error_write(srv, __FILE__, __LINE__, "sd",
                                            "connection closed for fd", con->fd);
                        }
                    }

                    break;
        case CON_STATE_READ_POST:
        case CON_STATE_READ:
                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }
                    connection_handle_read_state(srv, con);
                    break;
        case CON_STATE_WRITE:
                    if (srv->srvconf.log_state_handling) {
                        log_error_write(srv, __FILE__, __LINE__, "sds",
                                        "state for fd", con->fd, connection_get_state(con->state));
                    }

                    /* only try to write if we have something in the queue */
                    if (!chunkqueue_is_empty(con->write_queue)) {
                        if (con->is_writable) {
                            if (-1 == connection_handle_write(srv, con)) {
                                log_error_write(srv, __FILE__, __LINE__, "ds",
                                                con->fd,
                                                "handle write failed.");
                                connection_set_state(srv, con, CON_STATE_ERROR);
                            }
                        }
                    } else if (con->file_finished) {
                        connection_set_state(srv, con, CON_STATE_RESPONSE_END);
                    }

                    break;
        case CON_STATE_ERROR: /* transient */

                    /* even if the connection was drop we still have to write it to the access log */
                    if (con->http_status) {
                        plugins_call_handle_request_done(srv, con);
                    }
#ifdef USE_OPENSSL
                    if (srv_sock->is_ssl) {
                        int ret, ssl_r;
                        unsigned long err;
                        ERR_clear_error();
                        switch ((ret = SSL_shutdown(con->ssl))) {
                        case 1:
                            /* ok */
                            break;
                        case 0:
                            ERR_clear_error();
                            if (-1 != (ret = SSL_shutdown(con->ssl))) break;

                            /* fall through */
                        default:

                            switch ((ssl_r = SSL_get_error(con->ssl, ret))) {
                            case SSL_ERROR_WANT_WRITE:
                            case SSL_ERROR_WANT_READ:
                                break;
                            case SSL_ERROR_SYSCALL:
                                /* perhaps we have error waiting in our error-queue */
                                if (0 != (err = ERR_get_error())) {
                                    do {
                                        log_error_write(srv, __FILE__, __LINE__, "sdds", "SSL:",
                                                        ssl_r, ret,
                                                        ERR_error_string(err, NULL));
                                    } while((err = ERR_get_error()));
                                } else if (errno != 0) { /* ssl bug (see lighttpd ticket #2213): sometimes errno == 0 */
                                    switch(errno) {
                                    case EPIPE:
                                    case ECONNRESET:
                                        break;
                                    default:
                                        log_error_write(srv, __FILE__, __LINE__, "sddds", "SSL (error):",
                                                        ssl_r, ret, errno,
                                                        strerror(errno));
                                        break;
                                    }
                                }

                                break;
                    default:
                                while((err = ERR_get_error())) {
                                    log_error_write(srv, __FILE__, __LINE__, "sdds", "SSL:",
                                                    ssl_r, ret,
                                                    ERR_error_string(err, NULL));
                                }

                                break;
                            }
                        }
                        ERR_clear_error();
                    }
#endif

                    switch(con->mode) {
                    case DIRECT:
#if 0
                        log_error_write(srv, __FILE__, __LINE__, "sd",
                                        "emergency exit: direct",
                                        con->fd);
#endif
                        break;
                    default:
                        switch(r = plugins_call_handle_connection_close(srv, con)) {
                        case HANDLER_GO_ON:
                        case HANDLER_FINISHED:
                            break;
                        default:
                            log_error_write(srv, __FILE__, __LINE__, "sd", "unhandling return value", r);
                            break;
                        }
                        break;
                    }

                    connection_reset(srv, con);

                    /* close the connection */
                    if ((0 == shutdown(con->fd, SHUT_WR))) {
                        con->close_timeout_ts = srv->cur_ts;
                        connection_set_state(srv, con, CON_STATE_CLOSE);

                        if (srv->srvconf.log_state_handling) {
                            log_error_write(srv, __FILE__, __LINE__, "sd",
                                            "shutdown for fd", con->fd);
                        }
                    } else {
                        connection_close(srv, con);
                    }

                    con->keep_alive = 0;

                    srv->con_closed++;

                    break;
        default:
                    log_error_write(srv, __FILE__, __LINE__, "sdd",
                                    "unknown state:", con->fd, con->state);

                    break;
        }

        if (done == -1) {
            done = 0;
        } else if (ostate == con->state) {
            done = 1;
        }
    }

    if (srv->srvconf.log_state_handling) {
        log_error_write(srv, __FILE__, __LINE__, "sds",
                        "state at exit:",
                        con->fd,
                        connection_get_state(con->state));
    }

    switch(con->state) {
    case CON_STATE_READ_POST:
    case CON_STATE_READ:
    case CON_STATE_CLOSE:
        fdevent_event_set(srv->ev, &(con->fde_ndx), con->fd, FDEVENT_IN);
        break;
    case CON_STATE_WRITE:
        /* request write-fdevent only if we really need it
         * - if we have data to write
         * - if the socket is not writable yet
         */
        if (!chunkqueue_is_empty(con->write_queue) &&
            (con->is_writable == 0) &&
            (con->traffic_limit_reached == 0)) {
            fdevent_event_set(srv->ev, &(con->fde_ndx), con->fd, FDEVENT_OUT);
        } else {
            fdevent_event_del(srv->ev, &(con->fde_ndx), con->fd);
        }
        break;
    default:
        fdevent_event_del(srv->ev, &(con->fde_ndx), con->fd);
        break;
    }

    return 0;
}
