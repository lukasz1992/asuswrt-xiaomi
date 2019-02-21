/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPT_RESUME 	(1 << 0)
#define OPT_VERBOSE	(1 << 1)
#define OPT_QUIET	(1 << 2)
#define OPT_ACTIVE	(1 << 3)       	/* For FTP only */
#define OPT_NORESUME	(1 << 4)	/* see comments in option.c about
                                           this lameness */
#define OPT_PROGRESS	(1 << 5)	/* for python aka markus fleck */
#define OPT_BE_MOZILLA	(1 << 6)        /* To act like Mozilla */
#define OPT_BE_MSIE	(1 << 7)        /* To act like MSIE */

/* Funcs */

#ifdef PROTOTYPES

unsigned char set_options(unsigned char, char *);

#endif /* PROTOTYPES */


extern int default_opts;

#endif /* OPTIONS_H */


