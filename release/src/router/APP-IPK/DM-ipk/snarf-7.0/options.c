/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include "config.h"

#include "options.h"
#include "util.h"

int default_opts;
extern int debug_enabled;

unsigned char
set_options(unsigned char opts, char *optstring)
{
        int i;

        for(i = 0; optstring[i]; i++) {
                switch (optstring[i]) {
                case '-':
                        break;

                case 'r':
                        opts |= OPT_RESUME;
                        break;
                case 'R':
                        default_opts |= OPT_RESUME;
                        break;

                        /* lame, I know, but needed because resume is
                           auto-set, and disabling is not. */
                case 'n':
                        opts |= OPT_NORESUME;
                        break;
                case 'N':
                        opts |= OPT_NORESUME;

                case 'v':
                        opts |= OPT_VERBOSE;
                        break;
                case 'V':
                        default_opts |= OPT_VERBOSE;
                        break;

                case 'z':
                        opts |= OPT_BE_MOZILLA;
                        break;
                case 'Z':
                        default_opts |= OPT_BE_MOZILLA;
                        break;

                case 'm':
                        opts |= OPT_BE_MSIE;
                        break;
                case 'M':
                        default_opts |= OPT_BE_MSIE;
                        break;

                case 'a':
                        opts |= OPT_ACTIVE;
                        break;
                case 'A':
                        default_opts |= OPT_ACTIVE;
                        break;

                case 'q':
                        opts |= OPT_QUIET;
                        break;
                case 'Q':
                        default_opts |= OPT_QUIET;
                        break;

                case 'p':
                        opts |= OPT_PROGRESS;
                        break;
                case 'P':
                        default_opts |= OPT_PROGRESS;
                        break;

                case 'd':
                        debug_enabled = !debug_enabled;
                        break;
                default:
                        report(WARN, "unknown option `%c', ignoring", optstring[i]);
                }
        }

        return opts;
}
