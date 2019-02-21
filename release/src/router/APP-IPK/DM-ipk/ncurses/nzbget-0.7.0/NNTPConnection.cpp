/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2004 Sven Henkel <sidddy@users.sourceforge.net>
 *  Copyright (C) 2007-2008 Andrei Prygounkov <hugbug@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Revision: 1.2 $
 * $Date: 2011/08/15 07:26:12 $
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include "win32.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <cstdio>

#include "nzbget.h"
#include "Log.h"
#include "NNTPConnection.h"
#include "Connection.h"
#include "NewsServer.h"

// gauss add 110720 {
#include "logs.h"
#include "Options.h"
#include "QueueCoordinator.h"
#include "PrePostProcessor.h"

extern Options* g_pOptions;
extern QueueCoordinator* g_pQueueCoordinator;
extern PrePostProcessor* g_pPrePostProcessor;
extern int force_stop_nzbget;
extern int error_time;
extern int have_error_log;
extern int force_cancle_nzb;
extern int force_pause_nzb;
extern int have_update_log;
#define ERROR_TIME 5
// gauss add 110720 }

static const int CONNECTION_LINEBUFFER_SIZE = 1024*10;

NNTPConnection::NNTPConnection(NewsServer* server) : Connection(server)
{
	m_szActiveGroup = NULL;
	m_szLineBuf = (char*)malloc(CONNECTION_LINEBUFFER_SIZE);
	m_bAuthError = false;
}

NNTPConnection::~NNTPConnection()
{
	if (m_szActiveGroup)
	{
		free(m_szActiveGroup);
		m_szActiveGroup = NULL;
	}
	free(m_szLineBuf);
}

const char* NNTPConnection::Request(const char* req)
{
	if (!req)
	{
		return NULL;
	}

	m_bAuthError = false;

	WriteLine(req);

	char* answer = ReadLine(m_szLineBuf, CONNECTION_LINEBUFFER_SIZE, NULL);

	if (!answer)
	{
		return NULL;
	}

	if (!strncmp(answer, "480", 3))
	{
		debug("%s requested authorization", m_pNetAddress->GetHost());

		//authentication required!
		if (!Authenticate())
		{
			m_bAuthError = true;
			return NULL;
		}

		//try again
		WriteLine(req);
		answer = ReadLine(m_szLineBuf, CONNECTION_LINEBUFFER_SIZE, NULL);
		return answer;
	}

	return answer;
}

bool NNTPConnection::Authenticate()
{
	bool ret ;
	if (!((NewsServer*)m_pNetAddress)->GetUser() ||
		!((NewsServer*)m_pNetAddress)->GetPassword())
	{
		return true;
	}

        //return AuthInfoUser();	// gauss changed 110720{
	ret = AuthInfoUser();
	if(!ret)
        {
            error_time ++;
            //info("Account error_time is %d",error_time);
            if(error_time > ERROR_TIME){
                    ReportErrorAnswer("Connecting to %s failed ", "Account failed");
                    //force_stop_nzbget = 1;
            }
               //force_stop_nzbget = 1;
        }
	return ret;
        // gauss 110720}
}

bool NNTPConnection::AuthInfoUser(int iRecur)
{
	if (iRecur > 10)
	{
		return false;
	}

	char tmp[1024];
	snprintf(tmp, 1024, "AUTHINFO USER %s\r\n", ((NewsServer*)m_pNetAddress)->GetUser());
	tmp[1024-1] = '\0';

	WriteLine(tmp);

	char* answer = ReadLine(m_szLineBuf, CONNECTION_LINEBUFFER_SIZE, NULL);
	if (!answer)
	{
		ReportError("Authorization for %s failed: Connection closed by remote host.", m_pNetAddress->GetHost(), true, 0);
		return false;
	}

	if (!strncmp(answer, "281", 3))
	{
		debug("Authorization for %s successful", m_pNetAddress->GetHost());
		return true;
	}
	else if (!strncmp(answer, "381", 3))
	{
		return AuthInfoPass(++iRecur);
	}
	else if (!strncmp(answer, "480", 3))
	{
		return AuthInfoUser(++iRecur);
	}

	if (char* p = strrchr(answer, '\r')) *p = '\0'; // remove last CRLF from error message

	if (GetStatus() != csCancelled)
	{
                //ReportErrorAnswer("Authorization for %s failed (Answer: %s)", answer);
	}
	return false;
}

bool NNTPConnection::AuthInfoPass(int iRecur)
{
	if (iRecur > 10)
	{
		return false;
	}

	char tmp[1024];
	snprintf(tmp, 1024, "AUTHINFO PASS %s\r\n", ((NewsServer*)m_pNetAddress)->GetPassword());
	tmp[1024-1] = '\0';

	WriteLine(tmp);

	char* answer = ReadLine(m_szLineBuf, CONNECTION_LINEBUFFER_SIZE, NULL);
	if (!answer)
	{
		ReportError("Authorization for %s failed: Connection closed by remote host.", m_pNetAddress->GetHost(), true, 0);
		return false;
	}
	else if (!strncmp(answer, "2", 1))
	{
		debug("Authorization for %s successful", m_pNetAddress->GetHost());
		return true;
	}
	else if (!strncmp(answer, "381", 3))
	{
		return AuthInfoPass(++iRecur);
	}

	if (char* p = strrchr(answer, '\r')) *p = '\0'; // remove last CRLF from error message

	if (GetStatus() != csCancelled)
	{
                //ReportErrorAnswer("Authorization for %s failed (Answer: %s)", answer);
	}
	return false;
}

const char* NNTPConnection::JoinGroup(const char* grp)
{
	if (m_szActiveGroup && !strcmp(m_szActiveGroup, grp))
	{
		// already in group
		strcpy(m_szLineBuf, "211 ");
		return m_szLineBuf;
	}

	char tmp[1024];
	snprintf(tmp, 1024, "GROUP %s\r\n", grp);
	tmp[1024-1] = '\0';

	const char* answer = Request(tmp);
	if (m_bAuthError)
	{
		return answer;
	}

	if (answer && !strncmp(answer, "2", 1))
	{
		debug("Changed group to %s on %s", grp, GetServer()->GetHost());

		if (m_szActiveGroup)
		{
			free(m_szActiveGroup);
		}
		m_szActiveGroup = strdup(grp);
	}
	else
	{
		debug("Error changing group on %s to %s: %s.",
			 GetServer()->GetHost(), grp, answer);
	}

	return answer;
}

bool NNTPConnection::DoConnect()
{
	debug("Opening connection to %s", GetServer()->GetHost());
	bool res = Connection::DoConnect();
	if (!res)
	{	
                // gauss 110720{
		error_time ++;
                //info("Connect error_time is %d",error_time);
                if(error_time > ERROR_TIME){
			ReportErrorAnswer("Connecting to %s failed ", "Connecting failed");
                        //force_stop_nzbget = 1;
		}
                // gauss 110720}
		return res;
	}

        //error_time = 0;

#ifndef DISABLE_TLS
	if (GetNewsServer()->GetTLS())
	{
		if (!StartTLS())
		{	
                        // gauss 110720{
			error_time ++;
                        //info("SSL error_time is %d",error_time);
                        if(error_time > ERROR_TIME){
				ReportErrorAnswer("Connecting to %s with TLS/SSL failed", "TLS/SSL failed");
                                //force_stop_nzbget = 1;
			}
                // gauss 110720}
		
			return false;
		}
	}
#endif
        //error_time = 0;

	char* answer = DoReadLine(m_szLineBuf, CONNECTION_LINEBUFFER_SIZE, NULL);

	if (!answer)
	{
                error_time++;
                //info("Can't read error_time is %d",error_time);
                if(error_time > ERROR_TIME){
                     ReportErrorAnswer("Connection to %s failed: Connection closed by remote host.", "Recv Error");
                     //force_stop_nzbget = 1;	// gauss 110720
                }
                ReportError("Connection to %s failed: Connection closed by remote host.", m_pNetAddress->GetHost(), true, 0);

		return false;
	}

        //error_time = 0;

	if (strncmp(answer, "2", 1))
	{
                error_time++;
                //info("Read error _time is %d",error_time);
                if(error_time > ERROR_TIME){
                ReportErrorAnswer("Connection to %s failed (Answer: %s)", answer);
                //force_stop_nzbget = 1;	// gauss 110720
            }
		return false;
	}

	debug("Connection to %s established", GetServer()->GetHost());

	return true;
}

bool NNTPConnection::DoDisconnect()
{
	if (m_eStatus == csConnected)
	{
		Request("quit\r\n");
		if (m_szActiveGroup)
		{
			free(m_szActiveGroup);
			m_szActiveGroup = NULL;
		}
	}
	return Connection::DoDisconnect();
}

void NNTPConnection::ReportErrorAnswer(const char* szMsgPrefix, const char* szAnswer)
{
	char szErrStr[1024];
	snprintf(szErrStr, 1024, szMsgPrefix, m_pNetAddress->GetHost(), szAnswer);
	szErrStr[1024-1] = '\0';

        //info("enter reportErrorAnswer function");

        //rich ... we can add code here to tell giftd error info{

        //info("have_error_log is %d",have_error_log);

#if 1

        if( !have_error_log && !force_cancle_nzb  && !force_pause_nzb && !have_update_log)
        {
                       //info("############ write error log #############");
                        Log_struc log_s;
                        char logName[256];
                        int sv_fd = -1;

                        int log_id = 0 ;
                        GroupQueue groupQueue;
                        groupQueue.clear();
                        DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
                        pDownloadQueue->BuildGroups(&groupQueue);
                        g_pQueueCoordinator->UnlockQueue();

                        for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
                        {
                                GroupInfo* pGroupInfo = *it;
                                log_id = pGroupInfo->GetNZBInfo()->GetID();
                         }

                        sprintf(logName, "%snzb_%d", g_pOptions->GetLogsDir(), log_id);

                        /*
                        if(sema_open(psem_nntp, "/shares/dmathined/Download/.sems/sem.NNTP.nzb", O_RDONLY) == -1){
                                printf("open sema failed\n");
                                semUse = 0;
                        }
                        if(semUse)
                                sema_wait(psem_nntp);
                        */

                        //if((sv_fd = open(logName, O_RDONLY)) != -1)
                        //if( !force_cancle_nzb  && !force_pause_nzb )
                        {
                            close(sv_fd);

                            //info("############ write error log #############");
                            //info("answer is %s ",szAnswer);
                            //info("log file name is %s",logName);

                            if((sv_fd = open(logName, O_RDWR|O_CREAT)) < 0)
                                    //printf("nzb nntpcnctn Open log error\n");
                                    info("nzb nntpcnctn Open log error\n");

                            else{
                                    //log_s.status = S_HOLD;
                                    if(LOG_SIZE != read(sv_fd, &log_s, LOG_SIZE)){
                                            info("nzb nntpcnctn read log error\n");
                                    }
                                }

                            time(&log_s.now_t);
                            //sprintf(log_s.fullname, "%s", szErrStr);
                            snprintf(log_s.fullname, MAX_NAMELEN, "%s", szAnswer);
                            //sprintf(log_s.store_dst, "/tmp/harddisk/part0/share/Download/dst");
                            //log_s.filesize = pFileInfo->m_lSize;
                            //log_s.download_type = NNTP;


                            if( strcmp(szAnswer,"Connecting failed") == 0 || strncmp(szAnswer, "2", 1) == 0)
                            {
                                log_s.status = S_CONNECTFAIL;
                            }
                            else if(strcmp(szAnswer,"TLS/SSL failed") == 0)
                            {
                                log_s.status = S_SSLFAIL;
                            }
                            else if( strcmp(szAnswer,"Account failed") == 0)
                            {
                                log_s.status = S_ACCOUNTFAIL;
                            }
                            else
                                log_s.status = S_RECVFAIL;



                            //log_s.status = S_DEAD_OF_ERROR;
                            log_s.download_type = NZB;
                            log_s.ifile.inzb.rate = 0.0;
                            log_s.error = 1;
                            lseek(sv_fd, 0, SEEK_SET);

                            if(LOG_SIZE != write(sv_fd, &log_s, LOG_SIZE)){
                                    info("nzb nntpcnctn write log error\n");

                            }
                            close(sv_fd);

                            /*
                            if(semUse){
                                    sema_post(psem_nntp);
                                    sema_close(psem_nntp);
                            }
                            */
                            // rich add ends }
                             have_error_log = 1;

                        }


        }
#endif


	
	ReportError(szErrStr, NULL, false, 0);
}
