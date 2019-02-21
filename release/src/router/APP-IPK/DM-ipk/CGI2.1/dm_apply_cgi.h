#ifndef DM_APPLY_CGI_H
#define DM_APPLY_CGI_H 1
#ifdef DM_MIPSBIG
int mips_type;
#endif

enum aMule_status{
    DISABLE=1,
    DISCONNECT,
    CONNECT,
    CONNECTING,
    TIMEDOUT,
    REMOVED,
    ERROR
};
char *getlogid2(char *logname);
void init_tmp_dst(char *dst, char *src, int len);
void print_apply(char* d_type);
void DM_APPLY(char* d_type);
int DM_CTRL(char* cmd, char* task_id_name , char* d_type);
void unencode(char *src, char *last, char *dest);
void RemoveLogbyID(char *task_id);
#endif
