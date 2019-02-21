#ifndef __EX_H
#define __EX_H

#pragma (1)

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
#include <string.h>
#include <stdarg.h>

#define SEM_MAGIC       0x89674523
                                                                                                                                               
#ifdef SEM_FAILED
#undef SEM_FAILED
#define SEM_FAILED      ((Sem_t *)(-1))
#endif

#define GIFT_PORT       1213

typedef struct{
	int sem_fd[2];
	int sem_magic;
	int logid;
}Sem_t;


Sem_t  sem;

#endif
