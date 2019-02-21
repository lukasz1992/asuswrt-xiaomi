/*
 * CGI helper functions
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: dm_cgi.c,v 1.1.1.1 2011/06/02 01:00:43 magic_pan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef BCMDBG
#include <assert.h>
#else
#define assert(a)
#endif

#if defined(linux)
/* Use SVID search */
#define __USE_GNU
#include <search.h>
#elif defined(vxworks)
/* Use vxsearch */
#include <vxsearch.h>
extern char *strsep(char **stringp, char *delim);
#endif

/* CGI hash table */
static struct hsearch_data htab;

static void
        unescape(char *s)
{
    unsigned int c;

    while ((s = strpbrk(s, "%+"))) {
        /* Parse %xx */
        if (*s == '%') {
            sscanf(s + 1, "%02x", &c);
            *s++ = (char) c;
            strncpy(s, s + 2, strlen(s) + 1);
        }
        /* Space is special */
        else if (*s == '+')
            *s++ = ' ';
    }
}

char *
        get_cgi(char *name)
{
    ENTRY e, *ep;

    if (!htab.table)
        return NULL;

    e.key = name;
    hsearch_r(e, FIND, &ep, &htab);

    return ep ? ep->data : NULL;
}

void
        set_cgi(char *name, char *value)
{
    ENTRY e, *ep;

    if (!htab.table)
        return;

    e.key = name;
    hsearch_r(e, FIND, &ep, &htab);
    if (ep)
        ep->data = value;
    else {
        e.data = value;
        hsearch_r(e, ENTER, &ep, &htab);
    }
    assert(ep);
}

void
        init_cgi(char *query)
{
    int len, nel;
    char *q, *name, *value;

    /* Clear variables */
    if (!query) {
        hdestroy_r(&htab);
        return;
    }

    /* Parse into individual assignments */
    q = query;
    len = strlen(query);
    nel = 1;
    while (strsep(&q, "&;"))
        nel++;
    hcreate_r(nel, &htab);

    for (q = query; q < (query + len);) {
        /* Unescape each assignment */
        unescape(name = value = q);

        /* Skip to next assignment */
        for (q += strlen(q); q < (query + len) && !*q; q++);

        /* Assign variable */
        name = strsep(&value, "=");
        if (value){
            set_cgi(name, value);
        }
    }
}
