/* -*- mode: C; c-basic-offset: 8; indent-tabs-mode: nil; tab-width: 8 -*- */

#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

char	*h_errlist[] = {
	"Error 0",
	"Unknown host",
	"Host name lookup failure",
	"Unknown server error",
	"No address associated with name",
};
int	h_nerr = 5;

/*
 * herror --
 *	print the error indicated by the h_errno value.
 */
void
herror(s)
	const char *s;
{
	struct iovec iov[4];
	register struct iovec *v = iov;
	int error = h_errno;

	if (s && *s) {
		v->iov_base = (char *)s;
		v->iov_len = strlen(s);
		v++;
		v->iov_base = ": ";
		v->iov_len = 2;
		v++;
	}
	v->iov_base = ((unsigned int)(error) < h_nerr) ?
		h_errlist[error] : "Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = "\n";
	v->iov_len = 1;
	writev(STDERR_FILENO, iov, (v - iov) + 1);
}

char *
hstrerror(err)
	int err;
{
	return ((unsigned int)(err) < h_nerr) ? h_errlist[err]
		: "Unknown error";
}



