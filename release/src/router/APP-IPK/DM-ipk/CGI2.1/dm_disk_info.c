#include <stdio.h>
#include <sys/statfs.h>
#include <dirent.h>
#include "dm_hook.h"
#include "dm_disk_info.h"

void free_2_dimension_list(int *num, char ***list) {
    int i;
    char **target = *list;

    if (*num <= 0 || target == NULL){
        *num = 0;
        return;
    }

    for (i = 0; i < *num; ++i)
        if (target[i] != NULL)
            free(target[i]);

    if (target != NULL)
        free(target);

    *num = 0;
}

int free_partition_name(struct mount *partition_name_head)
{
    struct mount *partition_name_tmp;
    partition_name_tmp = partition_name_head;
    struct mount *old_mount;
    while(partition_name_tmp != NULL)
    {
        if(partition_name_tmp->mountname != NULL)
            free(partition_name_tmp->mountname);
        old_mount = partition_name_tmp;
        partition_name_tmp = partition_name_tmp->next;
        free(old_mount);
    }
}

int main()
{
    printf("ContentType:text/html\r\n");
    printf("Cache-Control:private,max-age=0;\r\n");
    printf("\r\n");

    char *data;

    data = getenv("QUERY_STRING");
    init_cgi(data);	// there wasn't '?' in the head.
    //char *value;
    //value = websGetVar(wp, "action_mode", "");

    char *path;

    path = websGetVar(data,"action_mode", "");


    {
        char **folder_list = NULL;
        int i,folder_num,disk_num,result,rootpath;
        char buf[256];
        char *p,*q;
        struct mount *partition_name;
        struct mount *partition_name_tmp;
        struct mount *partition_name_head;

        first_log = 1;
        i=folder_num=disk_num=result=rootpath=0;
        //fprintf(stderr,"\npath=%s\n",path);
        //fprintf(stderr,"folder_num=%d\n",folder_num);

        if((strlen(path) == 9) && (strncmp(path,"/tmp/mnt/",9) == 0))
            rootpath=1;

        if(strncmp(path,"get_disk_tree",13) == 0)
            rootpath=2;


        if(rootpath==1 || rootpath==2)
        {
            FILE *fp = fopen("/proc/mounts","r");
            if(fp)
            {
                while(!feof(fp))
                {
                    memset(buf,'\0',sizeof(buf));
                    fgets(buf,255,fp);
                    if(strncmp(buf,"/dev/sd",7) == 0)
                    {
                        p = strstr(buf,"/tmp/mnt/");
                        if(p)
                        {
                            q = strstr(p," ");
                            if(q)
                            {
                                partition_name_tmp = (struct mount *)malloc(sizeof(struct mount));
                                partition_name_tmp->mountname = (char *)malloc(56);
                                partition_name_tmp->next = NULL;
                                memset(partition_name_tmp->mountname,'\0',sizeof(partition_name_tmp->mountname));
                                i++;
                                p = p + 9;
                                if(i == 1)
                                {
                                    snprintf(partition_name_tmp->mountname,strlen(p)-strlen(q)+1,"%s",p);
                                    partition_name = partition_name_tmp;
                                    partition_name_head = partition_name_tmp;
                                }
                                else
                                {
                                    snprintf(partition_name_tmp->mountname,strlen(p)-strlen(q)+1,"%s",p);
                                    partition_name->next = partition_name_tmp;
                                    partition_name = partition_name_tmp;
                                }
                                disk_num++;
                            }
                        }
                    }
                }
                fclose(fp);
            }
            else
            {
                fflush(stdout);
                return 0;
            }
            if(rootpath==2)
            {
                printf("[\"/tmp/mnt#0#%u\"]",disk_num);
                free_partition_name(partition_name_head);
                fflush(stdout);
                return 0;
            }
            else
            {
                partition_name_tmp = partition_name_head;
                i=0;
                printf("[");
                while(partition_name_tmp != NULL)
                {
                    int result_sub;
                    int folder_num_sub;
                    char **folder_list_sub = NULL;
                    char fullname_sub[1024];
                    sprintf(fullname_sub,"%s%s",path,partition_name_tmp->mountname);
                    result_sub = get_all_folder_in_mount_path(fullname_sub, &folder_num_sub, &folder_list_sub);
                    if(result_sub < 0)
                        return 0;
                    if(first_log)
                    {
                        printf("\"%s#%u#%u\"",partition_name_tmp->mountname,i,folder_num_sub);
                        first_log = 0;
                    }
                    else
                    {
                        printf(",\"%s#%u#%u\"",partition_name_tmp->mountname,i,folder_num_sub);
                    }

                    partition_name_tmp = partition_name_tmp->next;
                    i++;
                    free_2_dimension_list(&folder_num_sub, &folder_list_sub);
                }
                printf("]");
                free_partition_name(partition_name_head);
                fflush(stdout);
                return 0;
            }
        }
        else
        {
            result = get_all_folder_in_mount_path(path, &folder_num, &folder_list);

            if(result < 0)
            {
                fflush(stdout);
                return 0;
            }

            printf("[");
            for(i=0;i<folder_num;i++)
            {
                int result_sub;
                int folder_num_sub;
                char **folder_list_sub = NULL;
                char fullname_sub[1024];
                sprintf(fullname_sub,"%s%s",path,folder_list[i]);
                result_sub = get_all_folder_in_mount_path(fullname_sub, &folder_num_sub, &folder_list_sub);
                if(result_sub < 0)
                    return 0;
                if(i==0)
                {
                    printf("\"%s#%u#%u\"", folder_list[i],i,folder_num_sub);
                }
                else
                {
                    printf(",\"%s#%u#%u\"", folder_list[i],i,folder_num_sub);
                }
                free_2_dimension_list(&folder_num_sub, &folder_list_sub);
            }
            printf("]");
            free_2_dimension_list(&folder_num, &folder_list);
        }
    }

    fflush(stdout);

    return 0;
}

int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list){
    DIR *pool_to_open;
    struct dirent *dp;
    char *testdir;
    char **tmp_folder_list, **tmp_folder;
    int len, i;

    pool_to_open = opendir(mount_path);
    if(pool_to_open == NULL){
        //csprintf("Can't opendir \"%s\".\n", mount_path);
        return -1;
    }

    *sh_num = 0;
    while((dp = readdir(pool_to_open)) != NULL){
        //if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        if(dp->d_name[0] == '.')
            continue;

        if(test_if_System_folder(dp->d_name) == 1)
            continue;

        len = strlen(mount_path)+strlen("/")+strlen(dp->d_name);
        testdir = (char *)malloc(sizeof(char)*(len+1));
        if(testdir == NULL){
            closedir(pool_to_open);
            return -1;
        }
        sprintf(testdir, "%s/%s", mount_path, dp->d_name);
        testdir[len] = 0;
        if(!test_if_dir(testdir)){
            free(testdir);
            continue;
        }
        free(testdir);

        tmp_folder = (char **)malloc(sizeof(char *)*(*sh_num+1));
        if(tmp_folder == NULL){
            //csprintf("Can't malloc \"tmp_folder\".\n");

            return -1;
        }

        len = strlen(dp->d_name);
        tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
        if(tmp_folder[*sh_num] == NULL){
            //csprintf("Can't malloc \"tmp_folder[%d]\".\n", *sh_num);
            free(tmp_folder);

            return -1;
        }
        strcpy(tmp_folder[*sh_num], dp->d_name);
        if(*sh_num != 0){
            for(i = 0; i < *sh_num; ++i)
                tmp_folder[i] = tmp_folder_list[i];

            free(tmp_folder_list);
            tmp_folder_list = tmp_folder;
        }
        else
            tmp_folder_list = tmp_folder;

        ++(*sh_num);
    }
    closedir(pool_to_open);

    *folder_list = tmp_folder_list;

    return 0;
}

int test_if_System_folder(const char *const dirname){
    char *MS_System_folder[] = {"SYSTEM VOLUME INFORMATION", "RECYCLER", "RECYCLED","asusware", NULL};
    char *Linux_System_folder[] = {"lost+found", NULL};
    int i;

    for(i = 0; MS_System_folder[i] != NULL; ++i){
        if(!upper_strcmp(dirname, MS_System_folder[i]))
            return 1;
    }

    for(i = 0; Linux_System_folder[i] != NULL; ++i){
        if(!upper_strcmp(dirname, Linux_System_folder[i]))
            return 1;
    }

    return 0;
}

int test_if_dir(const char *dir){
    DIR *dp = opendir(dir);

    if(dp == NULL)
        return 0;

    closedir(dp);
    return 1;
}

int upper_strcmp(const char *const str1, const char *const str2){
    int len1, len2, i;

    len1 = strlen(str1);
    len2 = strlen(str2);
    if(len1 != len2)
        return len1-len2;

    for(i = 0; i < len1; ++i){
        if(toupper(str1[i]) != toupper(str2[i]))
            return i+1;
    }

    return 0;
}
