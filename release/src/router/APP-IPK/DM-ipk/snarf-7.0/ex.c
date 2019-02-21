#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "ex.h"



// ******** semaphore impl

int Sem_open(Sem_t *sem, const char *pathname, int oflag, ... ){
    //printf("pathname is %s \n",pathname);
    int     i, flags, save_errno;
    char    c ;
    mode_t  mode;
    va_list ap;
    //Sem_t *sem;
    unsigned int    value;

    //printf("O_CREAT is %d\n",O_CREAT);

    if (oflag & O_CREAT) {
        //printf("create fifo start \n");
        va_start(ap, oflag);            /* init ap to final named argument */
        mode = va_arg(ap, mode_t);
        value = va_arg(ap, unsigned int);
        va_end(ap);

        if (mkfifo(pathname, mode) < 0) {
        //if (mknod(pathname, mode,0) < 0) {
        //if(mkfifo(pathname,S_IFIFO | 0666) == -1)
            if (errno == EEXIST && (oflag & O_EXCL) == 0)
            {
                oflag &= ~O_CREAT;      /* already exists, OK */
                fprintf(stderr,"create fifo error \n");
            }
            else{
                perror("sem mkfifo :");
                //return((Sem_t *)(-1));
                return -1;
            }
        }

    }
    /* *INDENT-OFF* */
    //if ( (sem = (Sem_t *)malloc(sizeof(Sem_t))) == NULL){
    //      perror("sem malloc :");
    //        return((Sem_t *)(-1));
    //}
    memset(sem, '\0', sizeof(Sem_t));

    if(sem == NULL)
    {
        printf("sem is NULL \n");
        return -1;
    }

    sem->sem_fd[0] = sem->sem_fd[1] = -1;

    if ( (sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0)
    {
        printf("error 1\n");
        goto error;
    }
    if ( (sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0)
    {
        printf("error 2\n");
        goto error;
    }
    /* *INDENT-ON* */

    if ( (flags = fcntl(sem->sem_fd[0], F_GETFL, 0)) < 0)
    {
        printf("error 3\n");
        goto error;
    }
    flags &= ~O_NONBLOCK;
    if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0)
    {
        printf("error 4\n");
        goto error;
    }

    //printf("test write start \n");

    if (oflag & O_CREAT) {          /* initialize semaphore */
        for (i = 0; i < value; i++)
            if (write(sem->sem_fd[1], &c, 1) != 1)
            {
            printf("error 5\n");
            perror("write error:");
            goto error;
        }
    }

    //printf(" open sem end \n");

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
    memset(sem, '\0', sizeof(Sem_t));
    errno = save_errno;
    perror("sem error :");
    //return((Sem_t *)(-1));
    return -1;
}

int makefifo(char *path)
{
    if(mkfifo(path,S_IFIFO | 0666) == -1)
    {
        perror("mkfifo error");
        return -1;
    }
}

int
        Sem_close(Sem_t *sem)
{
    if (sem->sem_magic != SEM_MAGIC) {
        errno = EINVAL;
        return(-1);
    }

    sem->sem_magic = 0;             /* in case caller tries to use it later */
    if (close(sem->sem_fd[0]) == -1 || close(sem->sem_fd[1]) == -1) {
        //free(sem);
        memset(sem, '\0', sizeof(Sem_t));
        return(-1);
    }
    //free(sem);
    memset(sem, '\0', sizeof(Sem_t));
    return(0);
}

int
        Sem_unlink(const char *pathname)
{
    return(unlink(pathname));
}

int
        Sem_wait(Sem_t *sem)
{
    char    c;

    //printf("$$[sn] sem wait\n");	// tmp test
    if (sem->sem_magic != SEM_MAGIC) {
        errno = EINVAL;
        return(-1);
    }

    if (read(sem->sem_fd[0], &c, 1) == 1)
        return(0);
    return(-1);
}

int
        Sem_post(Sem_t *sem)
{
    char    c;

    if (sem->sem_magic != SEM_MAGIC) {
        errno = EINVAL;
        return(-1);
    }

    if (write(sem->sem_fd[1], &c, 1) == 1){
        //printf("$$[sn] sem post\n");	// tmp test
        return(0);
    }
    return(-1);
}

// ********~
