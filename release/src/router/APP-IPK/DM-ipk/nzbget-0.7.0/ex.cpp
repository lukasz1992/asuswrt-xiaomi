#include <iostream>
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

#include "ex.h"
#include "Log.h"

//extern int semUse;
//extern sema_t* psem_nntp;
//sema_t sema;


int sema_init(char * semName){
        //return 0;// rich changed 091118
        //std::string semName;
	int  flag_sem = O_CREAT|O_RDWR;
	int  init_value = 1;

	//printf("\t\t\t\t\t pid %d sema_init\n", getpid());
	
        //semName = "/shares/dmathined/Download/.sems/sem." + hash;
        sema_t sema;
        //semUse = 1;

        if (sema_open(&sema, semName, flag_sem, FILE_MODE, init_value) == (-1)){
                //semUse = 0;
               info("open %s error",semName);
		return -1;
		}
        //chmod(semName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//Allen modify for M25
	
	return 0;	
}

int sema_open(sema_t *sem, const char *pathname, int oflag, ... ){
        //return 0;// rich changed 091118
        int     i, flags, save_errno;
        char    c;
        mode_t  mode;
        va_list ap;
        //Sem_t *sem;
        unsigned int    value;
	//printf("\t\t\t\t\t pid %d sema_open : %s\n", getpid(), pathname);

        if (oflag & O_CREAT) {
                va_start(ap, oflag);            /* init ap to final named argument */
                mode = va_arg(ap, mode_t);
                value = va_arg(ap, unsigned int);
                va_end(ap);
                                                                                                   
                if (mkfifo(pathname, mode) < 0) {
                        if (errno == EEXIST && (oflag & O_EXCL) == 0)
                                oflag &= ~O_CREAT;      /* already exists, OK */
                        else{
				perror("sem mkfifo :");
                                //return((Sem_t *)(-1));
				return -1;
			}
                }
        }
                                                                   
/* *INDENT-OFF* */
        //if ( (sem = (Sem_t *)malloc(sizeof(Sem_t))) == NULL){
	//	fprintf(stderr, "return 2\n");
	//	perror("sem malloc :");
        //        return((Sem_t *)(-1));
	//}
	memset(sem, '\0', sizeof(sema_t));

	if(sem == NULL)
		return -1;

        sem->sem_fd[0] = sem->sem_fd[1] = -1;

        if ( (sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0)
                goto error;
		if ( (sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0)
                goto error;

/* *INDENT-ON* */

        if ( (flags = fcntl(sem->sem_fd[0], F_GETFL, 0)) < 0)
                goto error;
        flags &= ~O_NONBLOCK;
        if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0)
                goto error;
                                                                                                                                               
        if (oflag & O_CREAT) {          /* initialize semaphore */
                for (i = 0; i < value; i++)
                        if (write(sem->sem_fd[1], &c, 1) != 1)
                                goto error;
        }
                                                                                                                                               
        sem->sem_magic = SEM_MAGIC;
        //return(sem);
	return 0;
                                                                                                                                               
error:
        save_errno = errno;
        if (oflag & O_CREAT)
                unlink(pathname);               /* if we created FIFO */
        close(sem->sem_fd[0]);          /* ignore error */
        close(sem->sem_fd[1]);          /* ignore error */
        //free(sem);
	memset(sem, '\0', sizeof(sema_t));
        errno = save_errno;
	//perror("sem error :");
        //return((Sem_t *)(-1));
	return -1;
}

int sema_close(sema_t *sem){
	//printf("\t\t\t\t\t pid %d sema close\n", getpid());
        //return 0;// rich changed 091118
        if (sem->sem_magic != SEM_MAGIC) {
                errno = EINVAL;
                return(-1);
        }
                                                                                                                                               
        sem->sem_magic = 0;             /* in case caller tries to use it later */
        if (close(sem->sem_fd[0]) == -1 || close(sem->sem_fd[1]) == -1) {
                //free(sem);
		memset(sem, '\0', sizeof(sema_t));
                return(-1);
        }
        //free(sem);
	memset(sem, '\0', sizeof(sema_t));
        return(0);
}

int sema_unlink(const char *pathname){
	//printf("\t\t\t\t\t pid %d sema unlink\n", getpid());
        //return 0;// rich changed 091118
        return(unlink(pathname));
}

int sema_wait(sema_t *sem){
        //return 0;// rich changed 091118
        char    c;
                                                                                                                                               
	//printf("\t\t\t\t\t pid %d sema wait\n", getpid());
        if (sem->sem_magic != SEM_MAGIC) {
                errno = EINVAL;
                return(-1);
        }
                                                                                                                                               
        if (read(sem->sem_fd[0], &c, 1) == 1)
                return(0);
        return(-1);
}

int sema_post(sema_t *sem){
        //return 0;// rich changed 091118
        char    c;
                                                                                                                                               
	//printf("\t\t\t\t\t pid %d sema post\n", getpid());
        if (sem->sem_magic != SEM_MAGIC) {
                errno = EINVAL;
                return(-1);
        }
                                                                                                                                               
        if (write(sem->sem_fd[1], &c, 1) == 1)
                return(0);
        return(-1);
}

void small_sleep(float nsec){
        struct timeval  tval;
                                                                                                                                               
        tval.tv_sec = (int)(nsec*1000000) / 1000000;
        tval.tv_usec = (int)(nsec*1000000) % 1000000;
                                                                                                                                               
        select(0, NULL, NULL, NULL, &tval);
}


