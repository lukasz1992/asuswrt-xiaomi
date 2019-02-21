#include "server.h"
#include "buffer.h"
#include "network.h"
#include "log.h"
#include "keyvalue.h"
#include "response.h"
#include "request.h"
#include "chunk.h"
#include "http_chunk.h"
#include "fdevent.h"
#include "connections.h"
#include "stat_cache.h"
#include "plugin.h"
#include "joblist.h"
#include "network_backends.h"
#include "version.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <locale.h>

#include <stdio.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifdef HAVE_VALGRIND_VALGRIND_H
# include <valgrind/valgrind.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifdef HAVE_PWD_H
# include <grp.h>
# include <pwd.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#ifdef HAVE_SYS_PRCTL_H
# include <sys/prctl.h>
#endif

#ifdef USE_OPENSSL
# include <openssl/err.h> 
#endif

#ifndef __sgi
/* IRIX doesn't like the alarm based time() optimization */
/* #define USE_ALARM */
#endif

#ifdef HAVE_GETUID
# ifndef HAVE_ISSETUGID

static int l_issetugid(void) {
    return (geteuid() != getuid() || getegid() != getgid());
}

#  define issetugid l_issetugid
# endif
#endif

#include <dm_func.h> //2011.0822 magic added

static volatile sig_atomic_t srv_shutdown = 0;
static volatile sig_atomic_t graceful_shutdown = 0;
static volatile sig_atomic_t handle_sig_alarm = 1;
static volatile sig_atomic_t handle_sig_hup = 0;
static volatile sig_atomic_t forwarded_sig_hup = 0;

int have_nice=0;
char hd_n[256],hd_n_t[256];

char usb_node_b[32];
unsigned int val_b;
char  port_path_b[8];
char  usb_get[1024];
int path_num=1;
FILE *fp;
char *dm_mount=NULL;
char dm_mount_node[128];
char dm_mount_path[128];
char dm_mount_tmp[128];
char *dm_mount_hci=NULL;
void check_nice(){
    have_nice=0;
    if(access("/bin/nice",0)==0){
        have_nice=1;
    }
    else{
        have_nice=0;
    }
}
// 20170509 leo added for hub {
enum {
    DEVICE_TYPE_UNKNOWN=0,
    DEVICE_TYPE_DISK,
    DEVICE_TYPE_PRINTER,
    DEVICE_TYPE_SG,
    DEVICE_TYPE_CD,
    DEVICE_TYPE_MODEM,
    DEVICE_TYPE_BECEEM
};
#define SYS_BLOCK "/sys/block"
typedef unsigned int u32;
#define MOUNT_FILE "/proc/mounts"
#define SYS_TTY "/sys/class/tty"
#define SYS_SG "/sys/class/scsi_generic"
#define SYS_USB "/sys/class/usb"
#define foreach(word, wordlist, next) \
        for (next = &wordlist[strspn(wordlist, " ")], \
                strncpy(word, next, sizeof(word)), \
                word[strcspn(word, " ")] = '\0', \
                word[sizeof(word) - 1] = '\0', \
                next = strchr(next, ' '); \
                strlen(word); \
                next = next ? &next[strspn(next, " ")] : "", \
                strncpy(word, next, sizeof(word)), \
                word[strcspn(word, " ")] = '\0', \
                word[sizeof(word) - 1] = '\0', \
                next = strchr(next, ' '))

#define SDCARD_PORT USB_EHCI_PORT_3
#define M2_SSD_PORT USB_EHCI_PORT_3


#define USB_XHCI_PORT_1 get_usb_xhci_port(0)
#define USB_XHCI_PORT_2 get_usb_xhci_port(1)
#define USB_EHCI_PORT_1 get_usb_ehci_port(0)
#define USB_EHCI_PORT_2 get_usb_ehci_port(1)
#define USB_OHCI_PORT_1 get_usb_ohci_port(0)
#define USB_OHCI_PORT_2 get_usb_ohci_port(1)
#define USB_EHCI_PORT_3 get_usb_ehci_port(2)
#define USB_OHCI_PORT_3 get_usb_ohci_port(2)
char xhci_string[32];
char ehci_string[32];
char ohci_string[32];


/*  function: use popen to get nvram
- *  return:   return a point to the value and need to free.
- *            If the nvram didn't exist,it will return NULL;
- * */
char *
    nvram_get_by_popen(const char *name)
{
    FILE *fp;
    char cmd[128];
    char *ret=NULL;

    ret = (char*)malloc(128);
    if(ret == NULL)
        return NULL;
    memset(cmd, '\0', 128);
    memset(ret, '\0', 128);

    snprintf(cmd, sizeof(cmd), "nvram get %s", name);
    fp = popen(cmd, "r");
    if(fp) {
        int len = fread(ret, sizeof(char), 128, fp);
        if( len > 1 ) {
            ret[len-1] = '\0';
        } else {
            free(ret);
            ret = NULL;
        }
        pclose(fp);
    }
    return ret;
}

int
    nvram_get_int(const char *name)
{
    char* value = nvram_get_by_popen(name);
    if(value) {
        int ret = atoi(value);
        free(value);
        return ret;
    } else {
        return -1;
    }

}

#ifdef DM_MIPSBIG
char *
    tcapi_get_by_popen(const char *node, const char *name)
{
    FILE *fp;
    char cmd[128];
    char *ret=NULL;

    ret = (char*)malloc(128);
    if(ret == NULL)
        return NULL;
    memset(cmd, '\0', 128);
    memset(ret, '\0', 128);
    snprintf(cmd, sizeof(cmd), "/userfs/bin/tcapi get %s %s", node, name);
    fp = popen(cmd, "r");
    if(fp) {
        int len = fread(ret, sizeof(char), 128, fp);
        if( len > 1 ) {
            ret[len-1] = '\0';
        } else {
            free(ret);
            ret = NULL;
        }
        pclose(fp);
    }
    return ret;
}

int
    tcapi_get_int(const char *node, const char *name)
{
    char* value = tcapi_get_by_popen(node, name);
    if(value) {
        int ret = -1;
        if(strncmp(value, "no attribute information", 24) == 0) {
            ret = -1;
        } else {
            ret = atoi(value);
        }
        free(value);
        return ret;
    } else {
        return -1;
    }

}
#endif

char *get_usb_xhci_port(int port)
{
    char word[100], *next;
    int i=0;
    strcpy(xhci_string, "xxxxxxxx");
#ifdef DM_MIPSBIG
if (access("/userfs/bin/tcapi",0) == 0){
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=tcapi_get_by_popen("USB_Entry","xhci_ports");
    if(strcmp(dm_mount_hci,"no attribute information") == 0){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        return "##xxxxxx";
    }
    free(dm_mount_hci);
    dm_mount_hci=NULL;
    foreach(word, tcapi_get_by_popen("USB_Entry","xhci_ports"), next){
            if(i == port){
                strcpy(xhci_string, word);
                break;
            }
            i++;
        }
    }
    else{
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("xhci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        return "##xxxxxx";
    }
    free(dm_mount_hci);
    dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("xhci_ports"), next){
            if(i == port){
                strcpy(xhci_string, word);
                break;
            }
            i++;
        }
    }
#else
dm_mount_hci=(char *)malloc(28);
dm_mount_hci=nvram_get_by_popen("xhci_ports");
if(dm_mount_hci == NULL){
    free(dm_mount_hci);
    dm_mount_hci=NULL;
    return "##xxxxxx";
}
free(dm_mount_hci);
dm_mount_hci=NULL;
foreach(word, nvram_get_by_popen("xhci_ports"), next){
        if(i == port){
            strcpy(xhci_string, word);
            break;
        }
        i++;
    }
#endif
    return xhci_string;
}

char *get_usb_ehci_port(int port)
{
    char word[100], *next;
    int i=0;
    strcpy(ehci_string, "xxxxxxxx");

#ifdef DM_MIPSBIG
    if(access("/userfs/bin/tcapi",0) == 0){
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=tcapi_get_by_popen("USB_Entry","ehci_ports");
        if(strcmp(dm_mount_hci,"no attribute information") == 0){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
            return "##xxxxxx";
        }
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        foreach(word, tcapi_get_by_popen("USB_Entry","ehci_ports"), next) {
            if(i==port) {
                strcpy(ehci_string, word);
                break;
            }
            i++;
        }
    }
    else{
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=nvram_get_by_popen("ehci_ports");
        if(dm_mount_hci == NULL){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
            return "##xxxxxx";
        }
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        foreach(word, nvram_get_by_popen("ehci_ports"), next) {
            if(i==port) {
                strcpy(ehci_string, word);
                break;
            }
            i++;
        }
    }
#else
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("ehci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        return "##xxxxxx";
    }
    free(dm_mount_hci);
    dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("ehci_ports"), next) {
        if(i==port) {
            strcpy(ehci_string, word);
            break;
        }
        i++;
    }
#endif
    return ehci_string;
}

char *get_usb_ohci_port(int port)
{
    char word[100], *next;
    int i=0;
    strcpy(ohci_string, "xxxxxxxx");
#ifdef DM_MIPSBIG
    if(access("/userfs/bin/tcapi",0) == 0){
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=tcapi_get_by_popen("USB_Entry","ohci_ports");
        if(strcmp(dm_mount_hci,"no attribute information") == 0){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
            return "##xxxxxx";
        }
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        foreach(word, tcapi_get_by_popen("USB_Entry","ohci_ports"), next) {
            if(i==port) {
                strcpy(ohci_string, word);
                break;
            }
            i++;
        }
    }
    else{
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=nvram_get_by_popen("ohci_ports");
        if(dm_mount_hci == NULL){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
            return "##xxxxxx";
        }
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        foreach(word, nvram_get_by_popen("ohci_ports"), next) {
            if(i==port) {
                strcpy(ohci_string, word);
                break;
            }
            i++;
        }
    }
#else
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("ohci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
        return "##xxxxxx";
    }
    free(dm_mount_hci);
    dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("ohci_ports"), next) {
        if(i==port) {
            strcpy(ohci_string, word);
            break;
        }
        i++;
    }
#endif
    return ohci_string;
}



// Get USB node.
int isStorageDevice(const char *device_name){
    if(!strncmp(device_name, "sd", 2))
        return 1;

    return 0;
}

int isMMCDevice(const char *device_name){
    if(!strncmp(device_name, "mmcblk", 6))
        return 1;

    return 0;
}
int isBeceemNode(const char *device_name)
{
    if(strstr(device_name, "usbbcm") == NULL)
        return 0;

    return 1;
}
int isTTYNode(const char *device_name)
{
    if(strncmp(device_name, "tty", 3))
        return 0;

    return 1;
}

int get_device_type_by_device(const char *device_name)
{
    if(device_name == NULL || strlen(device_name) <= 0){
        return DEVICE_TYPE_UNKNOWN;
    }

    if(isStorageDevice(device_name) || isMMCDevice(device_name) ){
        return DEVICE_TYPE_DISK;
    }

//#ifdef RTCONFIG_USB_PRINTER
    if(!strncmp(device_name, "lp", 2)){
        return DEVICE_TYPE_PRINTER;
    }
//#endif
//#ifdef RTCONFIG_USB_MODEM
    if(!strncmp(device_name, "sg", 2)){
        return DEVICE_TYPE_SG;
    }
    if(!strncmp(device_name, "sr", 2)){
        return DEVICE_TYPE_CD;
    }
    if(isTTYNode(device_name)){
        return DEVICE_TYPE_MODEM;
    }
//#endif
//#ifdef RTCONFIG_USB_BECEEM
    if(isBeceemNode(device_name)){
        return DEVICE_TYPE_BECEEM;
    }
//#endif

    return DEVICE_TYPE_UNKNOWN;
}
char *read_whole_file(const char *target) {
    FILE *fp = fopen(target, "r");
    char *buffer, *new_str;
    int i;
    unsigned int read_bytes = 0;
    unsigned int each_size = 1024;

    if (fp == NULL)
        return NULL;

    buffer = (char *)malloc(sizeof(char)*each_size+read_bytes);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }
    memset(buffer, 0, sizeof(char)*each_size+read_bytes);

    while ((i = fread(buffer+read_bytes, each_size * sizeof(char), 1, fp)) == 1){
        read_bytes += each_size;
        new_str = (char *)malloc(sizeof(char)*each_size+read_bytes);
        if (new_str == NULL) {
            free(buffer);
            fclose(fp);
            return NULL;
        }
        memset(new_str, 0, sizeof(char)*each_size+read_bytes);
        memcpy(new_str, buffer, read_bytes);

        free(buffer);
        buffer = new_str;
    }

    fclose(fp);
    return buffer;
}

 int get_disk_partitionnumber(const char *string, u32 *partition_number, u32 *mounted_number){
    char disk_name[8];
    char target_path[128];
    DIR *dp;
    struct dirent *file;
    int len;
    char *mount_info = NULL, target[8];

    if(string == NULL)
        return 0;

    if(partition_number == NULL)
        return 0;

    *partition_number = 0; // initial value.
    if(mounted_number != NULL){
        *mounted_number = 0; // initial value.
        mount_info = read_whole_file(MOUNT_FILE);
    }

    len = strlen(string);
    if(!is_disk_name(string)){
        while(isdigit(string[len-1]))
            --len;

//#ifdef BCM_MMC
        if(string[len-1] == 'p')
            --len;
//#endif
    }
    else if(mounted_number != NULL && mount_info != NULL){
        memset(target, 0, 8);
        sprintf(target, "%s ", string);
        if(strstr(mount_info, target) != NULL)
            ++(*mounted_number);
    }
    memset(disk_name, 0, 8);
    strncpy(disk_name, string, len);

    memset(target_path, 0, 128);
    sprintf(target_path, "%s/%s", SYS_BLOCK, disk_name);
    if((dp = opendir(target_path)) == NULL){
        if(mount_info != NULL)
            free(mount_info);
        return 0;
    }

    len = strlen(disk_name);
    while((file = readdir(dp)) != NULL){
        if(file->d_name[0] == '.')
            continue;

        if(!strncmp(file->d_name, disk_name, len)){
            ++(*partition_number);

            if(mounted_number == NULL || mount_info == NULL)
                continue;

            memset(target, 0, 8);
            sprintf(target, "%s ", file->d_name);
            if(strstr(mount_info, target) != NULL)
                ++(*mounted_number);
        }
    }
    closedir(dp);
    if(mount_info != NULL)
        free(mount_info);

    return 1;
}

 int isM2SSDDevice(const char *device_name)
 {
     char disk_name[32], *p;
     char disk_path[PATH_MAX], path[PATH_MAX];
     if(strncmp(device_name, "sd", 2))
         return 0;
     snprintf(disk_name,sizeof(disk_name),"%s",device_name);
     for (p = disk_name + strlen(disk_name) - 1; isdigit(*p) && p > disk_name; p--)
         *p = '\0';
     snprintf(disk_path, sizeof(disk_path), "/sys/block/%s", disk_name);
     if (readlink(disk_path, path, sizeof(path)) <= 0 || !strstr(path, "ahci"))
         return 0;

     return 1;
 }

int is_partition_name(const char *device_name, u32 *partition_order){
    int order;
    u32 partition_number;

    if(partition_order != NULL)
        *partition_order = 0;

    if(get_device_type_by_device(device_name) != DEVICE_TYPE_DISK)
        return 0;

    // get the partition number in the device_name
//#ifdef BCM_MMC
    if(isMMCDevice(device_name)) // SD card: mmcblk0p1.
        order = (u32)strtol(device_name+8, NULL, 10);
    else // sda1.
//#endif
        order = (u32)strtol(device_name+3, NULL, 10);
    if(order <= 0 || order == LONG_MIN || order == LONG_MAX)
        return 0;

    if(!get_disk_partitionnumber(device_name, &partition_number, NULL))
        return 0;

    if(partition_order != NULL)
        *partition_order = order;

    return 1;
}

int is_disk_name(const char *device_name){
    if(get_device_type_by_device(device_name) != DEVICE_TYPE_DISK)
        return 0;

//#ifdef BCM_MMC
    if(isMMCDevice(device_name)){
        if(strchr(device_name, 'p'))
            return 0;
    }
    else
//#endif
    if(isdigit(device_name[strlen(device_name)-1]))
        return 0;

    return 1;
}

char *get_disk_name(const char *string, char *buf, const int buf_size){
    int len;
    if(string == NULL || buf_size <= 0)
        return NULL;
    memset(buf, 0, buf_size);
    if(!is_disk_name(string) && !is_partition_name(string, NULL))
        return NULL;
    len = strlen(string);
    if(!is_disk_name(string)){
        while(isdigit(string[len-1]))
            --len;
//#ifdef BCM_MMC
        if(string[len-1] == 'p')
            --len;
//#endif
    }
    if(len > buf_size)
        return NULL;

    strncpy(buf, string, len);
    return buf;
}

int get_usb_port_number(const char *usb_port)
{
    char word[100], *next;
    int i;
    i = 0;
#ifdef DM_MIPSBIG
    if(access("/userfs/bin/tcapi",0) == 0){
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=tcapi_get_by_popen("USB_Entry","xhci_ports");
        if(strcmp(dm_mount_hci,"no attribute information") == 0){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
          }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, tcapi_get_by_popen("USB_Entry","xhci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
    else{
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=nvram_get_by_popen("xhci_ports");
        if(dm_mount_hci == NULL){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, nvram_get_by_popen("xhci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
#else
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("xhci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    }
    else{
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("xhci_ports"), next){
        ++i;
        if(!strcmp(usb_port, word)){
            return i;
        }
    }
    }
#endif


    i = 0;
#ifdef DM_MIPSBIG
    if(access("/userfs/bin/tcapi",0) == 0){
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=tcapi_get_by_popen("USB_Entry","ehci_ports");
        if(strcmp(dm_mount_hci,"no attribute information") == 0){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, tcapi_get_by_popen("USB_Entry","ehci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
    else{
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=nvram_get_by_popen("ehci_ports");
        if(dm_mount_hci == NULL){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, nvram_get_by_popen("ehci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
#else
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("ehci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    }
    else{
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("ehci_ports"), next){
        ++i;
        if(!strcmp(usb_port, word)){
            return i;
        }
    }
    }
#endif


    i = 0;
#ifdef DM_MIPSBIG
    if(access("/userfs/bin/tcapi",0) == 0){
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=tcapi_get_by_popen("USB_Entry","ohci_ports");
        if(strcmp(dm_mount_hci,"no attribute information") == 0){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, tcapi_get_by_popen("USB_Entry","ohci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
    else{
        dm_mount_hci=(char *)malloc(28);
        dm_mount_hci=nvram_get_by_popen("ohci_ports");
        if(dm_mount_hci == NULL){
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        }
        else{
            free(dm_mount_hci);
            dm_mount_hci=NULL;
        foreach(word, nvram_get_by_popen("ohci_ports"), next){
            ++i;
            if(!strcmp(usb_port, word)){
                return i;
            }
        }
        }
    }
#else
    dm_mount_hci=(char *)malloc(28);
    dm_mount_hci=nvram_get_by_popen("ohci_ports");
    if(dm_mount_hci == NULL){
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    }
    else{
        free(dm_mount_hci);
        dm_mount_hci=NULL;
    foreach(word, nvram_get_by_popen("ohci_ports"), next){
        ++i;
        if(!strcmp(usb_port, word)){
            return i;
        }
    }
    }
#endif


    return 0;
}

char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size)
{
    memset(buf, 0, buf_size);

    if(strstr(target_string, (const char *)USB_XHCI_PORT_1))
    {strcpy(buf, (const char *)USB_XHCI_PORT_1);
    }
    else if(strstr(target_string, (const char *)USB_XHCI_PORT_2))
    {strcpy(buf, (const char *)USB_XHCI_PORT_2);
    }
    else if(strstr(target_string, (const char *)USB_EHCI_PORT_1))
    {strcpy(buf, (const char *)USB_EHCI_PORT_1);
    }
    else if(strstr(target_string, (const char *)USB_EHCI_PORT_2))
    {strcpy(buf, (const char *)USB_EHCI_PORT_2);
    }
    else if(strstr(target_string, (const char *)USB_OHCI_PORT_1))
    {strcpy(buf, (const char *)USB_OHCI_PORT_1);
    }
    else if(strstr(target_string, (const char *)USB_OHCI_PORT_2))
    {strcpy(buf, (const char *)USB_OHCI_PORT_2);
    }
//#ifdef BCM_MMC
    else if(strstr(target_string, SDCARD_PORT))
    { strcpy(buf, SDCARD_PORT);
    }
//#endif
//#if defined(RTCONFIG_M2_SSD)
    else if(strstr(target_string, (const char *)M2_SSD_PORT))
    {strcpy(buf, M2_SSD_PORT);
    }
//#endif
    else if(strstr(target_string, (const char *)USB_EHCI_PORT_3))
    { strcpy(buf, (const char *)USB_EHCI_PORT_3);
    }
    else if(strstr(target_string, (const char *)USB_OHCI_PORT_3))
    {strcpy(buf, (const char *)USB_OHCI_PORT_3);
    }
    else{
        return NULL;
    }

    return buf;
}

char *get_usb_node_by_string(const char *target_string, char *ret, const int ret_size)
{
    char usb_port[32], buf[16];
    char *ptr, *ptr2, *ptr3;
    int len;

//#if defined(RTCONFIG_M2_SSD)
    if (isM2SSDDevice(target_string)) {
        snprintf(ret,ret_size,"%s",M2_SSD_PORT);
        return ret;
    }
//#endif
    memset(usb_port, 0, sizeof(usb_port));
    if(get_usb_port_by_string(target_string, usb_port, sizeof(usb_port)) == NULL)
        return NULL;
    if((ptr = strstr(target_string, usb_port)) == NULL)
        return NULL;
    if(ptr != target_string)
        ptr += strlen(usb_port)+1;
    if((ptr2 = strchr(ptr, ':')) == NULL)
        return NULL;
    ptr3 = ptr2;
    *ptr3 = 0;
    if((ptr2 = strrchr(ptr, '/')) == NULL)
        ptr2 = ptr;
    else
        ptr = ptr2+1;
    len = strlen(ptr);
    if(len > 16)
        len = 16;
    memset(buf, 0, sizeof(buf));
    strncpy(buf, ptr, len);
    len = strlen(buf);
    if(len > ret_size)
        len = ret_size;
    memset(ret, 0, ret_size);
    strncpy(ret, buf, len);
    *ptr3 = ':';
    return ret;
}

char *get_usb_node_by_device(const char *device_name, char *buf, const int buf_size)
{
    int device_type = get_device_type_by_device(device_name);
    char device_path[128], usb_path[PATH_MAX];
    char disk_name[16];
    if(device_type == DEVICE_TYPE_UNKNOWN)
        return NULL;
    memset(device_path, 0, 128);
    memset(usb_path, 0, PATH_MAX);
    if(device_type == DEVICE_TYPE_DISK){
        get_disk_name(device_name, disk_name, 16);
        sprintf(device_path, "%s/%s/device", SYS_BLOCK, disk_name);
        if(realpath(device_path, usb_path) == NULL)
            return NULL;
    }
    else
//#ifdef RTCONFIG_USB_PRINTER
    if(device_type == DEVICE_TYPE_PRINTER){
        sprintf(device_path, "%s/%s/device", SYS_USB, device_name);
        if(realpath(device_path, usb_path) == NULL){
            return NULL;
        }
    }
    else
//#endif
//#ifdef RTCONFIG_USB_MODEM
    if(device_type == DEVICE_TYPE_SG){
        sprintf(device_path, "%s/%s/device", SYS_SG, device_name);
        if(realpath(device_path, usb_path) == NULL){
            return NULL;
        }
    }
    else
    if(device_type == DEVICE_TYPE_CD){
        sprintf(device_path, "%s/%s/device", SYS_BLOCK, device_name);
        if(realpath(device_path, usb_path) == NULL){
            return NULL;
        }
    }
    else
    if(device_type == DEVICE_TYPE_MODEM){
        sprintf(device_path, "%s/%s/device", SYS_TTY, device_name);
        if(realpath(device_path, usb_path) == NULL){
            sleep(1); // Sometimes link would be built slowly, so try again.
            if(realpath(device_path, usb_path) == NULL){
                return NULL;
            }
        }
    }
    else
//#endif
//#ifdef RTCONFIG_USB_BECEEM
    if(device_type == DEVICE_TYPE_BECEEM){
        sprintf(device_path, "%s/%s/device", SYS_USB, device_name);
        if(realpath(device_path, usb_path) == NULL){
            if(realpath(device_path, usb_path) == NULL){
                return NULL;
            }
        }
    }
    else
//#endif
        return NULL;
//#ifdef BCM_MMC
    if(isMMCDevice(device_name)){ // SD card.
        snprintf(buf, buf_size, "%s", SDCARD_PORT);
    }
    else
//#endif
//#if defined(RTCONFIG_M2_SSD)
    /* M.2 SATA SSD */
    if (isM2SSDDevice(device_name)) {
        snprintf(buf, buf_size, "%s", M2_SSD_PORT);
        }
    else
//#endif
    if(get_usb_node_by_string(usb_path, buf, buf_size) == NULL){
        return NULL;
    }
    return buf;
}


char *get_path_by_node(const char *usb_node, char *buf, const int buf_size){
    char usb_port[32], *hub_path;
    int port_num = 0, len;
    if(usb_node == NULL || buf == NULL || buf_size <= 0){
        return NULL;
    }
    // Get USB port.
    if(get_usb_port_by_string(usb_node, usb_port, sizeof(usb_port)) == NULL){
        return NULL;
    }
    port_num = get_usb_port_number(usb_port);
    if(port_num == 0){
        return NULL;
    }
    if(strlen(usb_node) > (len = strlen(usb_port))){
        hub_path = (char *)usb_node+len;
        snprintf(buf, buf_size, "%d%s", port_num, hub_path);
    }
    else
        snprintf(buf, buf_size, "%d", port_num);
    return buf;
}

// 20170509 leo added for hub }
#ifdef DM_I686
char jqs_command[1024];
#endif
#if defined(HAVE_SIGACTION) && defined(SA_SIGINFO)
static volatile siginfo_t last_sigterm_info;
static volatile siginfo_t last_sighup_info;

static void sigaction_handler(int sig, siginfo_t *si, void *context) {
    static siginfo_t empty_siginfo;
    UNUSED(context);

    if (!si) si = &empty_siginfo;

    switch (sig) {
    case SIGTERM:
        //srv_shutdown = 1;
        srv_save_shutdown = 1;
        last_sigterm_info = *si;
        //run_offswap(); //2011.11.28 magic added
        break;
    case SIGUSR2:		  //2011.07.27 magic added
        //allow_download = check_download_time();
        //20120821 magic modify{
        //getbasepath();
        if (access("/tmp/have_dm2",0) == 0){
            getdmconfig();
            //20120821 magic modify}
            allow_download=timecheck_item(nv_data,nv_time1,nv_time2);  //2012.07.10 magic added for cross-night
        }
        break;
    case SIGUSR1:
        if (access("/tmp/have_dm2",0) == 0){
            while(1)
            {
                if((access("/tmp/getdiskinfo_lock",F_OK) != 0) && (access("/tmp/usbinfo_lock",F_OK) == 0))//have no process to read and write
                {
                    unlink("/tmp/usbinfo_lock");
                    //system("/tmp/APPS/DM2/Script/dm2_usbget start");
                    /////////// {
                    system("echo \"\`mount |grep \"/dev/sd\" \| awk \'{print $1}\'| awk \'BEGIN{FS=\"/\"}{print $3}\'\`\" >/tmp/usb_hd_num");
                    memset(port_path_b,0,sizeof(port_path_b));
                    memset(usb_node_b,0,sizeof(usb_node_b));
                    fp=fopen("/tmp/usb_hd_num","r");
                    memset(hd_n,0,sizeof(hd_n));
                    memset(hd_n_t,0,sizeof(hd_n_t));
                    if(fp == NULL){
                        fprintf(stderr,"\nasuslighttpd: usb_hd_num not exist\n");
                    }
                    else {
                        while(fgets(hd_n,sizeof(hd_n),fp) != NULL){
                            hd_n[3]='\0';
                            if(strcmp(hd_n,hd_n_t) == 0){
                                continue;
                            }
                            if(get_usb_node_by_device(hd_n, usb_node_b, 32) == NULL){
                                fprintf(stderr,"\nasuslighttpd: get usb node error\n");
                            }

                            if(get_path_by_node(usb_node_b, port_path_b, 8) == NULL){
                                fprintf(stderr,"\nasuslighttpd: get usb path error\n");
                            }
                            snprintf(hd_n_t,sizeof(hd_n_t),"%s",hd_n);
                            memset(usb_get,0,sizeof(usb_get));
                            if(access("/tmp/APPS/DM2/Script/dm2_usbget",0) == 0)
                            snprintf(usb_get,sizeof(usb_get),"/tmp/APPS/DM2/Script/dm2_usbget start %s %d",port_path_b,path_num);
                            else
                                snprintf(usb_get,sizeof(usb_get),"/opt/etc/asus_script/dm2_usbget start %s %d",port_path_b,path_num);
                            system(usb_get);
                            path_num++;
                        }
                        path_num=1;

                    }
                    fclose(fp);
                    ////////////}
                    break;
                }
            }
        }
        break;
	case SIGINT:
        if (graceful_shutdown) {
            srv_shutdown = 1;
        } else {
            graceful_shutdown = 1;
        }
        last_sigterm_info = *si;
        //run_offswap(); //2011.11.28 magic added
        break;
	case SIGALRM: 
            handle_sig_alarm = 1;
            break;
	case SIGHUP:
            /**
		 * we send the SIGHUP to all procs in the process-group
		 * this includes ourself
		 * 
		 * make sure we only send it once and don't create a 
		 * infinite loop
		 */
            if (!forwarded_sig_hup) {
                handle_sig_hup = 1;
                last_sighup_info = *si;
            } else {
                forwarded_sig_hup = 0;
            }
            break;
	case SIGCHLD:
            break;
	}
}
#elif defined(HAVE_SIGNAL) || defined(HAVE_SIGACTION)
static void signal_handler(int sig) {
    switch (sig) {
	//case SIGTERM: srv_shutdown = 1; break;
    case SIGTERM: srv_save_shutdown = 1; break;
    case SIGINT:
        if (graceful_shutdown) srv_shutdown = 1;
        else graceful_shutdown = 1;

        break;
    case SIGALRM: handle_sig_alarm = 1; break;
    case SIGHUP:  handle_sig_hup = 1; break;
	//case SIGUSR2:   allow_download = check_download_time(); break;
	//case SIGUSR2:  getbasepath(); allow_download=timecheck_item(nv_data,nv_time1,nv_time2); break;//2012.07.10 magic added for cross-night
    case SIGUSR2:  if (access("/tmp/have_dm2",0) == 0){ getdmconfig(); allow_download=timecheck_item(nv_data,nv_time1,nv_time2); }break;//20120821 magic modify for new config
        case SIGUSR1:
        if (access("/tmp/have_dm2",0) == 0){
            while(1)
            {
                if((access("/tmp/getdiskinfo_lock",F_OK) != 0) && (access("/tmp/usbinfo_lock",F_OK) == 0))//have no process to read and write
                {
                    unlink("/tmp/usbinfo_lock");
                    /////////// {
                    system("echo \"\`mount |grep \"/dev/sd\" \| awk \'{print $1}\'| awk \'BEGIN{FS=\"/\"}{print $3}\'\`\" >/tmp/usb_hd_num");
                    memset(port_path_b,0,sizeof(port_path_b));
                    memset(usb_node_b,0,sizeof(usb_node_b));
                    fp=fopen("/tmp/usb_hd_num","r");
                    memset(hd_n,0,sizeof(hd_n));
                    memset(hd_n_t,0,sizeof(hd_n_t));
                    if(fp == NULL){
                        fprintf(stderr,"\nasuslighttpd: usb_hd_num not exist\n");
                    }
                    else {
                        while(fgets(hd_n,sizeof(hd_n),fp) != NULL){
                            hd_n[3]='\0';
                            if(strcmp(hd_n,hd_n_t) == 0){
                                continue;
                            }
                            if(get_usb_node_by_device(hd_n, usb_node_b, 32) == NULL){
                                fprintf(stderr,"\nasuslighttpd: get usb node error\n");
                            }

                            if(get_path_by_node(usb_node_b, port_path_b, 8) == NULL){
                                fprintf(stderr,"\nasuslighttpd: get usb path error\n");
                            }
                            snprintf(hd_n_t,sizeof(hd_n_t),"%s",hd_n);
                            memset(usb_get,0,sizeof(usb_get));
                            if(access("/tmp/APPS/DM2/Script/dm2_usbget",0) == 0)
                            snprintf(usb_get,sizeof(usb_get),"/tmp/APPS/DM2/Script/dm2_usbget start %s %d",port_path_b,path_num);
                            else
                                snprintf(usb_get,sizeof(usb_get),"/opt/etc/asus_script/dm2_usbget start %s %d",port_path_b,path_num);
                            system(usb_get);
                            path_num++;
                        }
                        path_num=1;

                    }
                    fclose(fp);
                    ////////////}
                    break;
                }
            }
        }
        break;
	case SIGCHLD:  break;
	}
}
#endif

#ifdef HAVE_FORK
static void daemonize(void) {
#ifdef SIGTTOU
    signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
    signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif
    if (0 != fork()) exit(0);

    if (-1 == setsid()) exit(0);

    signal(SIGHUP, SIG_IGN);

    if (0 != fork()) exit(0);

    if (0 != chdir("/")) exit(0);
}
#endif

static server *server_init(void) {
    int i;
    FILE *frandom = NULL;

    server *srv = calloc(1, sizeof(*srv));
    force_assert(srv);
#define CLEAN(x) \
    srv->x = buffer_init();

    CLEAN(response_header);
    CLEAN(parse_full_path);
    CLEAN(ts_debug_str);
    CLEAN(ts_date_str);
    CLEAN(errorlog_buf);
    CLEAN(response_range);
    CLEAN(tmp_buf);
    srv->empty_string = buffer_init_string("");
    CLEAN(cond_check_buf);

    CLEAN(srvconf.errorlog_file);
    CLEAN(srvconf.breakagelog_file);
    CLEAN(srvconf.groupname);
    CLEAN(srvconf.username);
    CLEAN(srvconf.changeroot);
    CLEAN(srvconf.bindhost);
    CLEAN(srvconf.event_handler);
    CLEAN(srvconf.pid_file);

    CLEAN(tmp_chunk_len);
#undef CLEAN

#define CLEAN(x) \
    srv->x = array_init();

    CLEAN(config_context);
    CLEAN(config_touched);
    CLEAN(status);
#undef CLEAN

    for (i = 0; i < FILE_CACHE_MAX; i++) {
        srv->mtime_cache[i].mtime = (time_t)-1;
        srv->mtime_cache[i].str = buffer_init();
    }

    if ((NULL != (frandom = fopen("/dev/urandom", "rb")) || NULL != (frandom = fopen("/dev/random", "rb")))
        && 1 == fread(srv->entropy, sizeof(srv->entropy), 1, frandom)) {
        unsigned int e;
        memcpy(&e, srv->entropy, sizeof(e) < sizeof(srv->entropy) ? sizeof(e) : sizeof(srv->entropy));
        srand(e);
        srv->is_real_entropy = 1;
    } else {
        unsigned int j;
        srand(time(NULL) ^ getpid());
        srv->is_real_entropy = 0;
        for (j = 0; j < sizeof(srv->entropy); j++)
            srv->entropy[j] = rand();
    }
    if (frandom) fclose(frandom);

    srv->cur_ts = time(NULL);
    srv->startup_ts = srv->cur_ts;

    srv->conns = calloc(1, sizeof(*srv->conns));
    force_assert(srv->conns);

    srv->joblist = calloc(1, sizeof(*srv->joblist));
    force_assert(srv->joblist);

    srv->fdwaitqueue = calloc(1, sizeof(*srv->fdwaitqueue));
    force_assert(srv->fdwaitqueue);

    srv->srvconf.modules = array_init();
    srv->srvconf.modules_dir = buffer_init_string(LIBRARY_DIR);
    srv->srvconf.network_backend = buffer_init();
    srv->srvconf.upload_tempdirs = array_init();
    srv->srvconf.reject_expect_100_with_417 = 1;

    /* use syslog */
    srv->errorlog_fd = STDERR_FILENO;
    srv->errorlog_mode = ERRORLOG_FD;

    srv->split_vals = array_init();

    return srv;
}

static void server_free(server *srv) {
    size_t i;

    for (i = 0; i < FILE_CACHE_MAX; i++) {
        buffer_free(srv->mtime_cache[i].str);
    }

#define CLEAN(x) \
    buffer_free(srv->x);

    CLEAN(response_header);
    CLEAN(parse_full_path);
    CLEAN(ts_debug_str);
    CLEAN(ts_date_str);
    CLEAN(errorlog_buf);
    CLEAN(response_range);
    CLEAN(tmp_buf);
    CLEAN(empty_string);
    CLEAN(cond_check_buf);

    CLEAN(srvconf.errorlog_file);
    CLEAN(srvconf.breakagelog_file);
    CLEAN(srvconf.groupname);
    CLEAN(srvconf.username);
    CLEAN(srvconf.changeroot);
    CLEAN(srvconf.bindhost);
    CLEAN(srvconf.event_handler);
    CLEAN(srvconf.pid_file);
    CLEAN(srvconf.modules_dir);
    CLEAN(srvconf.network_backend);

    CLEAN(tmp_chunk_len);
#undef CLEAN

#if 0
    fdevent_unregister(srv->ev, srv->fd);
#endif
    fdevent_free(srv->ev);

    free(srv->conns);

    if (srv->config_storage) {
        for (i = 0; i < srv->config_context->used; i++) {
            specific_config *s = srv->config_storage[i];

            if (!s) continue;

            buffer_free(s->document_root);
            buffer_free(s->server_name);
            buffer_free(s->server_tag);
            buffer_free(s->ssl_pemfile);
            buffer_free(s->ssl_ca_file);
            buffer_free(s->ssl_cipher_list);
            buffer_free(s->ssl_dh_file);
            buffer_free(s->ssl_ec_curve);
            buffer_free(s->error_handler);
            buffer_free(s->errorfile_prefix);
            array_free(s->mimetypes);
            buffer_free(s->ssl_verifyclient_username);
#ifdef USE_OPENSSL
            SSL_CTX_free(s->ssl_ctx);
            EVP_PKEY_free(s->ssl_pemfile_pkey);
            X509_free(s->ssl_pemfile_x509);
            if (NULL != s->ssl_ca_file_cert_names) sk_X509_NAME_pop_free(s->ssl_ca_file_cert_names, X509_NAME_free);
#endif
            free(s);
        }
        free(srv->config_storage);
        srv->config_storage = NULL;
    }

#define CLEAN(x) \
    array_free(srv->x);

    CLEAN(config_context);
    CLEAN(config_touched);
    CLEAN(status);
    CLEAN(srvconf.upload_tempdirs);
#undef CLEAN

    joblist_free(srv, srv->joblist);
    fdwaitqueue_free(srv, srv->fdwaitqueue);

    if (srv->stat_cache) {
        stat_cache_free(srv->stat_cache);
    }

    array_free(srv->srvconf.modules);
    array_free(srv->split_vals);

#ifdef USE_OPENSSL
    if (srv->ssl_is_init) {
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();
        ERR_remove_state(0);
        EVP_cleanup();
    }
#endif

    free(srv);
}

static void show_version (void) {
#ifdef USE_OPENSSL
# define TEXT_SSL " (ssl)"
#else
# define TEXT_SSL
#endif
    char *b = PACKAGE_DESC TEXT_SSL \
              " - a light and fast webserver\n" \
              "Build-Date: " __DATE__ " " __TIME__ "\n";
    ;
#undef TEXT_SSL
    write_all(STDOUT_FILENO, b, strlen(b));
}

static void show_features (void) {
    const char features[] = ""
#ifdef USE_SELECT
                            "\t+ select (generic)\n"
#else
                            "\t- select (generic)\n"
#endif
#ifdef USE_POLL
                            "\t+ poll (Unix)\n"
#else
                            "\t- poll (Unix)\n"
#endif
#ifdef USE_LINUX_SIGIO
                            "\t+ rt-signals (Linux 2.4+)\n"
#else
                            "\t- rt-signals (Linux 2.4+)\n"
#endif
#ifdef USE_LINUX_EPOLL
                            "\t+ epoll (Linux 2.6)\n"
#else
                            "\t- epoll (Linux 2.6)\n"
#endif
#ifdef USE_SOLARIS_DEVPOLL
                            "\t+ /dev/poll (Solaris)\n"
#else
                            "\t- /dev/poll (Solaris)\n"
#endif
#ifdef USE_SOLARIS_PORT
                            "\t+ eventports (Solaris)\n"
#else
                            "\t- eventports (Solaris)\n"
#endif
#ifdef USE_FREEBSD_KQUEUE
                            "\t+ kqueue (FreeBSD)\n"
#else
                            "\t- kqueue (FreeBSD)\n"
#endif
#ifdef USE_LIBEV
                            "\t+ libev (generic)\n"
#else
                            "\t- libev (generic)\n"
#endif
                            "\nNetwork handler:\n\n"
#if defined USE_LINUX_SENDFILE
                            "\t+ linux-sendfile\n"
#else
                            "\t- linux-sendfile\n"
#endif
#if defined USE_FREEBSD_SENDFILE
                            "\t+ freebsd-sendfile\n"
#else
                            "\t- freebsd-sendfile\n"
#endif
#if defined USE_DARWIN_SENDFILE
                            "\t+ darwin-sendfile\n"
#else
                            "\t- darwin-sendfile\n"
#endif
#if defined USE_SOLARIS_SENDFILEV
                            "\t+ solaris-sendfilev\n"
#else
                            "\t- solaris-sendfilev\n"
#endif
#if defined USE_WRITEV
                            "\t+ writev\n"
#else
                            "\t- writev\n"
#endif
                            "\t+ write\n"
#ifdef USE_MMAP
                            "\t+ mmap support\n"
#else
                            "\t- mmap support\n"
#endif
                            "\nFeatures:\n\n"
#ifdef HAVE_IPV6
                            "\t+ IPv6 support\n"
#else
                            "\t- IPv6 support\n"
#endif
#if defined HAVE_ZLIB_H && defined HAVE_LIBZ
                            "\t+ zlib support\n"
#else
                            "\t- zlib support\n"
#endif
#if defined HAVE_BZLIB_H && defined HAVE_LIBBZ2
                            "\t+ bzip2 support\n"
#else
                            "\t- bzip2 support\n"
#endif
#if defined(HAVE_CRYPT) || defined(HAVE_CRYPT_R) || defined(HAVE_LIBCRYPT)
                            "\t+ crypt support\n"
#else
                            "\t- crypt support\n"
#endif
#ifdef USE_OPENSSL
                            "\t+ SSL Support\n"
#else
                            "\t- SSL Support\n"
#endif
#ifdef HAVE_LIBPCRE
                            "\t+ PCRE support\n"
#else
                            "\t- PCRE support\n"
#endif
#ifdef HAVE_MYSQL
                            "\t+ mySQL support\n"
#else
                            "\t- mySQL support\n"
#endif
#if defined(HAVE_LDAP_H) && defined(HAVE_LBER_H) && defined(HAVE_LIBLDAP) && defined(HAVE_LIBLBER)
                            "\t+ LDAP support\n"
#else
                            "\t- LDAP support\n"
#endif
#ifdef HAVE_MEMCACHE_H
                            "\t+ memcached support\n"
#else
                            "\t- memcached support\n"
#endif
#ifdef HAVE_FAM_H
                            "\t+ FAM support\n"
#else
                            "\t- FAM support\n"
#endif
#ifdef HAVE_LUA_H
                            "\t+ LUA support\n"
#else
                            "\t- LUA support\n"
#endif
#ifdef HAVE_LIBXML_H
                            "\t+ xml support\n"
#else
                            "\t- xml support\n"
#endif
#ifdef HAVE_SQLITE3_H
                            "\t+ SQLite support\n"
#else
                            "\t- SQLite support\n"
#endif
#ifdef HAVE_GDBM_H
                            "\t+ GDBM support\n"
#else
                            "\t- GDBM support\n"
#endif
                            "\n";
    show_version();
    printf("\nEvent Handlers:\n\n%s", features);
}

static void show_help (void) {
#ifdef USE_OPENSSL
# define TEXT_SSL " (ssl)"
#else
# define TEXT_SSL
#endif
    char *b = PACKAGE_DESC TEXT_SSL " ("__DATE__ " " __TIME__ ")" \
              " - a light and fast webserver\n" \
              "usage:\n" \
              " -f <name>  filename of the config-file\n" \
              " -m <name>  module directory (default: "LIBRARY_DIR")\n" \
              " -p         print the parsed config-file in internal form, and exit\n" \
              " -t         test the config-file, and exit\n" \
              " -D         don't go to background (default: go to background)\n" \
              " -v         show version\n" \
              " -V         show compile-time features\n" \
              " -h         show this help\n" \
              "\n"
              ;
#undef TEXT_SSL
#undef TEXT_IPV6
    write_all(STDOUT_FILENO, b, strlen(b));
}

int main (int argc, char **argv) {
    server *srv = NULL;
    int print_config = 0;
    int test_config = 0;
    int i_am_root;
    int o;
    int num_childs = 0;
    int pid_fd = -1, fd;
    size_t i;
    int flag=0;
#ifdef HAVE_SIGACTION
    struct sigaction act;
#endif
#ifdef HAVE_GETRLIMIT
    struct rlimit rlim;
#endif

#ifdef USE_ALARM
    struct itimerval interval;

    interval.it_interval.tv_sec = 1;
    interval.it_interval.tv_usec = 0;
    interval.it_value.tv_sec = 1;
    interval.it_value.tv_usec = 0;
#endif


    /* for nice %b handling in strfime() */
    setlocale(LC_TIME, "C");

    if (NULL == (srv = server_init())) {
        //fprintf(stderr, "did this really happen?\n");
        return -1;
    }

    /* init structs done */

    srv->srvconf.port = 0;
#ifdef HAVE_GETUID
    i_am_root = (getuid() == 0);
#else
    i_am_root = 0;
#endif
    srv->srvconf.dont_daemonize = 0;

    while(-1 != (o = getopt(argc, argv, "f:m:hvVDpt"))) {
        switch(o) {
        case 'f':
            if (srv->config_storage) {
                log_error_write(srv, __FILE__, __LINE__, "s",
                                "Can only read one config file. Use the include command to use multiple config files.");

                server_free(srv);
                return -1;
            }
            if (config_read(srv, optarg)) {
                server_free(srv);
                return -1;
            }
            break;
		case 'm':
            buffer_copy_string(srv->srvconf.modules_dir, optarg);
            break;
		case 'p': print_config = 1; break;
		case 't': test_config = 1; break;
		case 'D': srv->srvconf.dont_daemonize = 1; break;
		case 'v': show_version(); return 0;
		case 'V': show_features(); return 0;
		case 'h': show_help(); return 0;
		default:
                    show_help();
                    server_free(srv);
                    return -1;
		}
    }
    FILE *fp_usb;
    if(access("/tmp/notify",0)!=0)
    {
        mkdir("/tmp/notify",0777);
    }
    if(access("/tmp/notify/usb",0)!=0)
    {
        mkdir("/tmp/notify/usb",0777);
    }
    fp_usb = fopen("/tmp/notify/usb/asus_lighttpd","w");
    if(fp_usb!=NULL){
        fprintf(fp_usb,"%d",SIGUSR1);
        fclose(fp_usb);
    }
    //if (access("/tmp/have_dm2",0) == 0){
        /////////// {
        system("echo \"\`mount \|grep \"/dev/sd\" \| awk \'{print $1}\'\| awk \'BEGIN{FS=\"/\"}{print $3}\'\`\" >/tmp/usb_hd_num");
#ifdef DM_MIPSBIG
        if(access("/userfs/bin/tcapi",0) == 0){
            dm_mount=(char *)malloc(10);
            dm_mount=tcapi_get_by_popen("Apps_Entry","apps_dev");
        }
        else {
            dm_mount=(char *)malloc(10);
            dm_mount=nvram_get_by_popen("apps_dev");
        }
#else
        dm_mount=(char *)malloc(10);
        dm_mount=nvram_get_by_popen("apps_dev");
#endif
        memset(dm_mount_path,0,sizeof(dm_mount_path));
        memset(dm_mount_node,0,sizeof(dm_mount_node));
        if(get_usb_node_by_device(dm_mount,dm_mount_node, 32) == NULL){
            fprintf(stderr,"\ndm: get mount hd usb_node error\n");
        }
        if(get_path_by_node(dm_mount_node, dm_mount_path, 8) == NULL){
           fprintf(stderr,"\ndm: get mount hd path error\n");
        }
        free(dm_mount);
        dm_mount=NULL;
        memset(dm_mount_tmp,0,sizeof(dm_mount_tmp));
        snprintf(dm_mount_tmp,sizeof(dm_mount_tmp),"echo dm_mount_path=%s >/tmp/dm_mount_path_n",dm_mount_path);
        system(dm_mount_tmp);
        memset(port_path_b,0,sizeof(port_path_b));
        memset(usb_node_b,0,sizeof(usb_node_b));
        fp=fopen("/tmp/usb_hd_num","r");
        memset(hd_n,0,sizeof(hd_n));
        memset(hd_n_t,0,sizeof(hd_n_t));
        if(fp == NULL){
            fprintf(stderr,"\nasuslighttpd: usb_hd_num not exist\n");
        }
        else {
            while(fgets(hd_n,sizeof(hd_n),fp) != NULL){
                hd_n[3]='\0';
                if(strcmp(hd_n,hd_n_t) == 0){
                    continue;
                }
                if(get_usb_node_by_device(hd_n, usb_node_b, 32) == NULL){
                    fprintf(stderr,"\nasuslighttpd: get usb node error\n");

                }

                if(get_path_by_node(usb_node_b, port_path_b, 8) == NULL){
                    fprintf(stderr,"\nasuslighttpd: get usb path error\n");

                }
                snprintf(hd_n_t,sizeof(hd_n_t),"%s",hd_n);
                memset(usb_get,0,sizeof(usb_get));
                if(access("/tmp/APPS/DM2/Script/dm2_usbget",0) == 0)
                snprintf(usb_get,sizeof(usb_get),"/tmp/APPS/DM2/Script/dm2_usbget start %s %d",port_path_b,path_num);
                else
                    snprintf(usb_get,sizeof(usb_get),"/opt/etc/asus_script/dm2_usbget start %s %d",port_path_b,path_num);
                system(usb_get);
                path_num++;
            }
            path_num=1;

        }
        fclose(fp);
        ////////////}
   // }

    if (!srv->config_storage) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "No configuration available. Try using -f option.");

        server_free(srv);
        return -1;
    }

    if (print_config) {
        data_unset *dc = srv->config_context->data[0];
        if (dc) {
            dc->print(dc, 0);
            fprintf(stdout, "\n");
        } else {
            /* shouldn't happend */
            //fprintf(stderr, "global config not found\n");
        }
    }

    if (test_config) {
        printf("Syntax OK\n");
    }

    if (test_config || print_config) {
        server_free(srv);
        return 0;
    }

    /* close stdin and stdout, as they are not needed */
    openDevNull(STDIN_FILENO);
    openDevNull(STDOUT_FILENO);

    if (0 != config_set_defaults(srv)) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "setting default values failed");
        server_free(srv);
        return -1;
    }

    /* UID handling */
#ifdef HAVE_GETUID
    if (!i_am_root && issetugid()) {
        /* we are setuid-root */

        log_error_write(srv, __FILE__, __LINE__, "s",
                        "Are you nuts ? Don't apply a SUID bit to this binary");

        server_free(srv);
        return -1;
    }
#endif

    /* check document-root */
    if (buffer_string_is_empty(srv->config_storage[0]->document_root)) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "document-root is not set\n");

        server_free(srv);

        return -1;
    }

    if (plugins_load(srv)) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "loading plugins finally failed");

        plugins_free(srv);
        server_free(srv);

        return -1;
    }

    /* open pid file BEFORE chroot */
    if (!buffer_string_is_empty(srv->srvconf.pid_file)) {
        if (-1 == (pid_fd = open(srv->srvconf.pid_file->ptr, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
            struct stat st;
            if (errno != EEXIST) {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                                "opening pid-file failed:", srv->srvconf.pid_file, strerror(errno));
                return -1;
            }

            if (0 != stat(srv->srvconf.pid_file->ptr, &st)) {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                                "stating existing pid-file failed:", srv->srvconf.pid_file, strerror(errno));
            }

            if (!S_ISREG(st.st_mode)) {
                log_error_write(srv, __FILE__, __LINE__, "sb",
                                "pid-file exists and isn't regular file:", srv->srvconf.pid_file);
                return -1;
            }

            if (-1 == (pid_fd = open(srv->srvconf.pid_file->ptr, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                                "opening pid-file failed:", srv->srvconf.pid_file, strerror(errno));
                return -1;
            }
        }
    }

    if (srv->event_handler == FDEVENT_HANDLER_SELECT) {
        /* select limits itself
		 *
		 * as it is a hard limit and will lead to a segfault we add some safety
		 * */
        srv->max_fds = FD_SETSIZE - 200;
    } else {
        srv->max_fds = 4096;
    }

    if (i_am_root) {
        struct group *grp = NULL;
        struct passwd *pwd = NULL;
        int use_rlimit = 1;

#ifdef HAVE_VALGRIND_VALGRIND_H
        if (RUNNING_ON_VALGRIND) use_rlimit = 0;
#endif

#ifdef HAVE_GETRLIMIT
        if (0 != getrlimit(RLIMIT_NOFILE, &rlim)) {
            log_error_write(srv, __FILE__, __LINE__,
                            "ss", "couldn't get 'max filedescriptors'",
                            strerror(errno));
            return -1;
        }

        if (use_rlimit && srv->srvconf.max_fds) {
            /* set rlimits */

            rlim.rlim_cur = srv->srvconf.max_fds;
            rlim.rlim_max = srv->srvconf.max_fds;

            if (0 != setrlimit(RLIMIT_NOFILE, &rlim)) {
                log_error_write(srv, __FILE__, __LINE__,
                                "ss", "couldn't set 'max filedescriptors'",
                                strerror(errno));
                return -1;
            }
        }

        if (srv->event_handler == FDEVENT_HANDLER_SELECT) {
            srv->max_fds = rlim.rlim_cur < ((int)FD_SETSIZE) - 200 ? rlim.rlim_cur : FD_SETSIZE - 200;
        } else {
            srv->max_fds = rlim.rlim_cur;
        }

        /* set core file rlimit, if enable_cores is set */
        if (use_rlimit && srv->srvconf.enable_cores && getrlimit(RLIMIT_CORE, &rlim) == 0) {
            rlim.rlim_cur = rlim.rlim_max;
            setrlimit(RLIMIT_CORE, &rlim);
        }
#endif
        if (srv->event_handler == FDEVENT_HANDLER_SELECT) {
            /* don't raise the limit above FD_SET_SIZE */
            if (srv->max_fds > ((int)FD_SETSIZE) - 200) {
                log_error_write(srv, __FILE__, __LINE__, "sd",
                                "can't raise max filedescriptors above",  FD_SETSIZE - 200,
                                "if event-handler is 'select'. Use 'poll' or something else or reduce server.max-fds.");
                return -1;
            }
        }


#ifdef HAVE_PWD_H
        /* set user and group */
        /*if (!buffer_string_is_empty(srv->srvconf.username)) {
			if (NULL == (pwd = getpwnam(srv->srvconf.username->ptr))) {
				log_error_write(srv, __FILE__, __LINE__, "sb",
						"can't find username", srv->srvconf.username);
				return -1;
			}

			if (pwd->pw_uid == 0) {
				log_error_write(srv, __FILE__, __LINE__, "s",
						"I will not set uid to 0\n");
				return -1;
			}
		}

		if (!buffer_string_is_empty(srv->srvconf.groupname)) {
			if (NULL == (grp = getgrnam(srv->srvconf.groupname->ptr))) {
				log_error_write(srv, __FILE__, __LINE__, "sb",
					"can't find groupname", srv->srvconf.groupname);
				return -1;
			}
			if (grp->gr_gid == 0) {
				log_error_write(srv, __FILE__, __LINE__, "s",
						"I will not set gid to 0\n");
				return -1;
			}
		}*/
#endif
        /* we need root-perms for port < 1024 */
        if (0 != network_init(srv)) {
            plugins_free(srv);
            server_free(srv);

            return -1;
        }
#ifdef HAVE_PWD_H
        /*
		 * Change group before chroot, when we have access
		 * to /etc/group
		 * */
        if (NULL != grp) {
            if (-1 == setgid(grp->gr_gid)) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "setgid failed: ", strerror(errno));
                return -1;
            }
            if (-1 == setgroups(0, NULL)) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "setgroups failed: ", strerror(errno));
                return -1;
            }
            if (!buffer_string_is_empty(srv->srvconf.username)) {
                initgroups(srv->srvconf.username->ptr, grp->gr_gid);
            }
        }
#endif
#ifdef HAVE_CHROOT
        if (!buffer_string_is_empty(srv->srvconf.changeroot)) {
            tzset();

            if (-1 == chroot(srv->srvconf.changeroot->ptr)) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "chroot failed: ", strerror(errno));
                return -1;
            }
            if (-1 == chdir("/")) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "chdir failed: ", strerror(errno));
                return -1;
            }
        }
#endif
#ifdef HAVE_PWD_H
        /* drop root privs */
        if (NULL != pwd) {
            if (-1 == setuid(pwd->pw_uid)) {
                log_error_write(srv, __FILE__, __LINE__, "ss", "setuid failed: ", strerror(errno));
                return -1;
            }
        }
#endif
#if defined(HAVE_SYS_PRCTL_H) && defined(PR_SET_DUMPABLE)
        /**
		 * on IRIX 6.5.30 they have prctl() but no DUMPABLE
		 */
        if (srv->srvconf.enable_cores) {
            prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
        }
#endif
    } else {

#ifdef HAVE_GETRLIMIT
        if (0 != getrlimit(RLIMIT_NOFILE, &rlim)) {
            log_error_write(srv, __FILE__, __LINE__,
                            "ss", "couldn't get 'max filedescriptors'",
                            strerror(errno));
            return -1;
        }

        /**
		 * we are not root can can't increase the fd-limit, but we can reduce it
		 */
        if (srv->srvconf.max_fds && srv->srvconf.max_fds < rlim.rlim_cur) {
            /* set rlimits */

            rlim.rlim_cur = srv->srvconf.max_fds;

            if (0 != setrlimit(RLIMIT_NOFILE, &rlim)) {
                log_error_write(srv, __FILE__, __LINE__,
                                "ss", "couldn't set 'max filedescriptors'",
                                strerror(errno));
                return -1;
            }
        }

        if (srv->event_handler == FDEVENT_HANDLER_SELECT) {
            srv->max_fds = rlim.rlim_cur < ((int)FD_SETSIZE) - 200 ? rlim.rlim_cur : FD_SETSIZE - 200;
        } else {
            srv->max_fds = rlim.rlim_cur;
        }

        /* set core file rlimit, if enable_cores is set */
        if (srv->srvconf.enable_cores && getrlimit(RLIMIT_CORE, &rlim) == 0) {
            rlim.rlim_cur = rlim.rlim_max;
            setrlimit(RLIMIT_CORE, &rlim);
        }

#endif
        if (srv->event_handler == FDEVENT_HANDLER_SELECT) {
            /* don't raise the limit above FD_SET_SIZE */
            if (srv->max_fds > ((int)FD_SETSIZE) - 200) {
                log_error_write(srv, __FILE__, __LINE__, "sd",
                                "can't raise max filedescriptors above",  FD_SETSIZE - 200,
                                "if event-handler is 'select'. Use 'poll' or something else or reduce server.max-fds.");
                return -1;
            }
        }

        if (0 != network_init(srv)) {
            plugins_free(srv);
            server_free(srv);

            return -1;
        }
    }

    /* set max-conns */
    if (srv->srvconf.max_conns > srv->max_fds/2) {
        /* we can't have more connections than max-fds/2 */
        log_error_write(srv, __FILE__, __LINE__, "sdd", "can't have more connections than fds/2: ", srv->srvconf.max_conns, srv->max_fds);
        srv->max_conns = srv->max_fds/2;
    } else if (srv->srvconf.max_conns) {
        /* otherwise respect the wishes of the user */
        srv->max_conns = srv->srvconf.max_conns;
    } else {
        /* or use the default: we really don't want to hit max-fds */
        srv->max_conns = srv->max_fds/3;
    }

    if (HANDLER_GO_ON != plugins_call_init(srv)) {
        log_error_write(srv, __FILE__, __LINE__, "s", "Initialization of plugins failed. Going down.");

        plugins_free(srv);
        network_close(srv);
        server_free(srv);

        return -1;
    }

#ifdef HAVE_FORK
    /* network is up, let's deamonize ourself */
    if (srv->srvconf.dont_daemonize == 0) daemonize();
#endif
#ifdef DM_MIPSBIG
    int mips_type = check_mips_type();
#endif
    check_nice();
    //2011.04.26 magic added{
    init_path();
    //2016.8.16 tina modify{
    //getdmconfig(); //20120821 magic modify for new conf
    char *dmconfig = getdmconfig();
    if(dmconfig != NULL)
        free(dmconfig);
    //}end tina
    Jqs_create();
    //struct timeval  tval;
    // init job_queues and que_file
    //head = (struct Lognote *)malloc(sizeof(struct Lognote));
    load_check_timestamp = time((time_t*)0);
    load_checkurl_timestamp = time((time_t*)0);
    load_checkbt_timestamp = time((time_t*)0);
    load_checked2k_timestamp = time((time_t*)0);
    utility_check_timestamp = time((time_t*)0);
    now = time((time_t*)0);
    on_heavy_counts = on_light_counts = total_heavy_counts = total_light_counts = 0;
    on_nntp_counts = total_nntp_counts = 0;
    //allow_download = check_download_time();
    allow_download=timecheck_item(nv_data,nv_time1,nv_time2);  //2012.07.10 magic added for cross-night
    allow_download_tmp = allow_download; //2012.07.10 magic added for cross-night
    //2011.04.26 magic added}
    char recall_tr[1024]; //2013.08.07 magic added
    //run_onswap(); //2011.11.28 magic added



#ifdef HAVE_SIGACTION
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
# if defined(SA_SIGINFO)
    act.sa_sigaction = sigaction_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
# else
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
# endif
    sigaction(SIGUSR2, &act, NULL); //2011.07.27 magic added
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGHUP,  &act, NULL);
    sigaction(SIGALRM, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);

#elif defined(HAVE_SIGNAL)
    /* ignore the SIGPIPE from sendfile() */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler); //2011.07.27 magic added
    signal(SIGALRM, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP,  signal_handler);
    signal(SIGCHLD,  signal_handler);
    signal(SIGINT,  signal_handler);
#endif

#ifdef USE_ALARM
    signal(SIGALRM, signal_handler);

    /* setup periodic timer (1 second) */
    if (setitimer(ITIMER_REAL, &interval, NULL)) {
        log_error_write(srv, __FILE__, __LINE__, "s", "setting timer failed");
        return -1;
    }

    getitimer(ITIMER_REAL, &interval);
#endif


    srv->gid = getgid();
    srv->uid = getuid();

    /* write pid file */
    if (pid_fd != -1) {
        buffer_copy_int(srv->tmp_buf, getpid());
        buffer_append_string_len(srv->tmp_buf, CONST_STR_LEN("\n"));
        if (-1 == write_all(pid_fd, CONST_BUF_LEN(srv->tmp_buf))) {
            log_error_write(srv, __FILE__, __LINE__, "ss", "Couldn't write pid file:", strerror(errno));
            close(pid_fd);
            return -1;
        }
        close(pid_fd);
        pid_fd = -1;
    }

    /* Close stderr ASAP in the child process to make sure that nothing
	 * is being written to that fd which may not be valid anymore. */
    if (-1 == log_error_open(srv)) {
        log_error_write(srv, __FILE__, __LINE__, "s", "Opening errorlog failed. Going down.");

        plugins_free(srv);
        network_close(srv);
        server_free(srv);
        return -1;
    }

    if (HANDLER_GO_ON != plugins_call_set_defaults(srv)) {
        log_error_write(srv, __FILE__, __LINE__, "s", "Configuration of plugins failed. Going down.");

        plugins_free(srv);
        network_close(srv);
        server_free(srv);

        return -1;
    }

    /* dump unused config-keys */
    for (i = 0; i < srv->config_context->used; i++) {
        array *config = ((data_config *)srv->config_context->data[i])->value;
        size_t j;

        for (j = 0; config && j < config->used; j++) {
            data_unset *du = config->data[j];

            /* all var.* is known as user defined variable */
            if (strncmp(du->key->ptr, "var.", sizeof("var.") - 1) == 0) {
                continue;
            }

            if (NULL == array_get_element(srv->config_touched, du->key->ptr)) {
                log_error_write(srv, __FILE__, __LINE__, "sbs",
                                "WARNING: unknown config-key:",
                                du->key,
                                "(ignored)");
            }
        }
    }

    if (srv->config_unsupported) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "Configuration contains unsupported keys. Going down.");
    }

    if (srv->config_deprecated) {
        log_error_write(srv, __FILE__, __LINE__, "s",
                        "Configuration contains deprecated keys. Going down.");
    }

    if (srv->config_unsupported || srv->config_deprecated) {
        plugins_free(srv);
        network_close(srv);
        server_free(srv);

        return -1;
    }


#ifdef HAVE_FORK
    /* start watcher and workers */
    num_childs = srv->srvconf.max_worker;
    if (num_childs > 0) {
        int child = 0;
        while (!child && !srv_shutdown && !graceful_shutdown) {
            if (num_childs > 0) {
                switch (fork()) {
                case -1:
                    return -1;
                case 0:
                    child = 1;
                    break;
                default:
                    num_childs--;
                    break;
                }
            } else {
                int status;

                if (-1 != wait(&status)) {
                    /**
					 * one of our workers went away 
					 */
                    num_childs++;
                } else {
                    switch (errno) {
                    case EINTR:
                        /**
						 * if we receive a SIGHUP we have to close our logs ourself as we don't 
						 * have the mainloop who can help us here
						 */
                        if (handle_sig_hup) {
                            handle_sig_hup = 0;

                            log_error_cycle(srv);

                            /**
							 * forward to all procs in the process-group
							 * 
							 * we also send it ourself
							 */
                            if (!forwarded_sig_hup) {
                                forwarded_sig_hup = 1;
                                kill(0, SIGHUP);
                            }
                        }
                        break;
					default:
                        break;
                    }
                }
            }
        }

        /**
		 * for the parent this is the exit-point 
		 */
        if (!child) {
            /**
			 * kill all children too 
			 */
            if (graceful_shutdown) {
                kill(0, SIGINT);
            } else if (srv_shutdown) {
                kill(0, SIGTERM);
            } else if (srv_save_shutdown) { //2011.07.27 magic added
                kill(0, SIGTERM);
            }

            log_error_close(srv);
            network_close(srv);
            connections_free(srv);
            plugins_free(srv);
            server_free(srv);
            return 0;
        }
    }
#endif

    if (NULL == (srv->ev = fdevent_init(srv, srv->max_fds + 1, srv->event_handler))) {
        log_error_write(srv, __FILE__, __LINE__,
                        "s", "fdevent_init failed");
        return -1;
    }

    /* libev backend overwrites our SIGCHLD handler and calls waitpid on SIGCHLD; we want our own SIGCHLD handling. */
#ifdef HAVE_SIGACTION
    sigaction(SIGCHLD, &act, NULL);
#elif defined(HAVE_SIGNAL)
    signal(SIGCHLD,  signal_handler);
#endif

    /*
	 * kqueue() is called here, select resets its internals,
	 * all server sockets get their handlers
	 *
	 * */
    if (0 != network_register_fdevents(srv)) {
        plugins_free(srv);
        network_close(srv);
        server_free(srv);

        return -1;
    }

    /* might fail if user is using fam (not gamin) and famd isn't running */
    if (NULL == (srv->stat_cache = stat_cache_init())) {
        log_error_write(srv, __FILE__, __LINE__, "s",
			"stat-cache could not be setup, dieing.");
        return -1;
    }

#ifdef HAVE_FAM_H
    /* setup FAM */
    if (srv->srvconf.stat_cache_engine == STAT_CACHE_ENGINE_FAM) {
        if (0 != FAMOpen2(&srv->stat_cache->fam, "lighttpd")) {
            log_error_write(srv, __FILE__, __LINE__, "s",
                            "could not open a fam connection, dieing.");
            return -1;
        }
#ifdef HAVE_FAMNOEXISTS
        FAMNoExists(&srv->stat_cache->fam);
#endif

        fdevent_register(srv->ev, FAMCONNECTION_GETFD(&srv->stat_cache->fam), stat_cache_handle_fdevent, NULL);
        fdevent_event_set(srv->ev, &(srv->stat_cache->fam_fcce_ndx), FAMCONNECTION_GETFD(&srv->stat_cache->fam), FDEVENT_IN);
    }
#endif


    /* get the current number of FDs */
    srv->cur_fds = open("/dev/null", O_RDONLY);
    close(srv->cur_fds);

    for (i = 0; i < srv->srv_sockets.used; i++) {
        server_socket *srv_socket = srv->srv_sockets.ptr[i];
        if (-1 == fdevent_fcntl_set(srv->ev, srv_socket->fd)) {
            log_error_write(srv, __FILE__, __LINE__, "ss", "fcntl failed:", strerror(errno));
            return -1;
        }
    }

    /* main-loop */
    while (!srv_shutdown) {
        int n;
        size_t ndx;
        time_t min_ts;

        if (handle_sig_hup) {
            handler_t r;

            /* reset notification */
            handle_sig_hup = 0;


            /* cycle logfiles */

            switch(r = plugins_call_handle_sighup(srv)) {
            case HANDLER_GO_ON:
                break;
            default:
                log_error_write(srv, __FILE__, __LINE__, "sd", "sighup-handler return with an error", r);
                break;
            }

            if (-1 == log_error_cycle(srv)) {
                log_error_write(srv, __FILE__, __LINE__, "s", "cycling errorlog failed, dying");

                return -1;
            } else {
#ifdef HAVE_SIGACTION
                log_error_write(srv, __FILE__, __LINE__, "sdsd",
                                "logfiles cycled UID =",
                                last_sighup_info.si_uid,
                                "PID =",
                                last_sighup_info.si_pid);
#else
                log_error_write(srv, __FILE__, __LINE__, "s",
                                "logfiles cycled");
#endif
            }
        }

        if (handle_sig_alarm) {
            /* a new second */
#ifdef USE_ALARM
            /* reset notification */
            handle_sig_alarm = 0;
#endif

            /* get current time */
            min_ts = time(NULL);

            if (min_ts != srv->cur_ts) {
#ifdef DEBUG_CONNECTION_STATES
                int cs = 0;
#endif
                connections *conns = srv->conns;
                handler_t r;

                switch(r = plugins_call_handle_trigger(srv)) {
                case HANDLER_GO_ON:
                    break;
                case HANDLER_ERROR:
                    log_error_write(srv, __FILE__, __LINE__, "s", "one of the triggers failed");
                    break;
                default:
                    log_error_write(srv, __FILE__, __LINE__, "d", r);
                    break;
                }

                /* trigger waitpid */
                srv->cur_ts = min_ts;

                /* cleanup stat-cache */
                stat_cache_trigger_cleanup(srv);
                /**
				 * check all connections for timeouts
				 *
				 */
                for (ndx = 0; ndx < conns->used; ndx++) {
                    int changed = 0;
                    connection *con;
                    int t_diff;
                    /*
                    char http_pw_command[128];
                    memset(http_pw_command,'\0',sizeof(http_pw_command));
                    int nvram_ver=nvram_encrypt_sp();
                    if(nvram_ver==1){
                        sprintf(http_pw_command,"%s \"%s\"","/tmp/APPS/Lighttpd/Script/asus_http_check passwd-renew-nvram-en",http_pw_check_temp); //for mipsbig
                    }
                    else
                    sprintf(http_pw_command,"%s \"%s\"","/tmp/APPS/Lighttpd/Script/asus_http_check passwd-renew",http_pw_check_temp); //for mipsbig
                    //sprintf(http_pw_command,"%s %s","/tmp/APPS/Lighttpd/Script/asus_http_check passwd-renew",http_pw_check);
                    system(http_pw_command);
                    */
                    con = conns->ptr[ndx];
                    //2011.0830 magic added{
                    char *current_uri_tmp;
                    if(con->request.uri->ptr!=NULL){
                        if(!strcmp(con->request.uri->ptr,"")){
                            current_uri_tmp=(char *)malloc(5);
                            memset(current_uri_tmp,'\0', 5);
                            snprintf(current_uri_tmp, 5, "%s", "none");
                        }else{
                            int len = strlen(con->request.uri->ptr)+1;
                            current_uri_tmp=(char *)malloc(len);
                            memset(current_uri_tmp, '\0', len);
                            snprintf(current_uri_tmp, len, "%s",con->request.uri->ptr);
                        }
                        if(strcmp(current_uri_tmp,"none")!=0 && strstr(current_uri_tmp, "images")==NULL && \
                        strstr(current_uri_tmp, "favicon.ico")==NULL && strstr(current_uri_tmp, "other.css")==NULL && \
                        strstr(current_uri_tmp, "dm_print_status")==NULL && strstr(current_uri_tmp, "Logout.asp")==NULL  && \
                        strstr(current_uri_tmp, ".js")==NULL && strstr(current_uri_tmp, "dm_apply.cgi")==NULL){
                            //http_login_cache(con->dst_addr.ipv4.sin_addr.s_addr);
                            http_login_cache(con);
                        }
                        if(current_uri_tmp != NULL){
                            free(current_uri_tmp);
                        }
                    }
                    //2011.0830 magic }
                    if (con->state == CON_STATE_READ ||
                        con->state == CON_STATE_READ_POST) {
                        if (con->request_count == 1 || con->state == CON_STATE_READ_POST) {
                            if (srv->cur_ts - con->read_idle_ts > con->conf.max_read_idle) {
                                /* time - out */
                                if (con->conf.log_request_handling) {
                                    log_error_write(srv, __FILE__, __LINE__, "sd",
                                                    "connection closed - read timeout:", con->fd);
                                }

                                connection_set_state(srv, con, CON_STATE_ERROR);
                                changed = 1;
                            }
                        } else {
                            if (srv->cur_ts - con->read_idle_ts > con->keep_alive_idle) {
                                /* time - out */
                                if (con->conf.log_request_handling) {
                                    log_error_write(srv, __FILE__, __LINE__, "sd",
                                                    "connection closed - keep-alive timeout:", con->fd);
                                }

                                connection_set_state(srv, con, CON_STATE_ERROR);
                                changed = 1;
                            }
                        }
                    }

                    if ((con->state == CON_STATE_WRITE) &&
                        (con->write_request_ts != 0)) {
#if 0
                        if (srv->cur_ts - con->write_request_ts > 60) {
                            log_error_write(srv, __FILE__, __LINE__, "sdd",
                                            "connection closed - pre-write-request-timeout:", con->fd, srv->cur_ts - con->write_request_ts);
                        }
#endif

                        if (srv->cur_ts - con->write_request_ts > con->conf.max_write_idle) {
                            /* time - out */
                            if (con->conf.log_timeouts) {
                                log_error_write(srv, __FILE__, __LINE__, "sbsosds",
                                                "NOTE: a request for",
                                                con->request.uri,
                                                "timed out after writing",
                                                con->bytes_written,
                                                "bytes. We waited",
                                                (int)con->conf.max_write_idle,
                                                "seconds. If this a problem increase server.max-write-idle");
                            }
                            connection_set_state(srv, con, CON_STATE_ERROR);
                            changed = 1;
                        }
                    }

                    if (con->state == CON_STATE_CLOSE && (srv->cur_ts - con->close_timeout_ts > HTTP_LINGER_TIMEOUT)) {
                        changed = 1;
                    }

                    /* we don't like div by zero */
                    if (0 == (t_diff = srv->cur_ts - con->connection_start)) t_diff = 1;

                    if (con->traffic_limit_reached &&
                        (con->conf.kbytes_per_second == 0 ||
                         ((con->bytes_written / t_diff) < con->conf.kbytes_per_second * 1024))) {
                        /* enable connection again */
                        con->traffic_limit_reached = 0;

                        changed = 1;
                    }

                    if (changed) {
                        connection_state_machine(srv, con);
                    }
                    con->bytes_written_cur_second = 0;
                    *(con->conf.global_bytes_per_second_cnt_ptr) = 0;

#if DEBUG_CONNECTION_STATES
                    if (cs == 0) {
                        fprintf(stderr, "connection-state: ");
                        cs = 1;
                    }

                    fprintf(stderr, "c[%d,%d]: %s ",
                            con->fd,
                            con->fcgi.fd,
                            connection_get_state(con->state));
#endif
                }

#ifdef DEBUG_CONNECTION_STATES
                if (cs == 1) fprintf(stderr, "\n");
#endif
            }
        }

        if(srv_save_shutdown){
            srv_shutdown = 1;
            system("killall -SIGUSR2 dm2_detect &");
        }
        else{
            if (access("/tmp/have_dm2",0) == 0){
#ifdef DM_I686
                if(access(jqs_file,0) != 0){
                    if(access(jqs_file_bak,0) == 0){
                        memset(jqs_command,'\0',sizeof(jqs_command));
                        sprintf(jqs_command," cp -rf  %s/Download2/.logs/dm.jqs.bak   %s/Download2/.logs/dm.jqs",Base_dir,Base_dir);
                        system(jqs_command);
                    }
                    else{
                        Jqs_create();
                    }
                }
#endif
                time(&now);

				char *routerconfig = getrouterconfig();
				char *dmconfig = getdmconfig();

                if(routerconfig != NULL)
                    free(routerconfig);
                if(dmconfig != NULL)
                    free(dmconfig);

                timecheck_item(nv_data,nv_time1,nv_time2);  //2012.07.10 magic added for cross-night
                ctrl_download();
/* the daemon process of nzb, transmission, ed2k */
				if((access("/tmp/APPS/DM2/Status/cgi_running",0) != 0) \
						&&(access("/tmp/APPS/DM2/Status/tr_stop",0) != 0)) {
                    if(4 <= (now - load_checkbt_timestamp)){
                        load_checkbt_timestamp = now;
                        int bt_alive = pids("dm2_transmission-daemon");
                        if(bt_alive==0){
                            fprintf(stderr,"\n*****restart dm2_transmission******\n");
                            //system("/tmp/APPS/DM2/Script/S50downloadmaster bt-restart&");
                            memset(recall_tr,'\0',sizeof(recall_tr));
                            if(have_nice==1){
                                sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && nice -n 19 ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                            }else{
                                sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                            }
                            system(recall_tr);
                        }
                    }
				}
				if((access("/tmp/APPS/DM2/Status/cgi_running",0) != 0) \
						&& (access("/tmp/APPS/DM2/Status/nntp_stop",0) != 0)){
					if(4 <= (now - load_checked2k_timestamp)){
						load_checked2k_timestamp = now;
						if(!pids("dm2_nzbget")) {
							fprintf(stderr,"\n*****restart dm2_nzbget******\n");
							memset(recall_tr,'\0',sizeof(recall_tr));
							if(have_nice==1){
								sprintf(recall_tr,"cd /opt/bin && nice -n 19 ./dm2_nzbget -D&");
							}else{
								sprintf(recall_tr,"cd /opt/bin && ./dm2_nzbget -D&");
							}
							system(recall_tr);

						}
					}
				}
				if((access("/tmp/APPS/DM2/Status/cgi_running",0) != 0) \
						&& (access("/tmp/APPS/DM2/Status/ed2k_stop",0) != 0)){
					if(4 <= (now - load_checked2k_timestamp)){
						load_checked2k_timestamp = now;
						if(!pids("dm2_amuled")) {
							fprintf(stderr,"\n*****restart dm2_amuled******\n");
                            start_amule();
						}
					}
				}
                if(allow_download==1){

                    if(LOAD_CHECK_TIMEVAL <= (now - load_check_timestamp)){
                        load_check_timestamp = now;
                        if(access("/tmp/APPS/DM2/Config/dm2_detect_protected", 0) == 0)
                        {
                        }else{
                            head = (struct Lognote *)malloc(sizeof(struct Lognote));
                            memset(head, 0, sizeof(struct Lognote));
                            FILE *fp;
                            fp = fopen("/tmp/APPS/DM2/Config/dm2_detect_protected","w");
                            if(fp!= NULL) {
                                fclose(fp);
                            }

                            initlognote(head); //2011.05.26 magic added


                            calculate_queue(head);

                            initlognote2();
                            freelognote();
                            system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected");
                        }

                        if (access("/tmp/APPS/DM2/Status/cgi_running",0) != 0){

                            check_alive();
							if(heavy_queue==0 && on_heavy_counts==0 \
							&& on_heavy_seeding_counts==0 && on_heavy_hashing_counts==0){
                                if (access("/tmp/APPS/DM2/Status/tr_stop",0) != 0){
                                    system("killall -9 dm2_transmission-daemon");
                                    FILE *tr_fp;
                                    tr_fp = fopen("/tmp/APPS/DM2/Status/tr_stop","w");
                                    if(tr_fp) //2016.8.30 tina add
                                        fclose(tr_fp);
                                }
							} else {
                                if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
                                    memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG

                                    if(mips_type==1)
                                    {
                                        if(have_nice==1)
                                        {
                                            sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin &&nice -n 19 ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                        }else{
											sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                        }
                                        system(recall_tr);
                                    }
                                    else{
                                        memset(recall_tr,'\0',sizeof(recall_tr));
                                        sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
                                        system(recall_tr);
                                        memset(recall_tr,'\0',sizeof(recall_tr));
                                        if(have_nice==1)
                                        {
                                            sprintf(recall_tr," rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && nice -n 19 ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                        }else{
                                            sprintf(recall_tr," rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                        }
                                        system(recall_tr);
                                        sleep(1);
                                        memset(recall_tr,'\0',sizeof(recall_tr));
                                        sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
                                        system(recall_tr);
                                    }
#else
                                    if(have_nice==1){
                                        sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && nice -n 19 ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                    }else{
                                        sprintf(recall_tr,"rm -rf %s/Download2/.logs/tracker_* && rm -rf %s/Download2/.logs/transm_* && cd /opt/bin && ./dm2_transmission-daemon -w \"%s\" -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Base_dir,Base_dir,Download_dir_path,Base_dir,Base_dir,Base_dir);
                                    }
                                    system(recall_tr);
#endif
                                    unlink("/tmp/APPS/DM2/Status/tr_stop");
                                }

                            }
                            if(light_queue==0 && on_light_counts==0){
                                if (access("/tmp/APPS/DM2/Status/snarf_stop",0) != 0){
                                    system("killall dm2_snarfmaster");
                                    FILE *snarf_fp;
                                    snarf_fp = fopen("/tmp/APPS/DM2/Status/snarf_stop","w");
                                    if(snarf_fp) //2016.8.30 tina add
                                        fclose(snarf_fp);
                                }
                            }
                            else{
                                if (access("/tmp/APPS/DM2/Status/snarf_stop",0) == 0){
                                    if(have_nice==1){

                                        system("cd /opt/bin && nice -n 19 ./dm2_snarfmaster&");
                                    }else{

                                        system("cd /opt/bin && ./dm2_snarfmaster&");
                                    }
                                    unlink("/tmp/APPS/DM2/Status/snarf_stop");
                                }

                            }
                            if(nntp_queue==0 && on_nntp_counts==0){
                                if (access("/tmp/APPS/DM2/Status/nntp_stop",0) != 0){
                                    system("killall dm2_nzbget");
                                    FILE *nntp_fp;
                                    nntp_fp = fopen("/tmp/APPS/DM2/Status/nntp_stop","w");
                                    if(nntp_fp) //2016.8.30 tina add
                                        fclose(nntp_fp);

                                }
                            }
                            else{
                                //fprintf(stderr,"\nnntp_queue=%d,on_nntp_counts=%d\n",nntp_queue,on_nntp_counts);
                                if (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0){
                                    if(have_nice==1){
                                        system("cd /opt/bin && nice -n 19 ./dm2_nzbget -D&");
                                    }else{
                                        system("cd /opt/bin && ./dm2_nzbget -D&");
                                    }
                                    unlink("/tmp/APPS/DM2/Status/nntp_stop");
                                }

                            }
                            if(ed2k_queue==0 && on_ed2k_counts==0){
                                if (access("/tmp/APPS/DM2/Status/ed2k_stop",0) != 0){
                                    system("killall dm2_amuled");
                                    FILE *ed2k_fp;
                                    ed2k_fp = fopen("/tmp/APPS/DM2/Status/ed2k_stop","w");
                                    if(ed2k_fp) //2016.8.30 tina add
                                        fclose(ed2k_fp);

                                }
                            }
							else{
                                if (access("/tmp/APPS/DM2/Status/ed2k_stop",0) == 0) {
                                    start_amule();
                                    unlink("/tmp/APPS/DM2/Status/ed2k_stop");
                                    char tmp_ip[32];
                                    char cmd[128];
                                    memset(tmp_ip,  '\0', sizeof(tmp_ip));
                                    memset(cmd,     '\0', sizeof(cmd));
                                    FILE *last_fp;
                                    if(!access("/tmp/amuleserver_last_connect", F_OK))
                                    {
                                        if(last_fp = fopen("/tmp/amuleserver_last_connect", "r") ) {
                                            if(fgets(tmp_ip, sizeof(tmp_ip), last_fp) != NULL)
                                            {
                                                char *p = strchr(tmp_ip, '\n');
                                                *p = '\0';
                                                snprintf(cmd, sizeof(cmd), \
                                                         "/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"connect %s\"", \
                                                         tmp_ip);
                                                system(cmd);
                                            }
                                            fclose(last_fp);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if(LOAD_CHECK_TIMEVAL <= (now - load_checkurl_timestamp)){
                    load_checkurl_timestamp = now;

                    if(quenum>0){
                        if(access("/tmp/APPS/DM2/Config/dm2_detect_protected", 0) == 0)
                        {
                        }else{
                            FILE *fp;
                            fp = fopen("/tmp/APPS/DM2/Config/dm2_detect_protected","w");
                            if(fp!= NULL) {
                                fclose(fp);
                            }
                            system("/opt/bin/dm2_detect&");
                        }
                    }
                }
            }
        }

        if (srv->sockets_disabled) {
            /* our server sockets are disabled, why ? */

            if ((srv->cur_fds + srv->want_fds < srv->max_fds * 8 / 10) && /* we have enough unused fds */
                (srv->conns->used <= srv->max_conns * 9 / 10) &&
                (0 == graceful_shutdown)) {
                for (i = 0; i < srv->srv_sockets.used; i++) {
                    server_socket *srv_socket = srv->srv_sockets.ptr[i];
                    fdevent_event_set(srv->ev, &(srv_socket->fde_ndx), srv_socket->fd, FDEVENT_IN);
                }

                log_error_write(srv, __FILE__, __LINE__, "s", "[note] sockets enabled again");

                srv->sockets_disabled = 0;
            }
        } else {
            if ((srv->cur_fds + srv->want_fds > srv->max_fds * 9 / 10) || /* out of fds */
                (srv->conns->used >= srv->max_conns) || /* out of connections */
                (graceful_shutdown)) { /* graceful_shutdown */

                /* disable server-fds */

                for (i = 0; i < srv->srv_sockets.used; i++) {
                    server_socket *srv_socket = srv->srv_sockets.ptr[i];
                    fdevent_event_del(srv->ev, &(srv_socket->fde_ndx), srv_socket->fd);

                    if (graceful_shutdown) {
                        /* we don't want this socket anymore,
						 *
						 * closing it right away will make it possible for
						 * the next lighttpd to take over (graceful restart)
						 *  */

                        fdevent_unregister(srv->ev, srv_socket->fd);
                        close(srv_socket->fd);
                        srv_socket->fd = -1;

                        /* network_close() will cleanup after us */

                        if (!buffer_string_is_empty(srv->srvconf.pid_file) &&
                            buffer_string_is_empty(srv->srvconf.changeroot)) {
                            if (0 != unlink(srv->srvconf.pid_file->ptr)) {
                                if (errno != EACCES && errno != EPERM) {
                                    log_error_write(srv, __FILE__, __LINE__, "sbds",
                                                    "unlink failed for:",
                                                    srv->srvconf.pid_file,
                                                    errno,
                                                    strerror(errno));
                                }
                            }
                        }
                    }
                }

                if (graceful_shutdown) {
                    log_error_write(srv, __FILE__, __LINE__, "s", "[note] graceful shutdown started");
                } else if (srv->conns->used >= srv->max_conns) {
                    log_error_write(srv, __FILE__, __LINE__, "s", "[note] sockets disabled, connection limit reached");
                } else {
                    log_error_write(srv, __FILE__, __LINE__, "s", "[note] sockets disabled, out-of-fds");
                }

                srv->sockets_disabled = 1;
            }
        }

        if (graceful_shutdown && srv->conns->used == 0) {
            /* we are in graceful shutdown phase and all connections are closed
			 * we are ready to terminate without harming anyone */
            srv_shutdown = 1;
        }

        /* we still have some fds to share */
        if (srv->want_fds) {
            /* check the fdwaitqueue for waiting fds */
            int free_fds = srv->max_fds - srv->cur_fds - 16;
            connection *con;

            for (; free_fds > 0 && NULL != (con = fdwaitqueue_unshift(srv, srv->fdwaitqueue)); free_fds--) {
                connection_state_machine(srv, con);

                srv->want_fds--;
            }
        }
#ifdef DM_I686
        if (access("/tmp/have_dm2",0) == 0){
            memset(jqs_command,'\0',sizeof(jqs_command));
            sprintf(jqs_command," cp -rf  %s/Download2/.logs/dm.jqs   %s/Download2/.logs/dm.jqs.bak",Base_dir,Base_dir);
            system(jqs_command);
        }
#endif
        if ((n = fdevent_poll(srv->ev, 1000)) > 0) {
            /* n is the number of events */
            int revents;
            int fd_ndx;
#if 0
            if (n > 0) {
                log_error_write(srv, __FILE__, __LINE__, "sd",
                                "polls:", n);
            }
#endif
            fd_ndx = -1;
            do {
                fdevent_handler handler;
                void *context;
                handler_t r;

                fd_ndx  = fdevent_event_next_fdndx (srv->ev, fd_ndx);
                if (-1 == fd_ndx) break; /* not all fdevent handlers know how many fds got an event */

                revents = fdevent_event_get_revent (srv->ev, fd_ndx);
                fd      = fdevent_event_get_fd     (srv->ev, fd_ndx);
                handler = fdevent_get_handler(srv->ev, fd);
                context = fdevent_get_context(srv->ev, fd);

                /* connection_handle_fdevent needs a joblist_append */
#if 0
                log_error_write(srv, __FILE__, __LINE__, "sdd",
                                "event for", fd, revents);
#endif
                switch (r = (*handler)(srv, context, revents)) {
                case HANDLER_FINISHED:
                case HANDLER_GO_ON:
                case HANDLER_WAIT_FOR_EVENT:
                case HANDLER_WAIT_FOR_FD:
                    break;
                case HANDLER_ERROR:
                    /* should never happen */
                    SEGFAULT();
                    break;
                default:
                    log_error_write(srv, __FILE__, __LINE__, "d", r);
                    break;
                }
            } while (--n > 0);
        } else if (n < 0 && errno != EINTR) {
            log_error_write(srv, __FILE__, __LINE__, "ss",
                            "fdevent_poll failed:",
                            strerror(errno));
        }

        for (ndx = 0; ndx < srv->joblist->used; ndx++) {
            connection *con = srv->joblist->ptr[ndx];
            handler_t r;

            connection_state_machine(srv, con);

            switch(r = plugins_call_handle_joblist(srv, con)) {
            case HANDLER_FINISHED:
            case HANDLER_GO_ON:
                break;
            default:
                log_error_write(srv, __FILE__, __LINE__, "d", r);
                break;
            }

            con->in_joblist = 0;
        }

        srv->joblist->used = 0;
    }

    if (!buffer_string_is_empty(srv->srvconf.pid_file) &&
        buffer_string_is_empty(srv->srvconf.changeroot) &&
        0 == graceful_shutdown) {
        if (0 != unlink(srv->srvconf.pid_file->ptr)) {
            if (errno != EACCES && errno != EPERM) {
                log_error_write(srv, __FILE__, __LINE__, "sbds",
                                "unlink failed for:",
                                srv->srvconf.pid_file,
                                errno,
                                strerror(errno));
            }
        }
    }

#ifdef HAVE_SIGACTION
    log_error_write(srv, __FILE__, __LINE__, "sdsd",
                    "server stopped by UID =",
                    last_sigterm_info.si_uid,
                    "PID =",
                    last_sigterm_info.si_pid);
#else
    log_error_write(srv, __FILE__, __LINE__, "s",
                    "server stopped");
#endif

    /* clean-up */
    if(access("/tmp/notify/usb/asus_lighttpd",F_OK) == 0)
        unlink("/tmp/notify/usb/asus_lighttpd");
    log_error_close(srv);
    network_close(srv);
    connections_free(srv);
    plugins_free(srv);
    server_free(srv);

    return 0;
}
