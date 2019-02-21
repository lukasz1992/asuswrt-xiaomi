
AUTOMAKE_OPTIONS = foreign ansi2knr

bin_PROGRAMS = snarf

snarf_SOURCES = \
	ftp.c    \
	ftp.h    \
	http.c   \
	http.h   \
	llist.c  \
	llist.h  \
	options.c\
	options.h\
	snarf.c  \
	url.c    \
	url.h    \
	util.c   \
	util.h   \
	gopher.c \
	gopher.h
EXTRA_snarf_SOURCES = \
	herror.c
snarf_LDADD = @HERROR@
snarf_DEPENDENCIES = @HERROR@

man_MANS = snarf.1

EXTRA_DIST = urls snarf.1 snarf-2.prj herror.c snarf.spec

rpm:
	make dist
	mv snarf-@VERSION@.tar.gz /usr/src/redhat/SOURCES
	rpm -bb snarf.spec
