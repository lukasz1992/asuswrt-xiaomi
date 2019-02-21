#ifndef DM_PRINT_STATUS_H
#define DM_PRINT_STATUS_H 1
#include"dm.h"

/*	get ed2k server status from /tmp/dm2_amule_status
 *
 *	return vale: 1 is connected
				 2 is connecting
				 3 is disconnected
				 0 is timeout
 */
int	get_ed2k_server_status();
int queue_job(struct Lognote *);
struct Lognote * createnote(int , char *,char *, char *, char * ,char *, int ,int);
void insertnote(struct Lognote *, struct Lognote *);
void read_queue_jobs(struct Lognote *);
void free_jobs_queue();
int read_tracker(Tracker_struct *, char *, int );
int read_file(File_detail *, char *, int );
void print_log( Log_struc *, char *, int , char* );
void print_queue_jobs(struct Lognote *, int);
void delet(char *,int);
void print_single_log( Log_struc *,  char *, char *);
int dm_print_single_status(char*,char*,char *);
int dm_print_status(char*);

#endif
