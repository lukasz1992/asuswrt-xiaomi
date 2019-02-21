/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 to include debug-code */
/* #undef DEBUG */

/* Define to 1 to not use curses */
/* #undef DISABLE_CURSES */

/* Define to 1 to disable smart par-verification and restoration */
#define DISABLE_PARCHECK 1

/* Define to 1 to not use TLS/SSL */
/* #undef DISABLE_TLS */

/* Define to the name of macro which returns the name of function being
   compiled */
/* #undef FUNCTION_MACRO_NAME */

/* Define to 1 to create stacktrace on segmentation faults */
/* #undef HAVE_BACKTRACE */

/* Define to 1 if ctime_r takes 2 arguments */
#define HAVE_CTIME_R_2 1

/* Define to 1 if ctime_r takes 3 arguments */
/* #undef HAVE_CTIME_R_3 */

/* Define to 1 if you have the <curses.h> header file. */
#define HAVE_CURSES_H 1

/* Define to 1 if getaddrinfo is supported */
#define HAVE_GETADDRINFO 1

/* Define to 1 if gethostbyname_r is supported */
/* #undef HAVE_GETHOSTBYNAME_R */

/* Define to 1 if gethostbyname_r takes 3 arguments */
/* #undef HAVE_GETHOSTBYNAME_R_3 */

/* Define to 1 if gethostbyname_r takes 5 arguments */
/* #undef HAVE_GETHOSTBYNAME_R_5 */

/* Define to 1 if gethostbyname_r takes 6 arguments */
/* #undef HAVE_GETHOSTBYNAME_R_6 */

/* Define to 1 if getopt_long is supported */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 to use GnuTLS library for TLS/SSL-support. */
/* #undef HAVE_LIBGNUTLS */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ncurses.h> header file. */
/* #undef HAVE_NCURSES_H */

/* Define to 1 if you have the <ncurses/ncurses.h> header file. */
/* #undef HAVE_NCURSES_NCURSES_H */

/* Define to 1 to use OpenSSL library for TLS/SSL-support. */
#define HAVE_OPENSSL 1

/* Define to 1 if libpar2 supports cancelling (needs a special patch) */
/* #undef HAVE_PAR2_CANCEL */

/* Define to 1 if stat64 is supported */
#define HAVE_STAT64 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/prctl.h> header file. */
#define HAVE_SYS_PRCTL_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if variadic macros are supported */
/* #undef HAVE_VARIADIC_MACROS */

/* Name of package */
#define PACKAGE "nzbget"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "hugbug@users.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "nzbget"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "nzbget 0.7.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "nzbget"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.7.0"

/* Determine what socket length (socklen_t) data type is */
#define SOCKLEN_T socklen_t

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.7.0"
