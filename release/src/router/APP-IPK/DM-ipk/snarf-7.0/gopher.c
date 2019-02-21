/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "url.h"
#include "gopher.h"
#include "options.h"
#include "util.h"
#include "http.h"



int
gopher_transfer(UrlResource *rsrc)
{
        Url *u 		= NULL;
        int sock	= 0;
        FILE *out	= NULL;
        char *request	= NULL;

        u = rsrc->url;


        /* initialize various things */

        rsrc->proxy = get_proxy("GOPHER_PROXY");

        if( rsrc->proxy ) {
                return http_transfer(rsrc);
        }
                
        
        if( !u->path )
                u->path = strdup("");
        else
                if( strlen(u->path) > 1 )
                        u->path = strdup(u->path + 2); /* fixme: mem leak
                                                          skip the leading 
                                                          slash */

        if( !u->file )
                u->file = strdup("");

        if( !u->port )
                u->port = 70;

        if( !rsrc->outfile )
                rsrc->outfile = strdup("gopherindex.txt");

        sock = tcp_connect(u->host, u->port);

        if( !sock )
                return 0;

        request = strconcat(u->path, u->file, "\r\n", NULL);

        if( !request )
                return 0;

        out = open_outfile(rsrc);

        if( !out ) {
                report(ERR, "opening %s: %s", rsrc->outfile, strerror(errno));
                close(sock);
                return 0;
        }
        
        write(sock, request, strlen(request));

        /* never know the size of a gopher file */
        return dump_data(rsrc, sock, out);
}
