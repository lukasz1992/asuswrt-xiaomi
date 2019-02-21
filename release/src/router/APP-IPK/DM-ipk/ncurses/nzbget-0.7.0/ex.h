#ifndef __EX_H
#define __EX_H

#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>

#define SEM_MAGIC     0x89674532

#ifndef FILE_MODE
#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif

#ifdef SEM_FAILED
#undef SEM_FAILED
#define SEM_FAILED    ((sem_t *)(-1))
#endif



typedef struct {
		int sem_fd[2];
		int sem_magic;
		int logid;
}sema_t;

int sema_init(char *);
int sema_close(sema_t *);
int sema_open(sema_t * sem,const char *,int ,...);
int sema_unlink(const char *);
int sema_post(sema_t *);
int sema_wait(sema_t *);
void small_sleep(float);

#endif
