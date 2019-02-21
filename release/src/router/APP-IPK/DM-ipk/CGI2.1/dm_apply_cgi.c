#include "dm_apply_cgi.h"
#include "dm_func.h"
#include "dm_hook.h"
#include "dm_url_parser.h"
#include "dm_http_parser.h"
#include "dm_ftp_parser.h"
#include <errno.h>

#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#ifdef DM_MIPSBIG
void check_mips_type(){

	mips_type=0;
	if(access("/userfs/bin/tcapi",0) == 0){
		mips_type=1;
	}
	else
	{
		mips_type=0;
	}
}
#endif
void unencode(char *src, char *last, char *dest)
{
    for(; src != last; src++, dest++){
        if(*src == '+'){
            *dest = ' ';
	}
        else if(*src == '%'){
            int code;
            if(sscanf(src+1, "%2x", &code) != 1)
            {
                code = '?';
            }
            *dest = code;
            src +=2;
        }else{
            *dest = *src;
        }
    }
    *dest = '\n';
    *++dest = ' ';
} 


char *getlogid2(char *logname)
{
    char *ptr;

    if((ptr = strstr(logname, "snarf_")) != NULL){
        ptr += 6;
    }
    else if((ptr = strstr(logname, "transm_")) != NULL){
        ptr += 7;
    }
    else if((ptr = strstr(logname, "nzb_")) != NULL){
        ptr += 4;
    }
    else if((ptr = strstr(logname, "ed2k_")) != NULL){
        ptr += 5;
    }
    //else if((ptr = strstr(logname, "error_")) != NULL){
    //    ptr += 6;
    //}
    else{
        return "none";
    }
    return ptr;
}


void init_tmp_dst(char *dst, char *src, int len){
    int i;

    for(i=len-1; i>=0; --i){
        if(src[i] == '/')
            break;
    }
    strncpy(dst, src+(i+1), len-(i+1));
}

void print_apply(char* d_type)
{
    if(!strcmp(d_type, "BT"))
    {
	int fd, len, i=0;
	    char ch, tmp[256], name[256], content[256];
	    memset(tmp, 0, sizeof(tmp));
	    memset(name, 0, sizeof(name));
	    memset(content, 0, sizeof(content));

	    if((fd = open("/tmp/APPS/DM2/Config/dm2_transmission.conf", O_RDONLY | O_NONBLOCK)) < 0)
	    {
            char* peer_port = "51413";
		    char* auth_type = "1";
		    char* max_torrent_peer = "60";
		    char* max_peer = "240";
		    char* en_dht = "1";
		    char* down_limit = "0";
		    char* down_rate = "100";
		    char* up_limit = "0";
		    char* up_rate = "100";
		    char* en_pex = "1";
		    char* en_peer_port = "0";

		    printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", peer_port,auth_type,max_torrent_peer,max_peer,en_dht,down_limit,down_rate,up_limit,up_rate,en_pex,en_peer_port);
	    }
	    else
	    {

		    while((len = read(fd, &ch, 1)) > 0)
		    {
		        if(ch == '=')
		        {
		            strcpy(name, tmp);
		            memset(tmp, 0, sizeof(tmp));
		            i = 0;
		            continue;
		        }
		        else if(ch == '\n')
		        {
		            strcpy(content, tmp);
		           
		            memset(tmp, 0, sizeof(tmp));
		            i = 0;

		            if(strcmp(name, "Peer_port") == 0)
		            {
		                printf("[\"%s\"",content);
		            }
		            else if(!strcmp(name, "Auth_type") || !strcmp(name, "Max_torrent_peer") || !strcmp(name, "Max_peer") || !strcmp(name, "Enable_dht") || !strcmp(name, "Down_limit") || !strcmp(name, "Down_rate") || !strcmp(name, "Up_limit") || !strcmp(name, "Up_rate") || !strcmp(name, "Enable_pex"))
		            {
		                printf(",\"%s\"",content);

		            }
		            else if(strcmp(name, "Enable_peer_port") == 0)
		            {
		                printf(",\"%s\"]",content);

		            }

		            continue;
		        }


		        memcpy(tmp+i, &ch, 1);
		        i++;
		    }
		    close(fd);
	      }
    }

    else if(!strcmp(d_type, "NZB"))
    {
	int fd, len, i=0;
	    char ch, tmp[256], name[256], content[256];
	    memset(tmp, 0, sizeof(tmp));
	    memset(name, 0, sizeof(name));
	    memset(content, 0, sizeof(content));

	    if((fd = open("/tmp/APPS/DM2/Config/dm2_nzbget_EX.conf", O_RDONLY | O_NONBLOCK)) < 0)
	    {
		    char host[5] = "\0";
		    char port[5] = "119";
		    char encryption[5] = "no";
		    char username[5] = "\0";;
		    char password[5] = "\0";;
		    char connections[5] = "4";
		    char downrate[5] = "0";

		    printf("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", host,port,encryption,username,password,connections,downrate);
	    }
	    else
	    {

		    while((len = read(fd, &ch, 1)) > 0)
		    {
		        if(ch == '=')
		        {
		            strcpy(name, tmp);
		            memset(tmp, 0, sizeof(tmp));
		            i = 0;
		            continue;
		        }
		        else if(ch == '\n')
		        {
		            strcpy(content, tmp);
		           
		            memset(tmp, 0, sizeof(tmp));
		            i = 0;

		            if(strcmp(name, "Server1.Host") == 0)
		            {
		                printf("[\"%s\"",content);
		            }
		            else if(!strcmp(name, "Server1.Port") || !strcmp(name, "Server1.Encryption") || !strcmp(name, "Server1.Username") || !strcmp(name, "Server1.Password") || !strcmp(name, "Server1.Connections"))
		            {
		                printf(",\"%s\"",content);

		            }
		            else if(strcmp(name, "DownloadRate") == 0)
		            {
		                printf(",\"%s\"]",content);

		            }

		            continue;
		        }


		        memcpy(tmp+i, &ch, 1);
		        i++;
		    }
		    close(fd);
	      }
    } else if(!strcmp(d_type, "ED2K_SERVER_LIST")) {
        FILE *fp;//for server.met
		char *p;
        char amule_ip[16];
        char server_list[2048];
        char amule_server[30];
        char timedout_server[16];
        int  status=0;
        int  status1=0;

        memset(amule_ip,        '\0', sizeof(amule_ip));
        memset(server_list,     '\0', sizeof(server_list));
        memset(amule_server,    '\0', sizeof(amule_server));
        memset(timedout_server, '\0', sizeof(timedout_server));

        if(detect_process("dm2_amuled")==0) {
            //the amule disable
            status = DISABLE; //1
            snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                     "\"0\",\"0\",\"1\",");//for inital status
        } else {
            snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                     "\"0\",\"0\",\"2\",");
            FILE *fp_status;//dm2_amule_status
            if (fp_status=fopen("/tmp/dm2_amule_status", "r"))
                //read really status form the file
            {
                while (fgets(amule_server, 30, fp_status) != NULL)
                {
                    if(!strncmp(amule_server, "status:", 7)) {
                        if(!strncmp(amule_server+7, "disconnected", 12)) {
                            status = DISCONNECT;//2
                        } else if(!strncmp(amule_server+7, "connected", 9)) {
                            status = CONNECT;//3
                        } else if(!strncmp(amule_server+7, "connecting", 10)) {
                            status = CONNECTING;//4
                        } else {
                            status = DISCONNECT;
                        }
                    } else if(!strncmp(amule_server, "server_ip:", 10)) {
                        strncpy(amule_ip, amule_server+10, 15);
						if((p = strchr(amule_ip, '\n')) != NULL)
							*p = '\0';
                    }
                }
                fclose(fp_status);
            } else {
                status = DISCONNECT;
            }
            if (!access("/tmp/dm2_amule_timedout", F_OK)) {
                FILE *fp_timedout;
                char amule_timeout[30];
                memset(amule_timeout, '\0', sizeof(amule_timeout));

                if(fp_timedout = fopen("/tmp/dm2_amule_timedout", "r")) {
                    while (fgets(amule_timeout, 30, fp_timedout) != NULL)
                    {
                        if(!strncmp(amule_timeout, "status:", 7)) {
                            if(!strncmp(amule_timeout+7, "timedout", 8)) {
                                status1 = TIMEDOUT;//5
                            }
                        } else if(!strncmp(amule_timeout, "server_ip:", 10)) {
                            strncpy(timedout_server, amule_timeout+10, 15);
							if((p = strchr(timedout_server, '\n')) != NULL)
								*p = '\0';
                        }
                    }
                    fclose(fp_timedout);
                }
            }
        }
        if(fp = fopen("/tmp/APPS/DM2/Config/dm2_amule/aMule_server", "r")) {
            char tmp[32];
            char ip[16];
			char port[6];
            memset(tmp, '\0', sizeof(tmp));
			while(fgets(tmp, sizeof(tmp), fp))//ip:port
			{
                memset(ip, '\0', sizeof(ip));
                memset(port, '\0', sizeof(port));
                p = strchr(tmp , ':');
                if(p) {
					strncpy(port, p+1, 5);
					*p = '\0';
                    strncpy(ip, tmp, 16);
				}
				if((p = strchr(port, '\n')) != NULL)
					*p = '\0';
                if(strlen(ip) < 7 || strlen(port)<1)
                    continue;
                if (status == DISABLE) {
                    snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                             "\"%s\",\"%s\",\"%d\",", ip, port, status);
				} else if (status1 == TIMEDOUT && status != CONNECT && !strcmp(ip, timedout_server)) {
                    snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                             "\"%s\",\"%s\",\"%d\",", ip, port, status1);
                } else if (status == DISCONNECT){
                    snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                             "\"%s\",\"%s\",\"2\",", ip, port);
                } else if(!strcmp(ip, amule_ip)) {
                    snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                             "\"%s\",\"%s\",\"%d\",", ip, port, status);
                } else {
                    snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                             "\"%s\",\"%s\",\"2\",", ip, port);
                }
                memset(tmp, '\0', sizeof(tmp));
            }

            fclose(fp);
        }
        //will printf the removed server at here
        FILE *fp_removed;
        char amule_remove[30];
        if(fp_removed=fopen("/tmp/APPS/DM2/Config/dm2_amule/server_removed","r")) {
            char removed_ip[16];
            char removed_port[6];
            while(!feof(fp_removed))

			{
				memset(removed_ip, '\0', sizeof(removed_ip));
				memset(removed_port,'\0', sizeof(removed_port));
                if(fgets(amule_remove, 30, fp_removed) != NULL)
                {
                    if(!strncmp(amule_remove, "ip:", 3)) {
						strncpy(removed_ip, amule_remove+3, 15);
						if((p = strchr(removed_ip, '\n')) != NULL)
							*p = '\0';
                    }
                } else {
                    break;
                }
                if(fgets(amule_remove, 30, fp_removed) != NULL)
                {
                    if(!strncmp(amule_remove, "port:", 5)) {
						strncpy(removed_port, amule_remove+5, 5);
						if((p = strchr(removed_port, '\n')) != NULL)
							*p = '\0';
                    }
                } else {
                    break;
                }
                snprintf(server_list+strlen(server_list), sizeof(server_list)-strlen(server_list), \
                         "\"%s\",\"%s\",\"6\",", removed_ip, removed_port);
            }
            fclose(fp_removed);
        }
        printf("[%s]", server_list);

    } else if(!strcmp(d_type, "General"))
    {

        system("touch /tmp/asus_routergen_tag;/tmp/APPS/Lighttpd/Script/asus_check_general router-general-renew &");
        while(access("/tmp/asus_routergen_tag",0)==0);

        char *routerconfig = getrouterconfig();
        if(routerconfig != NULL)
            free(routerconfig);

        if (access("/tmp/APPS/DM2/Config/dm2_general.conf",0) == 0)
        {

            int fd, len, i=0;
            char ch, tmp[256], name[256], content[256];
            memset(tmp, 0, sizeof(tmp));
            memset(name, 0, sizeof(name));
            memset(content, 0, sizeof(content));

            if((fd = open("/tmp/APPS/DM2/Config/dm2_general.conf", O_RDONLY | O_NONBLOCK)) > 0)
            {
                while((len = read(fd, &ch, 1)) > 0)
                {
                    if(ch == '=')
                    {
                        strcpy(name, tmp);
                        memset(tmp, 0, sizeof(tmp));
                        i = 0;
                        continue;
                    }
                    else if(ch == '\n')
                    {
                        strcpy(content, tmp);
                        memset(tmp, 0, sizeof(tmp));
                        i = 0;

                        if(strcmp(name, "Enable_time") == 0)
                        {
                            printf("[\"%s\"",content);
                        }
                        else if(strcmp(name, "Start_hour") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "Start_minute") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "End_hour") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "End_minute") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "Day") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "Download_dir") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "Refresh_rate") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "BASE_PATH") == 0)
                        {
                            printf(",\"%s\"",Base_dir);

                        }
                        else if(strcmp(name, "MISC_HTTP_X") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "LAN_IP") == 0)
                        {
                            printf(",\"%s\"",lan_ip_addr);

                        }
                        else if(strcmp(name, "MISCR_HTTPPORT_X") == 0)
                        {

                            printf(",\"%s\"",miscr_httpport_x_check);

                        }
                        else if(strcmp(name, "MISCR_HTTP_X") == 0)
                        {
                            printf(",\"%s\"",miscr_http_x_check);

                        }
                        else if(strcmp(name, "DM_PORT") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "LANGUAGE") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "PRODUCTID") == 0)
                        {
                            printf(",\"%s\"",productid_check);

                        }
                        else if(strcmp(name, "APPS_DEV") == 0)
                        {
                            printf(",\"%s\"",apps_dev_path);

                        }
                        else if(strcmp(name, "WAN_IP") == 0)
                        {
                            printf(",\"%s\"",wan_ip_check);

                        }
                        else if(strcmp(name, "DDNS_ENABLE_X") == 0)
                        {
                            printf(",\"%s\"",ddns_enable_x_check);

                        }
                        else if(strcmp(name, "DDNS_HOSTNAME_X") == 0)
                        {
                            printf(",\"%s\"",ddns_hostname_x_check);

                        }
                        else if(strcmp(name, "MAX_ON_HEAVY") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "MAX_QUEUES") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "MAX_ON_ED2K") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "RFW_ENABLE_X") == 0)
                        {
                            printf(",\"%s\"",rfw_enable_x_check);

                        }
                        else if(strcmp(name, "DEVICE_TYPE") == 0)
                        {
                            printf(",\"%s\"",device_type_check);

                        }
                        else if(strcmp(name, "dm_radio_time_x") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "dm_radio_time2_x") == 0)
                        {
                            printf(",\"%s\"",content);

                        }
                        else if(strcmp(name, "MISC_SEEDING_X") == 0)
                        {
                            printf(",\"%s\"",content);

                        }

                        continue;
                    }

                    memcpy(tmp+i, &ch, 1);
                    i++;
                }
                printf(",\"%s\"",utility_ver_check);
                printf(",\"%s\"",local_domain_check);
                printf(",\"%s\"",http_autologout_check);
                printf(",\"%s\"",miscr_httpsport_x_check);
                printf(",\"%s\"",router_timezone);
                printf(",\"%s\"",router_sw_mode);
                printf(",\"%s\"",router_https_port);
				printf(",\"%s\"",https_lanport_check);
				printf(",\"%s\"]",http_enable);
                close(fd);
            }
        }

    }
}

int Dir_renew_socket(char *socket_type)
{
    int sfd;
    int result = UNKNOW;
    char command[24];
    memset(command, '\0', sizeof(command));
  if(!strcmp(socket_type, "all")){
	sprintf(command, "%s", "all-renew-now");	
	}
  else if(!strcmp(socket_type, "dir")){
	sprintf(command, "%s", "dir-renew-now");	
   }
  else if(!strcmp(socket_type, "seeding")){
	sprintf(command, "%s", "seeding-renew-now");	
   }
    struct sockaddr_in btaddr;

    memset(&btaddr, '\0', sizeof(btaddr));
    btaddr.sin_family = AF_INET;
    btaddr.sin_port = htons(BT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
    result = ACK_SUCESS;

		FILE *cgi_fp;
		cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
		fclose(cgi_fp);
		if(detect_process("dm2_transmission-daemon")==0) 
  		{
			int recall_engine=0;
			char recall_tr[1024];
				memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG
				if(mips_type==1)
				{
			    		sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
				}
				else{
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
					system(recall_tr);
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
					sleep(1);
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
					system(recall_tr);
				}
#else
		    		sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
				//fprintf(stderr,"\nrecall_tr=%s\n",recall_tr);
				system(recall_tr);
#endif

				recall_engine=3;
			if(recall_engine>0){
				//fprintf(stderr,"\nrecall_engine=%d\n",recall_engine);
				sleep(recall_engine);
			}
		}

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        result = ACK_FAIL;
	unlink("/tmp/APPS/DM2/Status/tr_stop");
	unlink("/tmp/APPS/DM2/Status/cgi_running");
        return result;
    }

    if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
        result = ACK_FAIL;
	unlink("/tmp/APPS/DM2/Status/tr_stop");
	unlink("/tmp/APPS/DM2/Status/cgi_running");
        return result;
    }
    if(write(sfd, command, strlen(command)) != strlen(command))
    {
        result = ACK_FAIL;
	unlink("/tmp/APPS/DM2/Status/tr_stop");
	unlink("/tmp/APPS/DM2/Status/cgi_running");
        return result;
    }
	unlink("/tmp/APPS/DM2/Status/tr_stop");
	unlink("/tmp/APPS/DM2/Status/cgi_running");
    close(sfd);
    sleep(1);
    //fprintf(stderr,"\nDir_renew_socket end\n");
    //fprintf(stderr,"\nresult=%d\n",result);
    return result;
}

void DM_APPLY(char* d_type)
{
    int result = UNKNOW;
    FILE *fp;

    if(!strcmp(d_type, "BT")){

        int sfd;
        char *command;
        struct sockaddr_in btaddr;
        memset(&btaddr, '\0', sizeof(btaddr));

        btaddr.sin_family = AF_INET;

        char* peer_port;
        char* auth_type;
        char* max_torrent_peer;
        char* max_peer;
        char* en_dht;
        char* down_limit;
        char* down_rate;
        char* up_limit;
        char* up_rate;
        char* en_pex;
        char* en_peer_port;
        char* peer_port_change;
        char* bt_port_tmp;

        peer_port = websGetVar(wp, "Peer_port", "");
        auth_type = websGetVar(wp, "Auth_type", "");
        max_torrent_peer = websGetVar(wp, "Max_torrent_peer", "");
        max_peer = websGetVar(wp, "Max_peer", "");
        en_dht = websGetVar(wp, "Enable_dht", "");
        down_limit = websGetVar(wp, "Down_limit", "");
        down_rate = websGetVar(wp, "Down_rate", "");
        up_limit = websGetVar(wp, "Up_limit", "");
        up_rate = websGetVar(wp, "Up_rate", "");
        en_pex = websGetVar(wp, "Enable_pex", "");
        en_peer_port = websGetVar(wp, "Enable_peer_port", "");
        peer_port_change = websGetVar(wp, "peer_port_change", "");
        bt_port_tmp = websGetVar(wp, "bt_port_tmp", "");

        command=malloc(strlen(peer_port)+strlen(auth_type)+strlen(max_torrent_peer)+strlen(max_peer)+strlen(en_dht)+strlen(down_limit)+strlen(down_rate)+strlen(up_limit)+strlen(up_rate)+strlen(en_pex)+200);
        memset(command,'\0',sizeof(command));

        sprintf(command,"apply@peer_port=%s@auth_type=%s@max_torrent_peer=%s@max_peer=%s@en_dht=%s@down_limit=%s@down_rate=%s@up_limit=%s@up_rate=%s@en_pex=%s@",peer_port,auth_type,max_torrent_peer,max_peer,en_dht,down_limit,down_rate,up_limit,up_rate,en_pex);

        btaddr.sin_port = htons(BT_PORT);
        inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);

        if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("ACK_FAIL");
        }
        else if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
            printf("ACK_FAIL");
        }
        else if(write(sfd, command, strlen(command)) != strlen(command)){
            printf("ACK_FAIL");
        }
        else{
            printf("ACK_SUCESS");
        }
	
        close(sfd);
        if(command != NULL)
        {
            free(command);
        }
        fp = fopen("/opt/etc/dm2_transmission.conf","w+");
        result = fprintf(fp,"Peer_port=%s\nAuth_type=%s\nMax_torrent_peer=%s\nMax_peer=%s\nEnable_dht=%s\nDown_limit=%s\nDown_rate=%s\nUp_limit=%s\nUp_rate=%s\nEnable_pex=%s\nEnable_peer_port=%s\n",peer_port,auth_type,max_torrent_peer,max_peer,en_dht,down_limit,down_rate,up_limit,up_rate,en_pex,en_peer_port);
        fclose(fp);

		char changbt[256];
		memset(changbt,'\0',sizeof(changbt));
        char nvramcmd[256];//eric added for nvram change 2012.10.25{
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"cp -rf /opt/etc/dm2_transmission.conf /tmp/APPS/DM2/Config/dm2_transmission.conf");
        system(nvramcmd);
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        char nvramcmmit[]={"nvram commit"};

		sprintf(changbt,"sed -i 's/^.*peer-port\":.*/    \"peer-port\": %s, /g' %s/config/settings.json",peer_port,Share_dir);
		system(changbt);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_peer_port %s",peer_port);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_peer_port=%s",peer_port);
        }
        system(nvramcmd);
#else
        sprintf(nvramcmd,"nvram set trs_peer_port=%s",peer_port);
        system(nvramcmd);
#endif
        memset(nvramcmd,'\0',sizeof(nvramcmd));
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_enable_peer_port %s",en_peer_port);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_enable_peer_port=%s",en_peer_port);
        }
#else
            sprintf(nvramcmd,"nvram set trs_enable_peer_port=%s",en_peer_port);
#endif

        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
		memset(changbt,'\0',sizeof(changbt));

		if(!strcmp(auth_type,"0")){
			sprintf(changbt,"sed -i 's/^.*encryption\":.*/    \"encryption\": 0, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_auth_type 0");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_auth_type=0");
            }
#else
            sprintf(nvramcmd,"nvram set trs_auth_type=0");
#endif
        } else if (!strcmp(auth_type,"1")){
			sprintf(changbt,"sed -i 's/^.*encryption\":.*/    \"encryption\": 1, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_auth_type 1");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_auth_type=1");
            }
#else
            sprintf(nvramcmd,"nvram set trs_auth_type=1");
#endif                      
        } else if(!strcmp(auth_type,"2")){
			sprintf(changbt,"sed -i 's/^.*encryption\":.*/    \"encryption\": 2, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_auth_type 2");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_auth_type=2");
            }
#else
            sprintf(nvramcmd,"nvram set trs_auth_type=2");
#endif

        }
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        sprintf(changbt,"sed -i 's/^.*peer-limit-global\":.*/    \"peer-limit-global\": %s, /g' %s/config/settings.json",max_peer,Share_dir);
        system(changbt);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_max_peer %s",max_peer);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_max_peer=%s",max_peer);
        }
#else
        sprintf(nvramcmd,"nvram set trs_max_peer=%s",max_peer);
#endif

        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));

        sprintf(changbt,"sed -i 's/^.*peer-limit-per-torrent\":.*/    \"peer-limit-per-torrent\": %s, /g' %s/config/settings.json",max_torrent_peer,Share_dir);
        system(changbt);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_max_torrent_peer %s",max_torrent_peer);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_max_torrent_peer=%s",max_torrent_peer);
        }
#else
        sprintf(nvramcmd,"nvram set trs_max_torrent_peer=%s",max_torrent_peer);
#endif
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        if(!strcmp(en_dht,"0")){
            sprintf(changbt,"sed -i 's/^.*dht-enabled\":.*/    \"dht-enabled\": false, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG 
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_enable_dht 0");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_enable_dht=0");
            }
#else
            sprintf(nvramcmd,"nvram set trs_enable_dht=0");
#endif
        }
        else if(!strcmp(en_dht,"1")){
            sprintf(changbt,"sed -i 's/^.*dht-enabled\":.*/    \"dht-enabled\": true, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG 
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_enable_dht 1");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_enable_dht=1");
            }
#else
            sprintf(nvramcmd,"nvram set trs_enable_dht=1");
#endif
        }
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        if(!strcmp(down_limit,"0")){
            sprintf(changbt,"sed -i 's/^.*speed-limit-down-enabled\":.*/    \"speed-limit-down-enabled\": false, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG 
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_down_limit 0");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_down_limit=0");
            }
#else
            sprintf(nvramcmd,"nvram set trs_down_limit=0");
#endif
        }
        else if(!strcmp(down_limit,"1")){
            sprintf(changbt,"sed -i 's/^.*speed-limit-down-enabled\":.*/    \"speed-limit-down-enabled\": true, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_down_limit 1");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_down_limit=1");
            }
#else
            sprintf(nvramcmd,"nvram set trs_down_limit=1");
#endif
        }
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        sprintf(changbt,"sed -i 's/^.*speed-limit-down\":.*/    \"speed-limit-down\": %s, /g' %s/config/settings.json",down_rate,Share_dir);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_down_rate %s",down_rate);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_down_rate=%s",down_rate);
        }
#else
        sprintf(nvramcmd,"nvram set trs_down_rate=%s",down_rate);
#endif
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        if(!strcmp(up_limit,"0")){
            //sprintf(changbt,"sed -i '54s/^.*$/    \"speed-limit-up-enabled\": false,/' %s/config/settings.json",Share_dir);
            sprintf(changbt,"sed -i 's/^.*speed-limit-up-enabled\":.*/    \"speed-limit-up-enabled\": false, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_up_limit 0");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_up_limit=0");
            }
#else
            sprintf(nvramcmd,"nvram set trs_up_limit=0");
#endif
        }
        else if(!strcmp(up_limit,"1")){
            sprintf(changbt,"sed -i 's/^.*speed-limit-up-enabled\":.*/    \"speed-limit-up-enabled\": true, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_up_limit 1");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_up_limit=1");
            }
#else
            sprintf(nvramcmd,"nvram set trs_up_limit=1");
#endif		
        }
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));

        sprintf(changbt,"sed -i 's/^.*speed-limit-up\":.*/    \"speed-limit-up\": %s, /g' %s/config/settings.json",up_rate,Share_dir);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_up_rate %s",up_rate);
        }
        else
        {
            sprintf(nvramcmd,"nvram set trs_up_rate=%s",up_rate);
        }
#else
        sprintf(nvramcmd,"nvram set trs_up_rate=%s",up_rate);

#endif
        system(changbt);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        memset(changbt,'\0',sizeof(changbt));
        if(!strcmp(en_pex,"0")){
            sprintf(changbt,"sed -i 's/^.*pex-enabled\":.*/    \"pex-enabled\": false, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_enable_pex 0");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_enable_pex=0");
            }
#else
            sprintf(nvramcmd,"nvram set trs_enable_pex=0");
#endif
        }
        else if(!strcmp(en_pex,"1")){
            sprintf(changbt,"sed -i 's/^.*pex-enabled\":.*/    \"pex-enabled\": true, /g' %s/config/settings.json",Share_dir);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry trs_enable_pex 1");
            }
            else
            {
                sprintf(nvramcmd,"nvram set trs_enable_pex=1");
            }
#else
            sprintf(nvramcmd,"nvram set trs_enable_pex=1");
#endif
        }
        system(changbt);
        system(nvramcmd);
        if(!strcmp(peer_port_change,"1")){
            memset(changbt,'\0',sizeof(changbt));
            sprintf(changbt,"/tmp/APPS/DM2/Script/S50downloadmaster firewall-restart %s &",bt_port_tmp);
            system(changbt);
        }
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            system("/userfs/bin/tcapi commit Apps");
            system("/userfs/bin/tcapi save");	//eric added for nvram change 2012.10.25}
        }
        else
        {
            system(nvramcmmit);
        }
#else

        system(nvramcmmit);//eric added for nvram change 2012.10.25}
#endif
    }
    else if(!strcmp(d_type, "NZB")){

        char* host;
        char* port;
        char* encryption;
        char* username;
        char* password;
        char* connections;
        char* downrate;

        host = websGetVar(wp, "Server1.Host", "");
        port = websGetVar(wp, "Server1.Port", "");
        encryption = websGetVar(wp, "Server1.Encryption", "");
        username = websGetVar(wp, "Server1.Username", "");
        password = websGetVar(wp, "Server1.Password", "");
        connections = websGetVar(wp, "Server1.Connections", "");
        downrate = websGetVar(wp, "DownloadRate", "");


        fp = fopen("/opt/etc/dm2_nzbget_EX.conf","wr");
        if( fp != NULL )
            result = fprintf(fp,"Server1.Host=%s\nServer1.Port=%s\nServer1.Encryption=%s\nServer1.Username=%s\nServer1.Password=%s\nServer1.Connections=%s\nDownloadRate=%s\n",host,port,encryption,username,password,connections,downrate);
        fclose(fp);

        char nvramcmd[256]; //eric added for nvram change 2012.10.31{
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"cp -rf /opt/etc/dm2_nzbget_EX.conf /tmp/APPS/DM2/Config/dm2_nzbget_EX.conf");
        system(nvramcmd);
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_host %s",host);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_port %s",port);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_ssl %s",encryption);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_name %s",username);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_pw %s",password);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_connect %s",connections);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry nzb_rate %s",downrate);
            system(nvramcmd);

            system("/userfs/bin/tcapi commit Apps");
            system("/userfs/bin/tcapi save");//eric added for nvram change 2012.10.31}
        }
        else{
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_host=%s",host);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_port=%s",port);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_ssl=%s",encryption);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_name=%s",username);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_pw=%s",password);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_connect=%s",connections);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set nzb_rate=%s",downrate);
            system(nvramcmd);

            system("nvram commit");//eric added for nvram change 2012.10.31}
        }
#else
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_host=%s",host);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_port=%s",port);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_ssl=%s",encryption);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_name=%s",username);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_pw=%s",password);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_connect=%s",connections);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set nzb_rate=%s",downrate);
        system(nvramcmd);

        system("nvram commit");//eric added for nvram change 2012.10.31}
#endif
        if(result < 0)
            printf("ACK_FAIL");
        else
        {
            if (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0){
                printf("ACK_SUCESS");
            }else{
                system("killall -SIGINT dm2_nzbget");
                sleep(1);
				system("killall -9 dm2_nzbget; /opt/bin/dm2_nzbget -D &");
                printf("ACK_SUCESS");
            }
        }

    }
    else if(!strcmp(d_type, "General")){
        char* enable_time;
        char* start_hour;
        char* start_minute;
        char* end_hour;
        char* end_minute;
        char* day;
        char* download_dir;
        char* refresh;
        char* base_path;
        char* misc_http_x;
        char* miscr_httpport_x;
        char* miscr_http_x;
        char* lan_ip;
        char* dm_port;
        char* dm_https_port;
        char* dm_port_renew;
        char* dm_https_port_renew;
        char* dm_dir_renew;
        char* enable_time_renew;
        char* dm_language;
        char* productid;
        char* apps_dev;
        char* wan_ip;
        char* ddns_enable_x;
        char* ddns_hostname_x;
        char* max_on_heavy;
        char* max_queues;
        char* max_on_ed2k;
        char* rfw_enable_x;
        char* device_type;
        char* dm_radio_time_x;
        char* dm_radio_time2_x;
        char* misc_seeding_x;
        char* misc_seeding_x_renew;

        enable_time = websGetVar(wp, "Enable_time", "");
        enable_time_renew = websGetVar(wp, "Enable_time_renew", "");
        start_hour = websGetVar(wp, "Start_hour", "");
        start_minute = websGetVar(wp, "Start_minute", "");
        end_hour = websGetVar(wp, "End_hour", "");
        end_minute = websGetVar(wp, "End_minute", "");
        day = websGetVar(wp, "Day", "");
        download_dir = websGetVar(wp, "Download_dir", "");
        refresh = websGetVar(wp, "Refresh_rate", "");
        base_path = websGetVar(wp, "Base_path", "");
        misc_http_x = websGetVar(wp, "Misc_http_x", "");
        miscr_httpport_x = websGetVar(wp, "Miscr_httpport_x", "");
        miscr_http_x = websGetVar(wp, "Miscr_http_x", "");
        lan_ip = websGetVar(wp, "Lan_ip", "");
        dm_port = websGetVar(wp, "DM_port", "");
        dm_port_renew = websGetVar(wp, "DM_port_renew", "");
        dm_https_port = websGetVar(wp, "DM_https_port", "");
        dm_https_port_renew = websGetVar(wp, "DM_https_port_renew", "");
        dm_dir_renew = websGetVar(wp, "DM_dir_renew", "");
        dm_language = websGetVar(wp, "DM_language", "");
        productid = websGetVar(wp, "Productid", "");
        apps_dev = websGetVar(wp, "APPS_DEV", "");
        wan_ip = websGetVar(wp, "WAN_IP", "");
        ddns_enable_x = websGetVar(wp, "DDNS_ENABLE_X", "");
        ddns_hostname_x = websGetVar(wp, "DDNS_HOSTNAME_X", "");
        max_on_heavy =	websGetVar(wp, "MAX_ON_HEAVY", "");
        max_queues = websGetVar(wp, "MAX_QUEUES", "");
        max_on_ed2k = websGetVar(wp, "MAX_ON_ED2K", "");
        rfw_enable_x = websGetVar(wp, "RFW_ENABLE_X", "");
        device_type = websGetVar(wp, "Device_type", "");
        dm_radio_time_x = websGetVar(wp, "dm_radio_time_x", "");
        dm_radio_time2_x = websGetVar(wp, "dm_radio_time2_x", "");
        misc_seeding_x= websGetVar(wp, "misc_seeding_x", "");
        misc_seeding_x_renew= websGetVar(wp, "misc_seeding_x_renew", "");

        char command[512];
        memset(command,'\0',sizeof(command));
        #ifdef DM_MIPSBIG
        system("/tmp/APPS/Lighttpd/Script/asus_check_general router-general-renew&");
        #else
        system("touch /tmp/asus_routergen_tag;/tmp/APPS/Lighttpd/Script/asus_check_general router-general-renew&");
        while(access("/tmp/asus_routergen_tag",0)==0);
        #endif

        char *routerconfig = getrouterconfig();
        if(routerconfig != NULL)
            free(routerconfig);


        char nvramcmd[256];

        if(access(download_dir,0) == 0)//download_dir is exists
		{
            struct disk_info *disks_tmp;
            disks_tmp = follow_disk_info_start;
            while(disks_tmp!=NULL)
            {
                if(disks_tmp->mountpath!=NULL)
                {
                    if((strncmp(disks_tmp->mountpath,download_dir,strlen(disks_tmp->mountpath)))==0)
                        break;
                    else
                        disks_tmp=disks_tmp->next;
                }
                else
                    disks_tmp=disks_tmp->next;
            }

            fp = fopen("/opt/etc/dm2_general.conf","w+");
			if( fp != NULL ){
              result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,disks_tmp->serialnum,disks_tmp->vendor,disks_tmp->product,disks_tmp->partitionport,misc_seeding_x,dm_https_port);
            fclose(fp);
            }

            fp = fopen("/opt/etc/dm2_general_bak.conf","w+");
            if( fp != NULL ){
                result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,disks_tmp->serialnum,disks_tmp->vendor,disks_tmp->product,disks_tmp->partitionport,misc_seeding_x,dm_https_port);
                fclose(fp);
            }

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"cp -rf /opt/etc/dm2_general.conf /tmp/APPS/DM2/Config/dm2_general.conf");
            system(nvramcmd);
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"cp -rf /opt/etc/dm2_general_bak.conf /tmp/APPS/DM2/Config/dm2_general_bak.conf");
            system(nvramcmd);
#ifdef DM_MIPSBIG
            if(mips_type==1)
            {
                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_serial \"%s\"",disks_tmp->serialnum);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_vonder %s",disks_tmp->vendor);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_product %s",disks_tmp->product);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_partition %d",disks_tmp->partitionport);
                system(nvramcmd);
            }
            else
            {
                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"nvram set gen_serial=\"%s\"",disks_tmp->serialnum);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"nvram set gen_vonder=%s",disks_tmp->vendor);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"nvram set gen_product=%s",disks_tmp->product);
                system(nvramcmd);

                memset(nvramcmd,'\0',sizeof(nvramcmd));
                sprintf(nvramcmd,"nvram set gen_partition=%d",disks_tmp->partitionport);
                system(nvramcmd);
            }
#else
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_serial=\"%s\"",disks_tmp->serialnum);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_vonder=%s",disks_tmp->vendor);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_product=%s",disks_tmp->product);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_partition=%d",disks_tmp->partitionport);
            system(nvramcmd);
#endif
        }
		else {
            struct disk_info *disks_tmp;
            disks_tmp = follow_disk_info_start;
            while(disks_tmp!=NULL)
            {
                if(disks_tmp->mountpath!=NULL)
                {
                    if((strncmp(disks_tmp->serialnum,gen_serial,strlen(gen_serial)) == 0)&&(strncmp(disks_tmp->vendor,gen_vonder,strlen(gen_vonder)) == 0)&&(strncmp(disks_tmp->product,gen_product,strlen(gen_product)) == 0)&&(disks_tmp->partitionport == gen_partition))
                        break;
                    else
                        disks_tmp = disks_tmp->next;
                }
                else
                    disks_tmp = disks_tmp->next;
            }
            if((disks_tmp)&&(disks_tmp->serialnum))//umount disk and then mount disk but name changed
            {
                char *folderpoint;
                folderpoint = my_nstrchr('/',download_dir,4);
                if(folderpoint)
                {
                    char download_dirtmp[strlen(folderpoint)+1];
                    memset(download_dirtmp,'\0',sizeof(download_dirtmp));
                    strncpy(download_dirtmp,folderpoint,strlen(folderpoint));
					download_dir=disks_tmp->mountpath;
                    strcat(download_dir,download_dirtmp);
                }
            fp = fopen("/opt/etc/dm2_general.conf","w+");
                if( fp != NULL ){
                  result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,disks_tmp->serialnum,disks_tmp->vendor,disks_tmp->product,disks_tmp->partitionport,misc_seeding_x,dm_https_port);

                fclose(fp);
                }

                fp = fopen("/opt/etc/dm2_general_bak.conf","w+");
                if( fp != NULL ){
                  result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,disks_tmp->serialnum,disks_tmp->vendor,disks_tmp->product,disks_tmp->partitionport,misc_seeding_x,dm_https_port);
                fclose(fp);
                }
            }
            else//umount disk and then do not mount again
            {
				fp = fopen("/opt/etc/dm2_general.conf","w+");
                if( fp != NULL ){
                  result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,gen_serial,gen_vonder,gen_product,gen_partition,misc_seeding_x,dm_https_port);
                fclose(fp);
                }

                fp = fopen("/opt/etc/dm2_general_bak.conf","w+");
                if( fp != NULL ){
                  result = fprintf(fp,"Enable_time=%s\nStart_hour=%s\nStart_minute=%s\nEnd_hour=%s\nEnd_minute=%s\nDay=%s\nDownload_dir=%s\nRefresh_rate=%s\n$MAINDIR=%s\n$EX_MAINDIR=%s\nEX_DOWNLOAD_PATH=%s\nBASE_PATH=%s\nMISC_HTTP_X=%s\nAPPS_DL_SHARE=1\nLAN_IP=%s\nMISCR_HTTPPORT_X=%s\nMISCR_HTTP_X=%s\nDM_PORT=%s\nLANGUAGE=%s\nPRODUCTID=%s\nAPPS_DEV=%s\nWAN_IP=%s\nDDNS_ENABLE_X=%s\nDDNS_HOSTNAME_X=%s\nMAX_ON_HEAVY=%s\nMAX_QUEUES=%s\nMAX_ON_ED2K=%s\nRFW_ENABLE_X=%s\nDEVICE_TYPE=%s\ndm_radio_time_x=%s\ndm_radio_time2_x=%s\nserial=%s\nvonder=%s\nproduct=%s\npartition=%d\nMISC_SEEDING_X=%s\nDM_HTTPS_PORT=%s\n",enable_time,start_hour,start_minute,end_hour,end_minute,day,download_dir,refresh,download_dir,base_path,download_dir,base_path,misc_http_x,lan_ip,miscr_httpport_x,miscr_http_x,dm_port,dm_language,productid,apps_dev,wan_ip,ddns_enable_x,ddns_hostname_x,max_on_heavy,max_queues,max_on_ed2k,rfw_enable_x,device_type,dm_radio_time_x,dm_radio_time2_x,gen_serial,gen_vonder,gen_product,gen_partition,misc_seeding_x,dm_https_port);
                fclose(fp);
                }
            }
        }

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"cp -rf /opt/etc/dm2_general.conf /tmp/APPS/DM2/Config/dm2_general.conf");
        system(nvramcmd);
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"cp -rf /opt/etc/dm2_general_bak.conf /tmp/APPS/DM2/Config/dm2_general_bak.conf");
        system(nvramcmd);

        //char nvramcmd[256];//eric added for nvram changed 2012.10.29{
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_refresh_rate %s",refresh);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_http_x %s",misc_http_x);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_lang %s",dm_language);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_time_begin %s",dm_radio_time_x);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_time_end %s",dm_radio_time2_x);
            system(nvramcmd);


            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_enable_time %s",enable_time);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_day %s",day);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_misc_seeding_x %s",misc_seeding_x);
            system(nvramcmd);
        }
        else
        {
            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_refresh_rate=%s",refresh);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_http_x=%s",misc_http_x);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_lang=%s",dm_language);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_time_begin=%s",dm_radio_time_x);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_time_end=%s",dm_radio_time2_x);
            system(nvramcmd);


            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_enable_time=%s",enable_time);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_day=%s",day);
            system(nvramcmd);

            memset(nvramcmd,'\0',sizeof(nvramcmd));
            sprintf(nvramcmd,"nvram set gen_misc_seeding_x=%s",misc_seeding_x);
            system(nvramcmd);
        }
#else
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_refresh_rate=%s",refresh);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_http_x=%s",misc_http_x);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_lang=%s",dm_language);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_time_begin=%s",dm_radio_time_x);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_time_end=%s",dm_radio_time2_x);
        system(nvramcmd);


        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_enable_time=%s",enable_time);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_day=%s",day);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set gen_misc_seeding_x=%s",misc_seeding_x);
        system(nvramcmd);
#endif
	if(!strcmp(enable_time_renew,"1")){
		system("killall -SIGUSR2 asus_lighttpd &");

        char *dmconfig = getdmconfig();
        if(dmconfig != NULL)
            free(dmconfig);

		int checktime=timecheck_item(nv_data,nv_time1,nv_time2); //2012.07.10 magic added for cross-night

		if(checktime==0){
			DM_CTRL("all_paused@","all","BT");
            		DM_CTRL("all_paused@","all","NZB");
            		DM_CTRL("all_paused@","all","HTTP");
			memset(command,0,sizeof(command));
#ifdef DM_I686
			sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"pause all\"","127.0.0.1");
#else
			sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"pause all\"",lan_ip_addr);
#endif
			system(command);
		}
		if(checktime==1){
			
			DM_CTRL("all_start@","all","BT");
            		DM_CTRL("all_start@","all","NZB");
            		DM_CTRL("all_start@","all","HTTP");
			memset(command,0,sizeof(command));
#ifdef DM_I686
			sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume all\"","127.0.0.1");
#else
			sprintf(command,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume all\"",lan_ip_addr);
#endif
			system(command);
		}
	}

    if(!strcmp(dm_dir_renew, "1") || !strcmp(misc_seeding_x_renew, "1")){
        char *download_dir_decode;
        char *base_path_decode;
        download_dir_decode = download_dir;

        base_path_decode = Base_dir; //20120821 magic modify for new config
        decode_path(download_dir_decode);
        decode_path(base_path_decode);

        system("killall -SIGUSR1 dm2_snarfmaster &");
        system("killall -SIGINT dm2_nzbget &");
        system("killall -SIGTERM dm2_amuled &");
    }

#ifdef DM_MIPSBIG
    system("/tmp/APPS/DM2/Script/S50downloadmaster firewall-restart &");
    while(access("/tmp/dm2_firewall_tag",0)!=0){
    }
#else      
    system("touch /tmp/dm2_firewallres_tag;/tmp/APPS/DM2/Script/S50downloadmaster firewall-restart &");
    while(access("/tmp/dm2_firewallres_tag",0)==0){
    }

#endif
    if(!strcmp(dm_dir_renew, "1") && !strcmp(misc_seeding_x_renew, "1")){
        Dir_renew_socket("all");
        system("/tmp/APPS/DM2/Script/S50downloadmaster dir-change all &");
        memset(nvramcmd,'\0',sizeof(nvramcmd));//eric added for nvram changed 2012.10.29{
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_dl_dir %s",download_dir);
        }
        else
        {
            sprintf(nvramcmd,"nvram set gen_dl_dir=%s",download_dir);
        }
#else
        sprintf(nvramcmd,"nvram set gen_dl_dir=%s",download_dir);
#endif
        system(nvramcmd);//eric added for nvram changed 2012.10.29}
    }
    else if(!strcmp(dm_dir_renew, "1") && !strcmp(misc_seeding_x_renew, "0")){
        Dir_renew_socket("dir");
        system("/tmp/APPS/DM2/Script/S50downloadmaster dir-change dir &");
        memset(nvramcmd,'\0',sizeof(nvramcmd));//eric added for nvram changed 2012.10.29{
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_dl_dir %s",download_dir);
        }
        else
        {
            sprintf(nvramcmd,"nvram set gen_dl_dir=%s",download_dir);
        }
#else
        sprintf(nvramcmd,"nvram set gen_dl_dir=%s",download_dir);
#endif
        system(nvramcmd);//eric added for nvram changed 2012.10.29}
    }
    else if(!strcmp(dm_dir_renew, "0") && !strcmp(misc_seeding_x_renew, "1")){
        Dir_renew_socket("seeding");
        system("/tmp/APPS/DM2/Script/S50downloadmaster dir-change seeding &");
    }

    if(!strcmp(dm_port_renew, "1")){
        char changport[256];
        memset(changport,'\0',sizeof(changport));
        sprintf(changport,"sed -i '96s/^.*$/server.port = %s/' /opt/etc/asus_lighttpd.conf",dm_port);
        system(changport);
        sleep(1);
        char nvramcmd[32];
        memset(nvramcmd,'\0',sizeof(nvramcmd));
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry dm_http_port %s",dm_port);
        }
        else
        {
            sprintf(nvramcmd,"nvram set dm_http_port=%s",dm_port);
        }
#else
        sprintf(nvramcmd,"nvram set dm_http_port=%s",dm_port);
#endif
        system(nvramcmd);
        //system("nvram commit");//eric delete for nvram changed 2012.10.29
        //system("/tmp/APPS/DM2/Script/S50downloadmaster lighttpd-restart &");
    }
    if(!strcmp(dm_https_port_renew, "1")){
        char changport2[256];
        memset(changport2,'\0',sizeof(changport2));
        sprintf(changport2,"sed -i '413s/^.*$/$SERVER[\"socket\"] == \":%s\" {/' /opt/etc/asus_lighttpd.conf",dm_https_port);
        system(changport2);
        sleep(1);
        char nvramcmd2[32];
        memset(nvramcmd2,'\0',sizeof(nvramcmd2));
#ifdef DM_MIPSBIG
        if(mips_type==1)
        {
            sprintf(nvramcmd2,"/userfs/bin/tcapi set Apps_Entry dm_https_port %s",dm_https_port);
        }
        else
        {
            sprintf(nvramcmd2,"nvram set dm_https_port=%s",dm_https_port);
        }
#else
        sprintf(nvramcmd2,"nvram set dm_https_port=%s",dm_https_port);
#endif
        system(nvramcmd2);
    }
    if(!strcmp(dm_port_renew, "1") || !strcmp(dm_https_port_renew, "1"))
    {
        system("/tmp/APPS/DM2/Script/S50downloadmaster lighttpd-restart &");

    }

    printf("ACK_SUCESS");
#ifdef DM_MIPSBIG
    if(mips_type==1)
    {
        system("/userfs/bin/tcapi commit Apps");
        system("/userfs/bin/tcapi save");//eric added for nvram changed 2012.10.29
    }
    else{
        system("nvram commit");
        //system("/tmp/APPS/DM2/Script/S50downloadmaster firewall-restart &");
    }
#else
    system("nvram commit");//eric added for nvram changed 2012.10.29
#endif
    }
}

int DM_ADD(char* cmd, char* url , char *real_url, char* d_type,struct Lognote *qhead)
{
	int sfd;
	int result = UNKNOW;
	char command[2048];
	struct sockaddr_in btaddr;
	char chk_tmp[2048];

	memset(command,'\0',sizeof(command));
	sprintf(command,"%s%s",cmd,real_url);
	memset(chk_tmp, 0x00, sizeof(chk_tmp));
	memset(&btaddr, '\0', sizeof(btaddr));

	check_alive();
	if(total_counts + on_heavy_counts + on_light_counts + on_nntp_counts + on_ed2k_counts< MAX_QUEUES){
		btaddr.sin_family = AF_INET;
		if(!strcmp(d_type, "3")){
			if(on_heavy_counts + on_ed2k_counts< MAX_ON_HEAVY){
				init_tmp_dst(chk_tmp, real_url, strlen(real_url));
				if(chk_on_process(chk_tmp, NULL) > 0){
					return BT_EXIST;
				}
				else if(checklognote(chk_tmp,qhead)>0){
					return BT_EXIST;
				}
				else if(on_hash_counts>0)
				{
					return NOW_HASHING;//2012.06.27 eric added
				}
				else{
					FILE *cgi_fp;
					cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
					fclose(cgi_fp);
					return BT_STOP;
				}
			}
			else{
				return HEAVY_FULL;
			}
		}
		else if(!strcmp(d_type, "1") || !strcmp(d_type, "2")){
			if(on_light_counts < MAX_ON_LIGHT){
				if(!strcmp(d_type, "2")){
					init_tmp_dst(chk_tmp, real_url, strlen(real_url));
					if(chk_on_process(chk_tmp, NULL) > 0){
						return BT_EXIST;
					}
					else if(checklognote(chk_tmp,qhead)>0){
						return BT_EXIST;
					}
				} else if(!strcmp(d_type, "1")){
					init_tmp_dst(chk_tmp, real_url, strlen(real_url));
					if(chk_on_process(chk_tmp, NULL) > 0){
						return BT_EXIST;
					}
					else if(checklognote(chk_tmp,qhead)>0){
						return BT_EXIST;
					}
				}
				//if (access("/tmp/APPS/DM2/Status/snarf_stop",0) == 0){
				FILE *cgi_fp;
				cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
				fclose(cgi_fp);
				return SNARF_STOP;
				//}else{
				//         	btaddr.sin_port = htons(SNARF_PORT);
				//}
			}
			else{
				return LIGHT_FULL;
			}
		}
		else if(!strcmp(d_type, "4")){
			btaddr.sin_port = htons(NZB_PORT);
		} else if(!strcmp(d_type, "6")) {//ed2k task check
			if(on_heavy_counts + on_ed2k_counts < MAX_ON_ED2K){
				if(checklognote(url, qhead)>0) {
					return BT_EXIST;
				}else if(on_hash_counts > 0) {
					return ED2K_FULL;
				} else {
					FILE *cgi_fp;
					cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
					fclose(cgi_fp);
					return AMULED_STOP;
				}
			}
			else{
				return ED2K_FULL;
			}
		}

		inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
		result =  ACK_SUCESS;

		if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			result = ACK_FAIL;
#ifndef DM_MIPSBIG
			return result;
#endif
		}

		if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
			result =  ACK_FAIL;
#ifndef DM_MIPSBIG
			return result;
#endif
		}

		if(write(sfd, command, strlen(command)) != strlen(command))
		{
			result =  ACK_FAIL;
#ifndef DM_MIPSBIG
			return result;
#endif
		}

		close(sfd);
		sleep(1);
		return result;
	}
	else{
		return TOTAL_FULL;
	}

}


int DM_CTRL(char* cmd, char* task_id_name , char* d_type)
{	    
    int sfd;
    int result = UNKNOW;
    char command[100];
    struct sockaddr_in btaddr;

    char id[10];
    memset(id,'\0',sizeof(id));
    if(!strcmp(task_id_name, "all")){
        sprintf(id,"%s",task_id_name);

    }
    else{
       sprintf(id,"%s",getlogid2(task_id_name));
    }

    memset(command,'\0',sizeof(command));
    sprintf(command,"%s%s",cmd,id);

    memset(&btaddr, '\0', sizeof(btaddr));
    btaddr.sin_family = AF_INET;
    if(!strcmp(d_type, "BT")){
        	btaddr.sin_port = htons(BT_PORT);
    }
    else if(!strcmp(d_type, "HTTP") || !strcmp(d_type, "FTP")){
        	btaddr.sin_port = htons(SNARF_PORT);
    }
    else if(!strcmp(d_type, "NZB")){
        	btaddr.sin_port = htons(NZB_PORT);
    }
    else if(!strcmp(d_type, "ED2K")){ //2011.10 magic added
        	
    }
    inet_pton(AF_INET, "127.0.0.1", &btaddr.sin_addr);
    result =  ACK_SUCESS;

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        result = ACK_FAIL;
            return result;
    }

    if(connect(sfd, (struct sockaddr*)&btaddr, sizeof(btaddr)) < 0){
        result =  ACK_FAIL;
            return result;
    }
    if(write(sfd, command, strlen(command)) != strlen(command))
    {
        result =  ACK_FAIL;
         return result;
    }
    close(sfd);
    sleep(1);
    return result;

}


void print_log(struct Lognote *phead)
{
    struct Lognote *au = (struct Lognote*)0;
    for(au=phead; au; au=au->next){
        fprintf(stderr, "\nprint_log\n");
        fprintf(stderr,"id = %d\n", au->id);
        fprintf(stderr,"url = %s\n", au->url);
        fprintf(stderr,"filenum = %s\n", au->filenum);
        fprintf(stderr,"type = %s\n", au->type);
    }
}

/* create a notet*/
//struct Lognote * createnote(int id, char *url, int status)
struct Lognote * createnote(int id, char *infohash,char *url, char *real_url, char *filenums, char *d_type, int status ,int times)
{
    struct Lognote *note = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(note, 0, sizeof(struct Lognote *));
    note->id = id;
    strcpy(note->url, url);

    strcpy(note->real_url, real_url);
    strcpy(note->filenum, filenums);
    strcpy(note->type, d_type);
    note->status = status;
    strcpy(note->infohash, infohash);
    note->checknum = times;
    return note;
}

/* insert a note to list*/
void insertnote(struct Lognote *note, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p2 = phead;
    p1 = phead->next;

    while(p1 != NULL)
    {
        if((note->id < p1->id) && (note->id > p2->id))
        {
            break;
        }
        p2 = p1;
        p1 = p1->next;
    }

    p2->next = note;
    note->next = p1;
}

void initlognote(struct Lognote *phead)
{
	total_counts=0;
    int fd_r;
    struct Lognote au;
    jqs_sem_use = 1;
    if(Sem_open(&jqs_sem, jqs_dir, 0) < 0)
    {
        jqs_sem_use = 0;
    }

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);

    if((fd_r = open(jqs_file, O_RDONLY)) < 0)
    {
        return;
    }

    memset(&au, 0, sizeof(struct Lognote));
    while(read(fd_r, &au, sizeof(struct Lognote)) > 0)
    {

        struct Lognote *note = createnote(au.id,au.infohash, au.url, au.real_url, au.filenum, au.type, au.status ,au.checknum);
        insertnote(note, phead);
	 if(strcmp(note->url,"")){
		total_counts++;
	}
	memset(&au, 0x00, sizeof(struct Lognote));
    }
    close(fd_r);

    if(jqs_sem_use)
        Sem_post(&jqs_sem);
    if(jqs_sem_use)
        Sem_close(&jqs_sem);
}

void refresh_jqs(struct Lognote *phead){
    int fd;
    int flag = O_CREAT|O_RDWR|O_TRUNC;
    struct Lognote *pau = (struct Lognote*)0;

    if(jqs_sem_use)
        Sem_wait(&jqs_sem);

    if((fd = open(jqs_file, flag)) < 0){
        if(jqs_sem_use)
            Sem_post(&jqs_sem);
        return;
    }

    for(pau = phead; pau; pau = pau->next){

        if(strcmp(pau->url,"")){
            write(fd, pau, sizeof(struct Lognote));
        }
        else{
        }
    }
    close(fd);
    if(jqs_sem_use)
        Sem_post(&jqs_sem);

}

/* create and add a note to list*/
int addlognote(char *url, char *infohash,char *real_url, char * filenums, struct Lognote *phead ,char *d_type)
{

    int i = 1;
    int j = 1;
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p2 = phead;
    p1 = phead->next;

    while(p1 != NULL)
    {
        //if(p1->id != i)
        //{
        //    break;
        //}
        //i++;
	j = p1->id;
	i = ++j;

        p2 = p1;
        p1 = p1->next;
    }

	if(!strncmp(url,"magnet:",7)){
		strcpy(real_url, url);
	}

    struct Lognote *note = createnote(i, infohash,url, real_url ,filenums, d_type, S_NOTBEGIN, 1);

    p2->next = note;
    note->next = p1;

    //refresh_jqs(p2);
    refresh_jqs(head);
    return note->id;
}

int checklognote(char *url, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;
	
    int result=0;	

    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
	    result=1;
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }

    return result;
}


void dellognote(int id, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(p1->id == id)
        {
            p2->next = p1->next;
            free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    //refresh_jqs(p2);
    refresh_jqs(head);
}

void dellognote_byurl(char *url, struct Lognote *phead)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
            p2->next = p1->next;
            free(p1);
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
	refresh_jqs(head);
}

void updatelognote_byurl(char *url, struct Lognote *phead, char *D_type, int status)
{
    struct Lognote *p1, *p2;
    p1=p2=(struct Lognote *)malloc(sizeof(struct Lognote));
    memset(p1, 0, sizeof(struct Lognote));
    memset(p2, 0, sizeof(struct Lognote));
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        if(!strcmp(p1->url,url))
        {
			strncpy(p1->type, D_type, sizeof(p1->type));
			p1->status = status;
            break;
        }
        p2 = p1;
        p1 = p1->next;

    }
    refresh_jqs(head);
}


int check_url(UrlResource *rsrc)
{
    Url *u = NULL;
    u   = rsrc->url;
    int parser_status;
    switch (u->service_type) {
    case SERVICE_HTTP:
        parser_status = http_transfer(rsrc);

		if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
			return 1;
		else
		{
            if( strstr(u->full_url, "http://") ) {
                    u->full_url += 7;
            }

            u->full_url = strconcat("ftp://",u->full_url,NULL);

            Url *new_url = NULL;
            new_url = url_new();
            url_init(new_url,u->full_url);

            rsrc->url = new_url;

            parser_status = ftp_transfer(rsrc);

            if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
               return 1;
            else
               return -1;
        }
        break;
    case SERVICE_FTP:
        parser_status = ftp_transfer(rsrc);

        if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
            return 1;
        else
        {
            if( strstr(u->full_url, "ftp://") ) {
                    u->full_url += 6;
            }

            u->full_url = strconcat("http://",u->full_url,NULL);

            Url *new_url = NULL;
            new_url = url_new();
            url_init(new_url,u->full_url);

            rsrc->url = new_url;

            parser_status = http_transfer(rsrc);

            if( parser_status == P_PASS )
               return 1;
            else
               return -1;
        }
        break;
    case SERVICE_BT:
	u->status = P_PASS;
        return 1;
        break;
    case SERVICE_NZB:
	u->status = P_PASS;
        return 1;
        break;
    case SERVICE_ED2K:
	u->status = P_PASS;
        return 1;
        break;
    default:
        //rsrc->url->full_url = strconcat("http://",rsrc->url->full_url,NULL);
        /*parser_status = http_transfer(rsrc);

        if( parser_status == P_PASS)
            return 1;
        else
        {
            u->full_url += 7;

            u->full_url = strconcat("ftp://",u->full_url,NULL);

            parser_status = ftp_transfer(rsrc);

            if( parser_status == P_PASS || parser_status == P_NEEDACCOUNT)
               return 1;
            else
               return -1;
        }*/
        break;
    }
}

/*
 * the format of id is all or ed2k_xxx
 */
int clean_ed2k_complete(char *id){
	DIR  *log_dir;
	struct dirent  *log_ptr;

	if( strcmp(id, "all") == 0) {
		if((log_dir=opendir(Log_dir)) == NULL)
			return -1;
		while((log_ptr=readdir(log_dir)) != NULL)
		{
			if(strncmp(log_ptr->d_name, "ed2k_", 5) == 0) {
				if(CheckEd2kTaskComplete(log_ptr->d_name) > 0)
					RemoveLogbyID(log_ptr->d_name);
			}
		}
		closedir(log_dir);
	} else if (strncmp(id, "ed2k_", 5) == 0) {
		if(CheckEd2kTaskComplete(id) > 0)
			RemoveLogbyID(id);
	}

	return 0;
}

int CheckEd2kTaskComplete(char *task_id) {
	Log_struc  slog;
	memset(&slog, '\0', sizeof(slog));
	if(read_log(&slog, task_id) > 0) {
		if(slog.status == S_COMPLETED) {
			return 1;
		}
	}
	return 0;
}

void RemoveLogbyID(char *task_id) {
	char file_dst[128];
	memset(file_dst, '\0', sizeof(file_dst));
	snprintf(file_dst, sizeof(file_dst), "%s/%s", Log_dir,task_id);

	if(access(file_dst, F_OK) == 0) {
		char rm_cmd[128];
		memset(rm_cmd, '\0', sizeof(rm_cmd));
		snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", file_dst);
		system(rm_cmd);
	}
}

Url  *url_parser(char *url,  struct Lognote *head,char *again)
{
    //char *type = "5" ;
    if(!strcmp(again,"no")){
         addlognote(url, " ",url , "nonum", head,"5");
	}

    Url *src_url = NULL;
    Url *new_url = NULL;
    UrlResource *rsrc = NULL;

    rsrc = url_resource_new();
	src_url = url_new();
	//new_url = url_new();
    url_init(src_url,url);

    rsrc->url = src_url;
    check_url(rsrc);
    new_url = rsrc->url;

	return new_url;

}

struct disk_info *initial_disk_data(struct disk_info **disk)
{
    struct disk_info *follow_disk;

    if(disk == NULL)
        return NULL;

    *disk = (struct disk_info *)malloc(sizeof(struct disk_info));
    if(*disk == NULL)
        return NULL;

    follow_disk = *disk;

    follow_disk->diskname = NULL;
    follow_disk->diskpartition = NULL;
    follow_disk->mountpath = NULL;
    follow_disk->serialnum = NULL;
    follow_disk->product = NULL;
    follow_disk->vendor = NULL;
    follow_disk->next = NULL;
    follow_disk->port = (unsigned int)0;
    follow_disk->partitionport = (unsigned int )0;

    return follow_disk;
}

void free_disk_struc(struct disk_info **disk)
{
    struct disk_info *follow_disk, *old_disk;

    if(disk == NULL)
        return;

    follow_disk = *disk;
    while(follow_disk != NULL)
    {
        if(follow_disk->diskname != NULL)
            free(follow_disk->diskname);
        if(follow_disk->diskpartition != NULL)
            free(follow_disk->diskpartition);
        if(follow_disk->mountpath != NULL)
            free(follow_disk->mountpath);
        if(follow_disk->serialnum != NULL)
            free(follow_disk->serialnum);
        if(follow_disk->product != NULL)
            free(follow_disk->product);
        if(follow_disk->vendor != NULL)
            free(follow_disk->vendor);

        old_disk = follow_disk;
        follow_disk = follow_disk->next;
        free(old_disk);
    }
}

int init_diskinfo_struct()
{
	//20130626 magic {
	//system("rm -rf /tmp/usbinfo");
//    while(access("/tmp/usbinfo_lock",0)==0){
//            //sleep(1);
//    }
    int len;
    FILE *fp;
    while(access("/tmp/getdiskinfo_lock",F_OK) == 0)
    {
        usleep(50);
    }

    //unlink("/tmp/usbinfo_lock");
    //system("/tmp/APPS/DM2/Script/dm2_usbget start&");
    while(access("/tmp/usbinfo_lock",0)!=0){
            //sleep(1);
    }

    fp = fopen("/tmp/getdiskinfo_lock","w");
    fclose(fp);

    if(access("/tmp/usbinfo",0)==0)
    {
        fp =fopen("/tmp/usbinfo","r");
        if(fp)
        {
            fseek(fp,0,SEEK_END);
            len = ftell(fp);
            fseek(fp,0,SEEK_SET);
        }
        else
        {
            unlink("/tmp/getdiskinfo_lock");
            return -1;
        }
    }
    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    fread(buf,1,len,fp);
    fclose(fp);

    unlink("/tmp/getdiskinfo_lock");

    if(initial_disk_data(&follow_disk_tmp) == NULL){
        fprintf(stderr,"No memory!!(follow_disk_info)\n");
        return -1;
    }
    if(initial_disk_data(&follow_disk_info_start) == NULL){
        fprintf(stderr,"No memory!!(follow_disk_info)\n");
        return -1;
    }

    follow_disk_info = follow_disk_info_start;
    //get diskname and mountpath
    char a[1024];
    char *p,*q;
    fp = fopen("/proc/mounts","r");
    if(fp)
    {
        while(!feof(fp))
        {
            memset(a,'\0',sizeof(a));
            fscanf(fp,"%[^\n]%*c",a);
            if((strlen(a) != 0)&&((p=strstr(a,"/dev/sd")) != NULL))
            {
                singledisk++;
                //fprintf(stderr,"\nnum=%d\n",singledisk);
                if(singledisk != 1){
                    initial_disk_data(&follow_disk_tmp);
                }
                //fprintf(stderr,"\na=%s\n",a);
                p = p + 5;
                follow_disk_tmp->diskname=(char *)malloc(5);
                memset(follow_disk_tmp->diskname,'\0',5);
                strncpy(follow_disk_tmp->diskname,p,4);
                //fprintf(stderr,"\ndiskname=%s\n",follow_disk_tmp->diskname);

                p = p + 3;
                follow_disk_tmp->partitionport=atoi(p);
                //fprintf(stderr,"\npartitionport=%d\n",follow_disk_tmp->partitionport);
                if((q=strstr(p,"/tmp")) != NULL)
                {
                    if((p=strstr(q," ")) != NULL)
                    {
                        follow_disk_tmp->mountpath=(char *)malloc(strlen(q)-strlen(p)+1);
                        memset(follow_disk_tmp->mountpath,'\0',strlen(q)-strlen(p)+1);
                        strncpy(follow_disk_tmp->mountpath,q,strlen(q)-strlen(p));
                        //fprintf(stderr,"\nmountpath=%s\n",follow_disk_tmp->mountpath);
                    }
                }
                char diskname_tmp[4];
                memset(diskname_tmp,'\0',sizeof(diskname_tmp));
                strncpy(diskname_tmp,follow_disk_tmp->diskname,3);
                //fprintf(stderr,"\ndiskname=%s\n",follow_disk_tmp->diskname);
                if((p=strstr(buf,diskname_tmp)) != NULL)
                {
                    //fprintf(stderr,"\nbuf=%s\n",buf);
                    p = p - 6;
                    //fprintf(stderr,"\np=%c\n",*p);
                    follow_disk_tmp->port=atoi(p);
                    follow_disk_tmp->diskpartition=(char *)malloc(4);
                    memset(follow_disk_tmp->diskpartition,'\0',4);
                    strncpy(follow_disk_tmp->diskpartition,diskname_tmp,3);
                    //fprintf(stderr,"\ndiskpartition=%s\n",follow_disk_tmp->diskpartition);
                    q=strstr(p,"_serial");
                    q = q + 8;
                    p=strstr(q,"_pid");
                    follow_disk_tmp->serialnum=(char *)malloc(strlen(q)-strlen(p)-4);
                    memset(follow_disk_tmp->serialnum,'\0',strlen(q)-strlen(p)-4);
                    strncpy(follow_disk_tmp->serialnum,q,strlen(q)-strlen(p)-5);
                    //fprintf(stderr,"\nserialnum=%s\n",follow_disk_tmp->serialnum);
                    p = p + 5;
                    q=strstr(p,"_vid");
                    follow_disk_tmp->product=(char *)malloc(strlen(p)-strlen(q)-4);
                    memset(follow_disk_tmp->product,'\0',5);
                    strncpy(follow_disk_tmp->product,p,strlen(p)-strlen(q)-5);
                    //fprintf(stderr,"\nproduct=%s\n",follow_disk_tmp->product);
                    q = q + 5;
                    follow_disk_tmp->vendor=(char *)malloc(5);
                    memset(follow_disk_tmp->vendor,'\0',5);
                    strncpy(follow_disk_tmp->vendor,q,4);
                    //fprintf(stderr,"\nvendor=%s\n",follow_disk_tmp->vendor);
                }
                else
                {
                    return -1;
                }

                follow_disk_info->next =  follow_disk_tmp;
                follow_disk_info = follow_disk_tmp;
            }
        }
        fclose(fp);
    }
    else
    {
        return -1;
    }
    return 0;
}

int check_url_type(const char *url)
{
    if(strncmp(url, "thunder://", strlen("thunder://")) == 0 || strncmp(url, "Thunder://", strlen("Thunder://")) == 0)
        return 1;
    else if(strncmp(url, "qqdl://", strlen("qqdl://")) == 0)
        return 2;
    else if(strncmp(url, "Flashget://", strlen("flashget://")) == 0)
        return 3;
    else
        return 0;
}

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static char find_pos(char ch)
{
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[]
    return (ptr - base);
}
/* */
char *base64_decode(const char *data, int data_len)
{
    int ret_len = (data_len / 4) * 3;
    int equal_count = 0;
    char *ret = NULL;
    char *f = NULL;
    int tmp = 0;
    int temp = 0;
    char need[3];
    int prepare = 0;
    int i = 0;
    if (*(data + data_len - 1) == '=')
    {
        equal_count += 1;
    }
    if (*(data + data_len - 2) == '=')
    {
        equal_count += 1;
    }
    if (*(data + data_len - 3) == '=')
    {//seems impossible
        equal_count += 1;
    }
    switch (equal_count)
    {
    case 0:
        ret_len += 4;//3 + 1 [1 for NULL]
        break;
    case 1:
        ret_len += 4;//Ceil((6*3)/8)+1
        break;
    case 2:
        ret_len += 3;//Ceil((6*2)/8)+1
        break;
    case 3:
        ret_len += 2;//Ceil((6*1)/8)+1
        break;
    }
    ret = (char *)malloc(ret_len);
    if (ret == NULL)
    {
        printf("No enough memory.\n");
        exit(0);
    }
    memset(ret, 0, ret_len);
    f = ret;
    while (tmp < (data_len - equal_count))
    {
        temp = 0;
        prepare = 0;
        memset(need, 0, 4);
        while (temp < 4)
        {
            if (tmp >= (data_len - equal_count))
            {
                break;
            }
            prepare = (prepare << 6) | (find_pos(data[tmp]));
            temp++;
            tmp++;
        }
        prepare = prepare << ((4-temp) * 6);
        for (i=0; i<3 ;i++ )
        {
            if (i == temp)
            {
                break;
            }
            *f = (char)((prepare>>((2-i)*8)) & 0xFF);
            f++;
        }
    }
    *f = '\0';
    return ret;
}

int main(void){
    int ctrl_result=0;
    int ctrl_result1=0;
    int ctrl_result2=0;
    int ctrl_result3=0;
    int ctrl_result4=0;
#ifdef DM_MIPSBIG
	check_mips_type();
#endif

    printf("ContentType:text/html\r\n");
    printf("Cache-Control:private,max-age=0;\r\n");
    printf("\r\n");

    char *data;

    data = getenv("QUERY_STRING");
    init_cgi(data);	// there wasn't '?' in the head.
    char *value;
    char *url;
    char *type;
    char *current_url;
    char *next_url;
    char *next_host;
    char *script;
    char *again; //yes or no;
    char *fid;
    char *path;
    char *new_floder_name;
    char *dm_lang;


    url = websGetVar(wp, "usb_dm_url", "");
    type = websGetVar(wp, "download_type", "");
    value = websGetVar(data,"action_mode", "");
    next_host = websGetVar(wp, "next_host", "");
    current_url = websGetVar(wp, "current_page", "");
    next_url = websGetVar(wp, "next_page", "");
    script = websGetVar(wp, "action_script","");
    again = websGetVar(wp, "again","");
    fid = websGetVar(wp, "fid","");

    path = websGetVar(wp, "path", "");
    new_floder_name = websGetVar(wp, "new_floder_name", "");

    init_diskinfo_struct();

    if(*(url+strlen(url)-1) == 10)
    {
        *(url+strlen(url)-1) = '\0';
    }//2012.06.13 eric added for url content \0

	int url_type = check_url_type(url);
	long len = 0;
	char *dec = NULL, *url_point = NULL, *url_tmp = NULL, *url_escape = NULL;
	switch(url_type)
	{
	case 0://normal url
		break;
	case 1://thunder download url
	{
		url_point = url + 10;
		len = strlen(url_point);
		dec = base64_decode(url_point, len);
		url_tmp = (char *)malloc(strlen(dec));
		memset(url_tmp, 0, sizeof(url_tmp));
		snprintf(url_tmp, strlen(dec) - 3, "%s", dec + 2);
		url_escape = oauth_url_unescape(url_tmp, NULL);
		memset(url, 0, sizeof(url));
		snprintf(url, strlen(url_escape) + 1, "%s", url_escape);
		if(dec)
			free(dec);
		if(url_tmp)
			free(url_tmp);
		if(url_escape)
			free(url_escape);
		break;
	}
	case 2://qq download url
	{
		url_point = url + 7;
		len = strlen(url_point);
		dec = base64_decode(url_point, len);
		url_tmp = (char *)malloc(strlen(dec));
		memset(url_tmp, 0, sizeof(url_tmp));
		snprintf(url_tmp, strlen(dec) + 1, "%s", dec);
		url_escape = oauth_url_unescape(url_tmp, NULL);
		memset(url, 0, sizeof(url));
		snprintf(url, strlen(url_escape) + 1, "%s", url_escape);
		if(dec)
			free(dec);
		if(url_tmp)
			free(url_tmp);
		if(url_escape)
			free(url_escape);
		break;
	}
	case 3://flashget download url
	{
		url_point = url + 11;
		len = strlen(url_point);
		dec = base64_decode(url_point, len);
		url_tmp = (char *)malloc(strlen(dec));
		memset(url_tmp, 0, sizeof(url_tmp));
		snprintf(url_tmp, strlen(dec) - 19, "%s", dec + 10);
		url_escape = oauth_url_unescape(url_tmp, NULL);
		memset(url, 0, sizeof(url));
		snprintf(url, strlen(url_escape) + 1, "%s", url_escape);
		if(dec)
			free(dec);
		if(url_tmp)
			free(url_tmp);
		if(url_escape)
			free(url_escape);
		break;
	}
	default:
		break;
	}

    char chk_tmp[2048];
    memset(chk_tmp, 0x00, sizeof(chk_tmp));

    head = (struct Lognote *)malloc(sizeof(struct Lognote));
    memset(head, 0, sizeof(struct Lognote));

    char dm_ctrl[10],task_id[20];
    memset(dm_ctrl, '\0', sizeof(dm_ctrl));
    memset(task_id, '\0', sizeof(task_id));

    init_path();
    //2016.8.22 tina modify{
    //getdmconfig();
    char *dmconfig = getdmconfig();
    if(dmconfig != NULL)
        free(dmconfig);
    //}end tina
    //initlognote(head);
	check_alive();
    if(!strcmp(value,"initial"))
    {
        print_apply(type);
        return 0;
    }
    else if(!strcmp(value,"DM_ADD_FOLDER")){

        int status;
        if(!access(path,0))
            printf("folder exists");
        else
        {
            status = mkdir(path,0777);
            if(status == 0)
                printf("success");
            else
            {
                printf("error");
            }
        }
        return 0;

        }
    else if(!strcmp(value,"DM_APPLY")){
        DM_APPLY(type);
        return 0;
    } else if(!strcmp(value, "DM_ED2K_ADD")) {
        char DM_ED2K_ADD[128];
        char *ed2k_server_ip;
        char *ed2k_server_port;

        memset(DM_ED2K_ADD, '\0', sizeof(DM_ED2K_ADD));
        ed2k_server_ip = websGetVar(wp, "ED2K_SERVER_IP", "");
        ed2k_server_port = websGetVar(wp, "ED2K_SERVER_PORT", "");

        snprintf(DM_ED2K_ADD, sizeof(DM_ED2K_ADD), \
                 "/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"add ed2k://|server|%s|%s|\"", \
                 ed2k_server_ip,ed2k_server_port);
        system(DM_ED2K_ADD);

        printf("ACK_SUCESS");
        return 0;

    } else if(!strcmp(value, "DM_ED2K_REM")) {
        char DM_ED2K_REM[128];
        char *ed2k_server_ip;
        char *ed2k_server_port;
        char *remove_ip;

        memset(DM_ED2K_REM, '\0', sizeof(DM_ED2K_REM));
        ed2k_server_ip = websGetVar(wp, "ED2K_SERVER_IP", "");
        ed2k_server_port = websGetVar(wp, "ED2K_SERVER_PORT", "");
        remove_ip = websGetVar(wp, "ED2K_SERVER_REMOVED", "");
        if(!strncmp(remove_ip, "removed", 7)) {
            if(!access("/tmp/APPS/DM2/Config/dm2_amule/server_removed",F_OK)) {
                FILE *fp_change;
                int i=0,num=-1, max=0;
                char tmp[20][30];
                memset(tmp, '\0', sizeof(tmp));
                if(fp_change = fopen("/tmp/APPS/DM2/Config/dm2_amule/server_removed", "r"))
                {

                    //get which line have the same ip, and max line;
                    for(i = 0; i<20; i++)
                    {
                        if(fgets(tmp[i], 30, fp_change) != NULL)
                        {
                            int len = strlen(tmp[i]);
                            if(len>4) {
                                // get same ip, note it num
                                if(!strncmp(tmp[i]+3, ed2k_server_ip, len-4))
                                    num = i;
                            }
                        } else {
                            max = i;
                            break;
                        }
                    }
                    fclose(fp_change);
                }
                if(fp_change = fopen("/tmp/APPS/DM2/Config/dm2_amule/server_removed", "w+"))
                {
                    if(max == 2)
                    {
                        unlink("/tmp/APPS/DM2/Config/dm2_amule/server_removed");
                    }
                    for(i = 0; i< max; i++)
                    {
                        if(num != i) {
                            fwrite(tmp[i], strlen(tmp[i]), 1, fp_change);
                        } else {
                            i++;//jump port
                        }
                    }
                }
            }
        } else {

            snprintf(DM_ED2K_REM, sizeof(DM_ED2K_REM), \
                     "/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"remove %s:%s\"", \
                     ed2k_server_ip,ed2k_server_port);
            system(DM_ED2K_REM);
        }
        printf("ACK_SUCESS");
        return 0;

    } else if(!strcmp(value, "DM_ED2K_CON")) {
        char DM_ED2K_CON[128];
        char *ed2k_server_ip;
        char *ed2k_server_port;

        memset(DM_ED2K_CON, '\0', sizeof(DM_ED2K_CON));
        ed2k_server_ip = websGetVar(wp, "ED2K_SERVER_IP", "");
        ed2k_server_port = websGetVar(wp, "ED2K_SERVER_PORT", "");

        snprintf(DM_ED2K_CON, sizeof(DM_ED2K_CON), \
                 "/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"connect %s:%s\"", \
                 ed2k_server_ip,ed2k_server_port);
        system(DM_ED2K_CON);
        system("rm /tmp/dm2_amule_timedout");
        char nvramcmd[256];
#ifdef DM_MIPSBIG
					if(mips_type==1)
					{
					memset(nvramcmd,'\0',sizeof(nvramcmd));
                                        sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry ed2k_ip %s",ed2k_server_ip);
					system(nvramcmd);

					memset(nvramcmd,'\0',sizeof(nvramcmd));
                                        sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry ed2k_port %s",ed2k_server_port);
					system(nvramcmd);

					system("/userfs/bin/tcapi commit Apps");
					system("/userfs/bin/tcapi save");//eric added for nvram change 2012.10.29
					}
					else
					{
        			memset(nvramcmd,'\0',sizeof(nvramcmd));
                                sprintf(nvramcmd,"nvram set ed2k_ip=%s",ed2k_server_ip);
        			system(nvramcmd);

        			memset(nvramcmd,'\0',sizeof(nvramcmd));
                                sprintf(nvramcmd,"nvram set ed2k_port=%s",ed2k_server_port);
        			system(nvramcmd);
        			system("nvram commit");//eric added for nvram change 2012.10.29
					}
#else
        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set ed2k_ip=%s",ed2k_server_ip);
        system(nvramcmd);

        memset(nvramcmd,'\0',sizeof(nvramcmd));
        sprintf(nvramcmd,"nvram set ed2k_port=%s",ed2k_server_port);
        system(nvramcmd);

        system("nvram commit");//eric added for nvram change 2012.10.29
#endif
        printf("ACK_SUCESS");
        return 0;
    } else if(!strcmp(value,"DM_ED2K_DISCON")) {
        char DM_ED2K_DISCON[128];
        memset(DM_ED2K_DISCON,0,sizeof(DM_ED2K_DISCON));

        snprintf(DM_ED2K_DISCON, sizeof(DM_ED2K_DISCON), \
                 "/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"disconnect\"");
        system(DM_ED2K_DISCON);

        printf("ACK_SUCESS");
        return 0;
	}
    else if(!strcmp(value,"DM_LANG")){
	    dm_lang = websGetVar(wp, "DM_language", "");
        char changlang[256];
        char nvramcmd[256];
        memset(nvramcmd,'\0',sizeof(nvramcmd));
	memset(changlang,'\0',sizeof(changlang));
        sprintf(changlang,"sed -i '19s/^.*$/LANGUAGE=%s/' /opt/etc/dm2_general.conf",dm_lang);
	system(changlang);
	system("cp -rf /opt/etc/dm2_general.conf /tmp/APPS/DM2/Config/dm2_general.conf");
	memset(changlang,'\0',sizeof(changlang));
        sprintf(changlang,"sed -i '19s/^.*$/LANGUAGE=%s/' /opt/etc/dm2_general_bak.conf",dm_lang);
	system(changlang);
	system("cp -rf /opt/etc/dm2_general_bak.conf /tmp/APPS/DM2/Config/dm2_general_bak.conf");
#ifdef DM_MIPSBIG
				if(mips_type==1)
				{
                                sprintf(nvramcmd,"/userfs/bin/tcapi set Apps_Entry gen_lang %s",dm_lang);
        			system(nvramcmd);//eric added for nvram change 2012.10.29
        			system("/userfs/bin/tcapi commit Apps");
					system("/userfs/bin/tcapi save");
				}
				else
				{
                        sprintf(nvramcmd,"nvram set gen_lang=%s",dm_lang);
        		system(nvramcmd);//eric added for nvram change 2012.10.29
        		system("nvram commit");
				}
#else
        sprintf(nvramcmd,"nvram set gen_lang=%s",dm_lang);
        system(nvramcmd);//eric added for nvram change 2012.10.29
        system("nvram commit");
#endif
	printf("ACK_SUCESS");
        return 0;
	}
	else if (!strcmp(value,"DM_ADD"))
	{
		system("killall dm2_detect&");
		system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected");
		initlognote(head);

		if(total_counts + on_heavy_counts + on_light_counts + on_nntp_counts + on_ed2k_counts< MAX_QUEUES){

			if(chk_on_process(url, NULL) > 0){
				printf("BT_EXIST");
			}
			else if(checklognote(url,head)>0){
				printf("BT_EXIST");
			}
			else{
				//add by gauss
				char *real_url;
				Url *new_url = NULL;
				new_url = url_parser(url,head,again);

				real_url = new_url->full_url;
				int checktime=timecheck_item(nv_data,nv_time1,nv_time2); //2012.07.10 magic added for cross-night

				if(new_url->status == P_PASS )
				{
					if(checktime==1){
						if(!strcmp(again,"no")){
							dellognote_byurl(url,head);
						} else {
							int fid_tmp=atoi(fid);
							dellognote(fid_tmp,head);
						}
						switch (new_url->service_type)
						{
						case SERVICE_HTTP:
							type = "1";
							break;
						case SERVICE_FTP:
							type = "2";
							break;
						case SERVICE_BT:
							type = "3";
							break;
						case SERVICE_NZB:
							type = "4";
							break;
						case SERVICE_ED2K:
							type = "6";
							break;
						default:
							type = "5";
							break;
						}
						ctrl_result=DM_ADD("add@0@",url,real_url,type,head);
						switch(ctrl_result)
						{
						case ACK_SUCESS:
							printf("ACK_SUCESS");
							break;
						case ACK_FAIL:
							printf("ACK_FAIL");
							break;
						case BT_EXIST:
							printf("BT_EXIST");
							break;
						case NOW_HASHING://2012.06.27 eric added
						case HEAVY_FULL:
							init_tmp_dst(chk_tmp, url, strlen(url));
							if(chk_on_process(chk_tmp, NULL) > 0){
								printf("BT_EXIST");
							}
							else if(checklognote(url,head)>0){
								printf("BT_EXIST");
							}
							else{
								addlognote(url," ",real_url,"nonum",head,type);
							}
							break;
						case BT_STOP:
							addlognote(url," ",real_url,"nonum",head,type);
							unlink("/tmp/APPS/DM2/Status/cgi_running");
							printf("ACK_SUCESS");
							break;
						case SNARF_STOP:
							addlognote(url," ",real_url,"nonum",head,type);
							unlink("/tmp/APPS/DM2/Status/cgi_running");
							printf("ACK_SUCESS");
							break;
						case AMULED_STOP:
							addlognote(url," ",real_url,"nonum",head,type);
							unlink("/tmp/APPS/DM2/Status/cgi_running");
							printf("ACK_SUCESS");
							break;
						case LIGHT_FULL:
							init_tmp_dst(chk_tmp, url, strlen(url));
							if(chk_on_process(chk_tmp, NULL) > 0){
								printf("BT_EXIST");
							}
							else if(checklognote(url,head)>0){
								printf("BT_EXIST");
							}
							else{
								addlognote(url," ",real_url,"nonum",head,type);
							}
							break;
						case ED2K_FULL:
							if(chk_on_process(url, NULL) > 0){
								printf("BT_EXIST");
							}
							else if(checklognote(url,head)>0){
								printf("BT_EXIST");
							}
							else{
								addlognote(url," ",real_url,"nonum",head,type);
							}
							break;
						case TOTAL_FULL:
							printf("TOTAL_FULL");
							break;
						default:
							printf("UNKNOW");
							break;
						}
					} else {
						switch (new_url->service_type)
						{

						case SERVICE_HTTP:
							type = "1";
							break;
						case SERVICE_FTP:
							type = "2";
							break;
						case SERVICE_BT:
							type = "3";
							break;
						case SERVICE_NZB:
							type = "4";
							break;
						case SERVICE_ED2K:
							type = "6";
							break;
						default:
							type = "5";
							break;

						}
						updatelognote_byurl(url,head,type,S_NOTBEGIN);
						printf("ACK_SUCESS");
					}
				}
				else if(new_url->status == P_NEEDACCOUNT)
				{
					switch (new_url->service_type)
					{
					case SERVICE_HTTP:
						type = "1";
						break;
					case SERVICE_FTP:
						type = "2";
						break;
					default:
						type = "5";
						break;
					}
					updatelognote_byurl(url, head, type, S_NEEDACCOUNT);
					printf("ACK_SUCESS");
				}
				else if(new_url->status == P_FAIL)
				{
					updatelognote_byurl(url,head,"5",S_PARSER_FAIL);
					printf("ACK_SUCESS");
				}
			}
		} else {
			printf("TOTAL_FULL");
		}
	}
	else if (!strcmp(value,"DM_CTRL"))
	{
		system("killall dm2_detect&");
		system("rm -rf /tmp/APPS/DM2/Config/dm2_detect_protected");
		initlognote(head);
		sprintf(dm_ctrl,"%s",websGetVar(data,"dm_ctrl", ""));

		if(strcmp(dm_ctrl, "start") == 0){
			char command_start[60];
			int checktime=timecheck_item(nv_data,nv_time1,nv_time2); //2012.07.10 magic added for cross-night
			sprintf(task_id,"%s",websGetVar(data,"task_id", ""));
			FILE *cgi_fp;
			cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
			fclose(cgi_fp);
			int recall_engine=0;
			if(checktime==1){
				if((strstr(task_id, "ed2k_")) != NULL)
				{
					memset(command_start, '\0', sizeof(command_start));
					char task_id_tmp[16];
					memset(task_id_tmp, '\0', sizeof(task_id_tmp));
					strncpy(task_id_tmp,task_id+5,3);
					sprintf(command_start,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume %s\"",\
							"127.0.0.1",task_id_tmp);
					system(command_start);
					ctrl_result=ACK_SUCESS;
				} else {
					if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
						char recall_tr[1024];
						memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG
						if(mips_type==1)
						{
							sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
							system(recall_tr);
						} else {
							memset(recall_tr,'\0',sizeof(recall_tr));
							sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
							system(recall_tr);
							memset(recall_tr,'\0',sizeof(recall_tr));
							sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
							system(recall_tr);
							sleep(1);
							memset(recall_tr,'\0',sizeof(recall_tr));
							sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
							system(recall_tr);
						}
#else
						sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
						system(recall_tr);
#endif
						unlink("/tmp/APPS/DM2/Status/tr_stop");
						recall_engine=4;
					}
					if(recall_engine>0){
						sleep(recall_engine);
					}
					ctrl_result=DM_CTRL("start@",task_id,type);
				}
			} else{
				ctrl_result=ACK_LIMIT;
			}
			unlink("/tmp/APPS/DM2/Status/cgi_running");
		} else if(strcmp(dm_ctrl, "paused") == 0){
			sprintf(task_id,"%s",websGetVar(data,"task_id", ""));
			if((strstr(task_id, "ed2k_")) != NULL)
			{
				char command_pause[60],task_id_tmp[4];
				memset(command_pause, '\0', sizeof(command_pause));
				memset(task_id_tmp, '\0', sizeof(task_id_tmp));
				strncpy(task_id_tmp,task_id+5,3);
				sprintf(command_pause,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"pause %s\"",\
						"127.0.0.1",task_id_tmp);
				system(command_pause);
				ctrl_result=ACK_SUCESS;
			}
			else{
				ctrl_result=DM_CTRL("pause@",task_id,type);
			}
		} else if(strcmp(dm_ctrl, "cancel") == 0){
			sprintf(task_id,"%s",websGetVar(data,"task_id", ""));
			char command_cancel[60];

			if(strlen(task_id)<5 && ((strstr(task_id, "nzb_")) == NULL)){
				int id = atoi(task_id);
				dellognote(id,head);
				ctrl_result=ACK_SUCESS;
			} else if((strstr(task_id, "error_")) != NULL) {
				memset(command_cancel, '\0', sizeof(command_cancel));
				sprintf(command_cancel,"rm -rf %s/%s",Log_dir,task_id);
				system(command_cancel);
				ctrl_result=ACK_SUCESS;
			} else if((strstr(task_id, "ed2k_")) != NULL) {
				if (detect_process("dm2_amuled") == 0) {
					clean_ed2k_complete(task_id);
				} else {
					memset(command_cancel, '\0', sizeof(command_cancel));
					snprintf(command_cancel, sizeof(command_cancel),\
							 "/opt/bin/dm2_amulecmd -h %s -P admin -c \"cancel %s\"",\
							 "127.0.0.1", task_id+5);
					system(command_cancel);
					RemoveLogbyID(task_id);
				}
				ctrl_result=ACK_SUCESS;
			} else {
				FILE *cgi_fp;
				cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
				fclose(cgi_fp);
				int recall_engine=0;
				if (!strcmp(type,"BT") && (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0)){
					char recall_tr[1024];
					memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG
					if(mips_type==1)
					{
						sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
						system(recall_tr);
					}
					else{
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
						system(recall_tr);
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
						system(recall_tr);
						sleep(1);
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
						system(recall_tr);
					}
#else
					sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
#endif
					unlink("/tmp/APPS/DM2/Status/tr_stop");
					recall_engine=1;
				}

				if (!strcmp(type,"HTTP") || !strcmp(type,"FTP")){
					if(access("/tmp/APPS/DM2/Status/snarf_stop",0) == 0){
						system("cd /opt/bin && ./dm2_snarfmaster&");
						unlink("/tmp/APPS/DM2/Status/snarf_stop");
						recall_engine=1;
					}
				}

				if (!strcmp(type,"NZB") && (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0)){
					system("cd /opt/bin && ./dm2_nzbget -D&");
					unlink("/tmp/APPS/DM2/Status/nntp_stop");
					recall_engine=1;
				}
				if(recall_engine==1){
					sleep(2);
				}

				ctrl_result=DM_CTRL("cancel@",task_id,type);
				unlink("/tmp/APPS/DM2/Status/cgi_running");
			}
		}
		else if(strcmp(dm_ctrl, "pause_all") == 0){
			if (access("/tmp/APPS/DM2/Status/tr_stop",F_OK) == 0)
				ctrl_result1=ACK_SUCESS;
			else
				ctrl_result1=DM_CTRL("all_paused@","all","BT");
			if(access("/tmp/APPS/DM2/Status/nntp_stop",F_OK) == 0)
				ctrl_result2=ACK_SUCESS;
			else
				ctrl_result2=DM_CTRL("all_paused@","all","NZB");
			if(access("/tmp/APPS/DM2/Status/snarf_stop",F_OK) == 0)
				ctrl_result3=ACK_SUCESS;
			else
				ctrl_result3=DM_CTRL("all_paused@","all","HTTP");
			if(access("/tmp/APPS/DM2/Status/ed2k_stop",F_OK) == 0) {
				ctrl_result4=ACK_SUCESS;
			} else {
				system("/opt/bin/dm2_amulecmd -h 127.0.0.1 -P admin -c \"pause all\"");
				ctrl_result4=ACK_SUCESS;
			}

			if((ctrl_result1 == ACK_FAIL) || (ctrl_result2 == ACK_FAIL)|| \
			   (ctrl_result3 == ACK_FAIL) || (ctrl_result4 == ACK_FAIL))
				ctrl_result = ACK_FAIL;
			else if ((ctrl_result1 == ACK_SUCESS) && (ctrl_result2 == ACK_SUCESS) &&\
					 (ctrl_result3 == ACK_SUCESS) && (ctrl_result4 == ACK_SUCESS))
				ctrl_result = ACK_SUCESS;
			else
				ctrl_result = UNKNOW;
		}
		else if(strcmp(dm_ctrl, "start_all") == 0){
			int checktime=timecheck_item(nv_data,nv_time1,nv_time2);
			char command_startall[60];
			FILE *cgi_fp;
			cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
			fclose(cgi_fp);
			int recall_engine=0;
			if(checktime==1){
				if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
					char recall_tr[1024];
					memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG
					if(mips_type==1)
					{
						sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
						system(recall_tr);
					}
					else{
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
						system(recall_tr);
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
						system(recall_tr);
						sleep(1);
						memset(recall_tr,'\0',sizeof(recall_tr));
						sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
						system(recall_tr);
					}
#else
					sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
#endif
					unlink("/tmp/APPS/DM2/Status/tr_stop");
					recall_engine=4;
				}
				if(recall_engine>0){
					sleep(recall_engine);
				}
				if (access("/tmp/APPS/DM2/Status/tr_stop",F_OK) == 0)
					ctrl_result1=ACK_SUCESS;
				else
					ctrl_result1=DM_CTRL("all_start@","all","BT");
				if(access("/tmp/APPS/DM2/Status/nntp_stop",F_OK) == 0)
					ctrl_result2=ACK_SUCESS;
				else
					ctrl_result2=DM_CTRL("all_start@","all","NZB");
				if(access("/tmp/APPS/DM2/Status/snarf_stop",F_OK) == 0)
					ctrl_result3=ACK_SUCESS;
				else
					ctrl_result3=DM_CTRL("all_start@","all","HTTP");
				if(access("/tmp/APPS/DM2/Status/ed2k_stop",F_OK) == 0) {
					ctrl_result4=ACK_SUCESS;
				} else {
					memset(command_startall, '\0', sizeof(command_startall));
					sprintf(command_startall,"/opt/bin/dm2_amulecmd -h %s -P admin -c \"resume all\"","127.0.0.1");
					system(command_startall);
					ctrl_result4=ACK_SUCESS;
				}


				if((ctrl_result1 == ACK_FAIL)||(ctrl_result2 == ACK_FAIL)||\
				   (ctrl_result3 == ACK_FAIL)||(ctrl_result4 == ACK_FAIL))
					ctrl_result = ACK_FAIL;
				else if ((ctrl_result1 == ACK_SUCESS)&&(ctrl_result2 == ACK_SUCESS)&&\
						 (ctrl_result3 == ACK_SUCESS)&&(ctrl_result4 == ACK_SUCESS))
					ctrl_result = ACK_SUCESS;
				else
					ctrl_result = UNKNOW;
			}
			else{
				ctrl_result=ACK_LIMIT;
			}
			unlink("/tmp/APPS/DM2/Status/cgi_running");
		} else if(strcmp(dm_ctrl, "clear") == 0) {
			FILE *cgi_fp;
			cgi_fp = fopen("/tmp/APPS/DM2/Status/cgi_running","w");
			fclose(cgi_fp);
			int recall_engine=0;
			if (access("/tmp/APPS/DM2/Status/snarf_stop",0) == 0){
				system("cd /opt/bin && ./dm2_snarfmaster&");
				unlink("/tmp/APPS/DM2/Status/snarf_stop");
				recall_engine=2;
			}
			if (access("/tmp/APPS/DM2/Status/nntp_stop",0) == 0){
				system("cd /opt/bin && ./dm2_nzbget -D&");
				unlink("/tmp/APPS/DM2/Status/nntp_stop");
				recall_engine=3;
			}
			if (access("/tmp/APPS/DM2/Status/tr_stop",0) == 0){
				char recall_tr[1024];
				memset(recall_tr,'\0',sizeof(recall_tr));
#ifdef DM_MIPSBIG
				if(mips_type==1)
				{
					sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
				}
				else{
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"ln -sf /lib/libc.so.0 /tmp/opt/lib/libc.so.0 && ln -sf /lib/libpthread.so.0 /tmp/opt/lib/libpthread.so.0");
					system(recall_tr);
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
					system(recall_tr);
					sleep(1);
					memset(recall_tr,'\0',sizeof(recall_tr));
					sprintf(recall_tr,"ln -sf /opt/lib/libuClibc-0.9.30.so /tmp/opt/lib/libc.so.0 && ln -sf /opt/lib/libpthread-0.9.30.so /tmp/opt/lib/libpthread.so.0");
					system(recall_tr);
				}
#else
				sprintf(recall_tr,"cd /opt/bin && ./dm2_transmission-daemon -w %s -g %s/Download2/config -G %s/Download2/ --incomplete-dir %s/Download2/InComplete&",Download_dir_path,Base_dir,Base_dir,Base_dir);
				system(recall_tr);
#endif
				unlink("/tmp/APPS/DM2/Status/tr_stop");
				recall_engine=4;
			}
			if(recall_engine>0){
				//fprintf(stderr,"\nrecall_engine=%d\n",recall_engine);
				sleep(recall_engine);
			}

			if (access("/tmp/APPS/DM2/Status/tr_stop",F_OK) == 0)
				ctrl_result1=ACK_SUCESS;
			else //neal added
				ctrl_result1=DM_CTRL("clean_completed@","all","BT");
			if (access("/tmp/APPS/DM2/Status/nntp_stop",F_OK) == 0)
				ctrl_result2=ACK_SUCESS;
			else //neal added
				ctrl_result2=DM_CTRL("clean_completed@","all","NZB");
			if (access("/tmp/APPS/DM2/Status/snarf_stop",F_OK) == 0)
				ctrl_result3=ACK_SUCESS;
			else //neal added
				ctrl_result3=DM_CTRL("clean_completed@","all","HTTP");

			clean_ed2k_complete("all");
			unlink("/tmp/APPS/DM2/Status/cgi_running");
			if((ctrl_result1 == ACK_FAIL)||(ctrl_result2 == ACK_FAIL)||(ctrl_result3 == ACK_FAIL))
				ctrl_result = ACK_FAIL;
			else if ((ctrl_result1 == ACK_SUCESS)&&(ctrl_result2 == ACK_SUCESS)&&(ctrl_result3 == ACK_SUCESS))
				ctrl_result = ACK_SUCESS;
			else
				ctrl_result = UNKNOW;
		}

		switch(ctrl_result)
		{
		case ACK_SUCESS:
			printf("ACK_SUCESS");
			break;
		case ACK_FAIL:
			printf("ACK_FAIL");
			break;
		case BT_EXIST:
			printf("BT_EXIST");
			break;
		case NOW_HASHING://2012.06.27 eric added
			printf("ACK_SUCESS");
			break;
		case HEAVY_FULL:
			printf("HEAVY_FULL");
			break;
		case LIGHT_FULL:
			printf("LIGHT_FULL");
			break;
		case NNTP_FULL:
			printf("NNTP_FULL");
			break;
		case ACK_LIMIT:
			printf("ACK_LIMIT");
			break;
		default:
			printf("UNKNOW");
			break;
		}
	}
	else if(!strcmp(value,"Lang_Hdr"))
    {
        FILE *fp;
#ifdef DM_MIPSBIG
		if(mips_type==1)
			fp = fopen("/boaroot/cgi-bin/Lang_Hdr","r");
		else
			fp = fopen("/www/Lang_Hdr","r");
#else
        fp = fopen("/www/Lang_Hdr","r");
#endif
		if(fp == NULL)
		{
			printf("LANG_EN,LANG_TW,LANG_CN,LANG_CZ,LANG_PL,LANG_RU,LANG_DE,LANG_FR,LANG_TR,LANG_TH,LANG_MS,LANG_NO,LANG_FI,LANG_DA,LANG_SV,LANG_BR,LANG_JP,LANG_ES,LANG_IT,LANG_UK,LANG_HU,LANG_RO");
		} else {
			char buf[64];
			char tmp[20];
			char bufLAN[512];
			char *p;
			memset(bufLAN, '\0', sizeof(bufLAN));
			while (fgets(buf, sizeof(buf), fp)!= NULL)
			{
				if((p = strstr(buf, "LANG_")) != NULL)
				{
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp,p,7);
					strcat(bufLAN, tmp);
					strcat(bufLAN, ",");
				}
			}
			fclose(fp);
			bufLAN[strlen(bufLAN) - 1] = '\0';
			printf("%s",bufLAN);
		}
	}

    free_disk_struc(&follow_disk_info_start);
    fflush(stdout);

    return 0;
}
