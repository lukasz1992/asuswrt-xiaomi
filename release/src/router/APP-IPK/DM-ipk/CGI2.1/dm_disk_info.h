#ifndef DM_DISK_INFO_H
#define DM_DISK_INFO_H 1

#include <stdio.h>

struct mount{
    char *mountname;
    struct mount *next;
};


int get_all_folder_in_mount_path(const char *const , int *, char ***);
int test_if_System_folder(const char *const );
int test_if_dir(const char *);
int upper_strcmp(const char *const , const char *const );

//int yantest();
#endif
