#ifndef __TRANSMISSION__
#endif

#ifndef TR_SOCKET_SERVER_H
#define TR_SOCKET_SERVER_H

void tr_tcpInit( tr_session * );
void tr_tcpUninit( tr_session * );

typedef void( *tr_socket_response_func )( tr_session      * session,
                                       struct evbuffer * response,
                                       void            * user_data );



#endif
int getFreeMem(void); //2011.12.20 magic added
int max_torrent_peer_tmp;
int max_peer_tmp;
int up_limit_enable;
int up_rate_tmp;
char disktype[10]; //2012.5.15 eric added
int getMinMem(void); //2013.06.24 magic added
void getproduct(void);

struct disk_info *initial_disk_data(struct disk_info **disk);
void free_disk_struc(struct disk_info **disk);
int init_diskinfo_struct();

struct disk_info{
    int port;
    int partitionport;
    char *disktype;//eric added for disktype
    char *product;
    char *vendor;
    char *serialnum;
    char *mountpath;
    char *diskname;
    char *diskpartition;
    struct disk_info *next;
};

int singledisk;
char dldir[100];
int dldirchangednum;

#define GENERALFILE "/opt/etc/dm2_general.conf"
#define LOGFILE "/tmp/dm2_log"
char generalbuf[100];
char serialtmp[50];
char producttmp[10];
char vondertmp[10];
int partitiontmp;

int misc_seeding_x;

struct disk_info *follow_disk_info,*follow_disk_info_start,*follow_disk_tmp;

