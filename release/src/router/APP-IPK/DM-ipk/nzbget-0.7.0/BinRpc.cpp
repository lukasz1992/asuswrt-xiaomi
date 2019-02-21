/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2005 Bo Cordes Petersen <placebodk@sourceforge.net>
 *  Copyright (C) 2007-2010 Andrei Prygounkov <hugbug@users.sourceforge.net>
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
 * $Revision: 1.11 $
 * $Date: 2012/01/03 07:16:49 $
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include "win32.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <fstream>
#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#endif

#include "nzbget.h"
#include "BinRpc.h"
#include "Log.h"
#include "Options.h"
#include "QueueCoordinator.h"
#include "QueueEditor.h"
#include "PrePostProcessor.h"
#include "Util.h"
#include "logs.h"
#include "ex.h"


//#define SEMA 1

extern Options* g_pOptions;
extern QueueCoordinator* g_pQueueCoordinator;
extern PrePostProcessor* g_pPrePostProcessor;
extern void ExitProc();

extern int force_pause_nzb;
extern int have_error_log;
extern int force_cancle_nzb;
extern int error_time;
extern int have_update_log;
extern int not_complete;
int disk_full_paused;
extern int auto_paused;
extern int above_4GB;
extern int iInit4GBErrorCount;
extern char szRecord4GBFile[256];
extern bool bDelInit;
//extern bool bDelQueueEnd;
extern bool bParseSeedEnd;

const char* g_szMessageRequestNames[] =
    { "N/A", "Download", "Pause/Unpause", "List", "Set download rate", "Dump debug", 
		"Edit queue", "Log", "Quit", "Version", "Post-queue", "Write log", "Scan", 
		"Pause/Unpause postprocessor", "History" };

const unsigned int g_iMessageRequestSizes[] =
    { 0,
		sizeof(SNZBDownloadRequest),
		sizeof(SNZBPauseUnpauseRequest),
		sizeof(SNZBListRequest),
		sizeof(SNZBSetDownloadRateRequest),
		sizeof(SNZBDumpDebugRequest),
		sizeof(SNZBEditQueueRequest),
		sizeof(SNZBLogRequest),
		sizeof(SNZBShutdownRequest),
		sizeof(SNZBVersionRequest),
		sizeof(SNZBPostQueueRequest),
		sizeof(SNZBWriteLogRequest),
		sizeof(SNZBScanRequest),
		sizeof(SNZBHistoryRequest)
    };

//*****************************************************************
// BinProcessor

void BinRpcProcessor::Execute()
{
	// Read the first package which needs to be a request
	int iBytesReceived = recv(m_iSocket, ((char*)&m_MessageBase) + sizeof(m_MessageBase.m_iSignature), sizeof(m_MessageBase) - sizeof(m_MessageBase.m_iSignature), 0);
	if (iBytesReceived < 0)
	{
		return;
	}

	// Make sure this is a nzbget request from a client
	if ((int)ntohl(m_MessageBase.m_iSignature) != (int)NZBMESSAGE_SIGNATURE)
	{
		warn("Non-nzbget request received on port %i from %s", g_pOptions->GetServerPort(), m_szClientIP);
		return;
	}

	if (strcmp(m_MessageBase.m_szPassword, g_pOptions->GetServerPassword()))
	{
		warn("nzbget request received on port %i from %s, but password invalid", g_pOptions->GetServerPort(), m_szClientIP);
		return;
	}

	debug("%s request received from %s", g_szMessageRequestNames[ntohl(m_MessageBase.m_iType)], m_szClientIP);

	Dispatch();
}

void BinRpcProcessor::Dispatch()
{
	if (ntohl(m_MessageBase.m_iType) >= (int)eRemoteRequestDownload &&
		   ntohl(m_MessageBase.m_iType) <= (int)eRemoteRequestHistory &&
		   g_iMessageRequestSizes[ntohl(m_MessageBase.m_iType)] != ntohl(m_MessageBase.m_iStructSize))
	{
		error("Invalid size of request: expected %i Bytes, but received %i Bytes",
			 g_iMessageRequestSizes[ntohl(m_MessageBase.m_iType)], ntohl(m_MessageBase.m_iStructSize));
		return;
	}
	
	BinCommand* command = NULL;

	switch (ntohl(m_MessageBase.m_iType))
	{
		case eRemoteRequestDownload:
			command = new DownloadBinCommand();
			break;

		case eRemoteRequestList:
			command = new ListBinCommand();
			break;

		case eRemoteRequestLog:
			command = new LogBinCommand();
			break;

		case eRemoteRequestPauseUnpause:
			command = new PauseUnpauseBinCommand();
			break;

		case eRemoteRequestEditQueue:
			command = new EditQueueBinCommand();
			break;

		case eRemoteRequestSetDownloadRate:
			command = new SetDownloadRateBinCommand();
			break;

		case eRemoteRequestDumpDebug:
			command = new DumpDebugBinCommand();
			break;

		case eRemoteRequestShutdown:
			command = new ShutdownBinCommand();
			break;

		case eRemoteRequestVersion:
			command = new VersionBinCommand();
			break;

		case eRemoteRequestPostQueue:
			command = new PostQueueBinCommand();
			break;

		case eRemoteRequestWriteLog:
			command = new WriteLogBinCommand();
			break;

		case eRemoteRequestScan:
			command = new ScanBinCommand();
			break;

		case eRemoteRequestHistory:
			command = new HistoryBinCommand();
			break;

		default:
			error("Received unsupported request %i", ntohl(m_MessageBase.m_iType));
			break;
	}

	if (command)
	{
		command->SetSocket(m_iSocket);
		command->SetMessageBase(&m_MessageBase);
		command->Execute();
		delete command;
	}
}

//*****************************************************************
// Commands

void BinCommand::SendBoolResponse(bool bSuccess, const char* szText)
{
	// all bool-responses have the same format of structure, we use SNZBDownloadResponse here
	SNZBDownloadResponse BoolResponse;
	memset(&BoolResponse, 0, sizeof(BoolResponse));
	BoolResponse.m_MessageBase.m_iSignature = htonl(NZBMESSAGE_SIGNATURE);
	BoolResponse.m_MessageBase.m_iStructSize = htonl(sizeof(BoolResponse));
	BoolResponse.m_bSuccess = htonl(bSuccess);
	int iTextLen = strlen(szText) + 1;
	BoolResponse.m_iTrailingDataLength = htonl(iTextLen);

	// Send the request answer
	send(m_iSocket, (char*) &BoolResponse, sizeof(BoolResponse), 0);
	send(m_iSocket, (char*)szText, iTextLen, 0);
}

bool BinCommand::ReceiveRequest(void* pBuffer, int iSize)
{
	memcpy(pBuffer, m_pMessageBase, sizeof(SNZBRequestBase));
	iSize -= sizeof(SNZBRequestBase);
	if (iSize > 0)
	{
		int iBytesReceived = recv(m_iSocket, ((char*)pBuffer) + sizeof(SNZBRequestBase), iSize, 0);
		if (iBytesReceived != iSize)
		{
			error("invalid request");
			return false;
		}
	}
	return true;
}

void PauseUnpauseBinCommand::Execute()
{
	SNZBPauseUnpauseRequest PauseUnpauseRequest;
	if (!ReceiveRequest(&PauseUnpauseRequest, sizeof(PauseUnpauseRequest)))
	{
		return;
	}

	switch (ntohl(PauseUnpauseRequest.m_iAction))
	{
		case eRemotePauseUnpauseActionDownload:
			g_pOptions->SetPauseDownload(ntohl(PauseUnpauseRequest.m_bPause));
			break;

		case eRemotePauseUnpauseActionDownload2:
			g_pOptions->SetPauseDownload2(ntohl(PauseUnpauseRequest.m_bPause));
			break;

		case eRemotePauseUnpauseActionPostProcess:
			g_pOptions->SetPausePostProcess(ntohl(PauseUnpauseRequest.m_bPause));
			break;

		case eRemotePauseUnpauseActionScan:
			g_pOptions->SetPauseScan(ntohl(PauseUnpauseRequest.m_bPause));
			break;
	}

	SendBoolResponse(true, "Pause-/Unpause-Command completed successfully");
}

void SetDownloadRateBinCommand::Execute()
{
	SNZBSetDownloadRateRequest SetDownloadRequest;
	if (!ReceiveRequest(&SetDownloadRequest, sizeof(SetDownloadRequest)))
	{
		return;
	}

	g_pOptions->SetDownloadRate(ntohl(SetDownloadRequest.m_iDownloadRate) / 1024.0f);
	SendBoolResponse(true, "Rate-Command completed successfully");
}

void DumpDebugBinCommand::Execute()
{
	SNZBDumpDebugRequest DumpDebugRequest;
	if (!ReceiveRequest(&DumpDebugRequest, sizeof(DumpDebugRequest)))
	{
		return;
	}

	g_pQueueCoordinator->LogDebugInfo();
	SendBoolResponse(true, "Debug-Command completed successfully");
}

void ShutdownBinCommand::Execute()
{
	SNZBShutdownRequest ShutdownRequest;
	if (!ReceiveRequest(&ShutdownRequest, sizeof(ShutdownRequest)))
	{
		return;
	}

	SendBoolResponse(true, "Stopping server");
	ExitProc();
}

void VersionBinCommand::Execute()
{
	SNZBVersionRequest VersionRequest;
	if (!ReceiveRequest(&VersionRequest, sizeof(VersionRequest)))
	{
		return;
	}

	SendBoolResponse(true, Util::VersionRevision());
}

void DownloadBinCommand::Execute()
{
	SNZBDownloadRequest DownloadRequest;
	if (!ReceiveRequest(&DownloadRequest, sizeof(DownloadRequest)))
	{
		return;
	}

	char* pRecvBuffer = (char*)malloc(ntohl(DownloadRequest.m_iTrailingDataLength) + 1);
	char* pBufPtr = pRecvBuffer;

	// Read from the socket until nothing remains
	int iResult = 0;
	int NeedBytes = ntohl(DownloadRequest.m_iTrailingDataLength);
	while (NeedBytes > 0)
	{
		iResult = recv(m_iSocket, pBufPtr, NeedBytes, 0);
		// Did the recv succeed?
		if (iResult <= 0)
		{
			error("invalid request");
			break;
		}
		pBufPtr += iResult;
		NeedBytes -= iResult;
	}

	if (NeedBytes == 0)
	{
		NZBFile* pNZBFile = NZBFile::CreateFromBuffer(DownloadRequest.m_szFilename, DownloadRequest.m_szCategory, pRecvBuffer, ntohl(DownloadRequest.m_iTrailingDataLength));

		if (pNZBFile)
		{
			info("Request: Queue collection %s", DownloadRequest.m_szFilename);
			g_pQueueCoordinator->AddNZBFileToQueue(pNZBFile, ntohl(DownloadRequest.m_bAddFirst));
			delete pNZBFile;

			char tmp[1024];
			snprintf(tmp, 1024, "Collection %s added to queue", Util::BaseFileName(DownloadRequest.m_szFilename));
			tmp[1024-1] = '\0';
			SendBoolResponse(true, tmp);
		}
		else
		{
			char tmp[1024];
			snprintf(tmp, 1024, "Download Request failed for %s", Util::BaseFileName(DownloadRequest.m_szFilename));
			tmp[1024-1] = '\0';
			SendBoolResponse(false, tmp);
		}
	}

	free(pRecvBuffer);
}

void ListBinCommand::Execute()
{
	SNZBListRequest ListRequest;
	if (!ReceiveRequest(&ListRequest, sizeof(ListRequest)))
	{
		return;
	}

	SNZBListResponse ListResponse;
	memset(&ListResponse, 0, sizeof(ListResponse));
	ListResponse.m_MessageBase.m_iSignature = htonl(NZBMESSAGE_SIGNATURE);
	ListResponse.m_MessageBase.m_iStructSize = htonl(sizeof(ListResponse));
	ListResponse.m_iEntrySize = htonl(sizeof(SNZBListResponseFileEntry));

	char* buf = NULL;
	int bufsize = 0;

	if (ntohl(ListRequest.m_bFileList))
	{
		// Make a data structure and copy all the elements of the list into it
		DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();

		// calculate required buffer size for nzbs
		int iNrNZBEntries = pDownloadQueue->GetNZBInfoList()->size();
		int iNrPPPEntries = 0;
		bufsize += iNrNZBEntries * sizeof(SNZBListResponseNZBEntry);
		for (NZBInfoList::iterator it = pDownloadQueue->GetNZBInfoList()->begin(); it != pDownloadQueue->GetNZBInfoList()->end(); it++)
		{
			NZBInfo* pNZBInfo = *it;
			bufsize += strlen(pNZBInfo->GetFilename()) + 1;
			bufsize += strlen(pNZBInfo->GetDestDir()) + 1;
			bufsize += strlen(pNZBInfo->GetCategory()) + 1;
			bufsize += strlen(pNZBInfo->GetQueuedFilename()) + 1;
			// align struct to 4-bytes, needed by ARM-processor (and may be others)
			bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;

			// calculate required buffer size for pp-parameters
			for (NZBParameterList::iterator it = pNZBInfo->GetParameters()->begin(); it != pNZBInfo->GetParameters()->end(); it++)
			{
				NZBParameter* pNZBParameter = *it;
				bufsize += sizeof(SNZBListResponsePPPEntry);
				bufsize += strlen(pNZBParameter->GetName()) + 1;
				bufsize += strlen(pNZBParameter->GetValue()) + 1;
				// align struct to 4-bytes, needed by ARM-processor (and may be others)
				bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;
				iNrPPPEntries++;
			}
		}

		// calculate required buffer size for files
		int iNrFileEntries = pDownloadQueue->GetFileQueue()->size();
		bufsize += iNrFileEntries * sizeof(SNZBListResponseFileEntry);
		for (FileQueue::iterator it = pDownloadQueue->GetFileQueue()->begin(); it != pDownloadQueue->GetFileQueue()->end(); it++)
		{
			FileInfo* pFileInfo = *it;
			bufsize += strlen(pFileInfo->GetSubject()) + 1;
			bufsize += strlen(pFileInfo->GetFilename()) + 1;
			// align struct to 4-bytes, needed by ARM-processor (and may be others)
			bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;
		}

		buf = (char*) malloc(bufsize);
		char* bufptr = buf;

		// write nzb entries
		for (NZBInfoList::iterator it = pDownloadQueue->GetNZBInfoList()->begin(); it != pDownloadQueue->GetNZBInfoList()->end(); it++)
		{
			unsigned long iSizeHi, iSizeLo;
			NZBInfo* pNZBInfo = *it;
			SNZBListResponseNZBEntry* pListAnswer = (SNZBListResponseNZBEntry*) bufptr;
			Util::SplitInt64(pNZBInfo->GetSize(), &iSizeHi, &iSizeLo);
			pListAnswer->m_iSizeLo				= htonl(iSizeLo);
			pListAnswer->m_iSizeHi				= htonl(iSizeHi);
			pListAnswer->m_iFilenameLen			= htonl(strlen(pNZBInfo->GetFilename()) + 1);
			pListAnswer->m_iDestDirLen			= htonl(strlen(pNZBInfo->GetDestDir()) + 1);
			pListAnswer->m_iCategoryLen			= htonl(strlen(pNZBInfo->GetCategory()) + 1);
			pListAnswer->m_iQueuedFilenameLen	= htonl(strlen(pNZBInfo->GetQueuedFilename()) + 1);
			bufptr += sizeof(SNZBListResponseNZBEntry);
			strcpy(bufptr, pNZBInfo->GetFilename());
			bufptr += ntohl(pListAnswer->m_iFilenameLen);
			strcpy(bufptr, pNZBInfo->GetDestDir());
			bufptr += ntohl(pListAnswer->m_iDestDirLen);
			strcpy(bufptr, pNZBInfo->GetCategory());
			bufptr += ntohl(pListAnswer->m_iCategoryLen);
			strcpy(bufptr, pNZBInfo->GetQueuedFilename());
			bufptr += ntohl(pListAnswer->m_iQueuedFilenameLen);
			// align struct to 4-bytes, needed by ARM-processor (and may be others)
			if ((size_t)bufptr % 4 > 0)
			{
				pListAnswer->m_iQueuedFilenameLen = htonl(ntohl(pListAnswer->m_iQueuedFilenameLen) + 4 - (size_t)bufptr % 4);
				memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
				bufptr += 4 - (size_t)bufptr % 4;
			}
		}

		// write ppp entries
		int iNZBIndex = 1;
		for (NZBInfoList::iterator it = pDownloadQueue->GetNZBInfoList()->begin(); it != pDownloadQueue->GetNZBInfoList()->end(); it++, iNZBIndex++)
		{
			NZBInfo* pNZBInfo = *it;
			for (NZBParameterList::iterator it = pNZBInfo->GetParameters()->begin(); it != pNZBInfo->GetParameters()->end(); it++)
			{
				NZBParameter* pNZBParameter = *it;
				SNZBListResponsePPPEntry* pListAnswer = (SNZBListResponsePPPEntry*) bufptr;
				pListAnswer->m_iNZBIndex	= htonl(iNZBIndex);
				pListAnswer->m_iNameLen		= htonl(strlen(pNZBParameter->GetName()) + 1);
				pListAnswer->m_iValueLen	= htonl(strlen(pNZBParameter->GetValue()) + 1);
				bufptr += sizeof(SNZBListResponsePPPEntry);
				strcpy(bufptr, pNZBParameter->GetName());
				bufptr += ntohl(pListAnswer->m_iNameLen);
				strcpy(bufptr, pNZBParameter->GetValue());
				bufptr += ntohl(pListAnswer->m_iValueLen);
				// align struct to 4-bytes, needed by ARM-processor (and may be others)
				if ((size_t)bufptr % 4 > 0)
				{
					pListAnswer->m_iValueLen = htonl(ntohl(pListAnswer->m_iValueLen) + 4 - (size_t)bufptr % 4);
					memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
					bufptr += 4 - (size_t)bufptr % 4;
				}
			}
		}

		// write file entries
		for (FileQueue::iterator it = pDownloadQueue->GetFileQueue()->begin(); it != pDownloadQueue->GetFileQueue()->end(); it++)
		{
			unsigned long iSizeHi, iSizeLo;
			FileInfo* pFileInfo = *it;
			SNZBListResponseFileEntry* pListAnswer = (SNZBListResponseFileEntry*) bufptr;
			pListAnswer->m_iID				= htonl(pFileInfo->GetID());

			int iNZBIndex = 0;
			for (unsigned int i = 0; i < pDownloadQueue->GetNZBInfoList()->size(); i++)
			{
				iNZBIndex++;
				if (pDownloadQueue->GetNZBInfoList()->at(i) == pFileInfo->GetNZBInfo())
				{
					break;
				}
			}
			pListAnswer->m_iNZBIndex		= htonl(iNZBIndex);

			Util::SplitInt64(pFileInfo->GetSize(), &iSizeHi, &iSizeLo);
			pListAnswer->m_iFileSizeLo		= htonl(iSizeLo);
			pListAnswer->m_iFileSizeHi		= htonl(iSizeHi);
			Util::SplitInt64(pFileInfo->GetRemainingSize(), &iSizeHi, &iSizeLo);
			pListAnswer->m_iRemainingSizeLo	= htonl(iSizeLo);
			pListAnswer->m_iRemainingSizeHi	= htonl(iSizeHi);
			pListAnswer->m_bFilenameConfirmed = htonl(pFileInfo->GetFilenameConfirmed());
			pListAnswer->m_bPaused			= htonl(pFileInfo->GetPaused());
			pListAnswer->m_iSubjectLen		= htonl(strlen(pFileInfo->GetSubject()) + 1);
			pListAnswer->m_iFilenameLen		= htonl(strlen(pFileInfo->GetFilename()) + 1);
			bufptr += sizeof(SNZBListResponseFileEntry);
			strcpy(bufptr, pFileInfo->GetSubject());
			bufptr += ntohl(pListAnswer->m_iSubjectLen);
			strcpy(bufptr, pFileInfo->GetFilename());
			bufptr += ntohl(pListAnswer->m_iFilenameLen);
			// align struct to 4-bytes, needed by ARM-processor (and may be others)
			if ((size_t)bufptr % 4 > 0)
			{
				pListAnswer->m_iFilenameLen = htonl(ntohl(pListAnswer->m_iFilenameLen) + 4 - (size_t)bufptr % 4);
				memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
				bufptr += 4 - (size_t)bufptr % 4;
			}
		}

		g_pQueueCoordinator->UnlockQueue();

		ListResponse.m_iNrTrailingNZBEntries = htonl(iNrNZBEntries);
		ListResponse.m_iNrTrailingPPPEntries = htonl(iNrPPPEntries);
		ListResponse.m_iNrTrailingFileEntries = htonl(iNrFileEntries);
		ListResponse.m_iTrailingDataLength = htonl(bufsize);
	}

	if (htonl(ListRequest.m_bServerState))
	{
		unsigned long iSizeHi, iSizeLo;
		ListResponse.m_iDownloadRate = htonl((int)(g_pQueueCoordinator->CalcCurrentDownloadSpeed() * 1024));
		Util::SplitInt64(g_pQueueCoordinator->CalcRemainingSize(), &iSizeHi, &iSizeLo);
		ListResponse.m_iRemainingSizeHi = htonl(iSizeHi);
		ListResponse.m_iRemainingSizeLo = htonl(iSizeLo);
		ListResponse.m_iDownloadLimit = htonl((int)(g_pOptions->GetDownloadRate() * 1024));
		ListResponse.m_bDownloadPaused = htonl(g_pOptions->GetPauseDownload());
		ListResponse.m_bDownload2Paused = htonl(g_pOptions->GetPauseDownload2());
		ListResponse.m_bPostPaused = htonl(g_pOptions->GetPausePostProcess());
		ListResponse.m_bScanPaused = htonl(g_pOptions->GetPauseScan());
		ListResponse.m_iThreadCount = htonl(Thread::GetThreadCount() - 1); // not counting itself
		PostQueue* pPostQueue = g_pQueueCoordinator->LockQueue()->GetPostQueue();
		ListResponse.m_iPostJobCount = htonl(pPostQueue->size());
		g_pQueueCoordinator->UnlockQueue();

		int iUpTimeSec, iDnTimeSec;
		long long iAllBytes;
		bool bStandBy;
		g_pQueueCoordinator->CalcStat(&iUpTimeSec, &iDnTimeSec, &iAllBytes, &bStandBy);
		ListResponse.m_iUpTimeSec = htonl(iUpTimeSec);
		ListResponse.m_iDownloadTimeSec = htonl(iDnTimeSec);
		ListResponse.m_bDownloadStandBy = htonl(bStandBy);
		Util::SplitInt64(iAllBytes, &iSizeHi, &iSizeLo);
		ListResponse.m_iDownloadedBytesHi = htonl(iSizeHi);
		ListResponse.m_iDownloadedBytesLo = htonl(iSizeLo);
	}

	// Send the request answer
	send(m_iSocket, (char*) &ListResponse, sizeof(ListResponse), 0);

	// Send the data
	if (bufsize > 0)
	{
		send(m_iSocket, buf, bufsize, 0);
	}

	if (buf)
	{
		free(buf);
	}
}

void LogBinCommand::Execute()
{
	SNZBLogRequest LogRequest;
	if (!ReceiveRequest(&LogRequest, sizeof(LogRequest)))
	{
		return;
	}

	Log::Messages* pMessages = g_pLog->LockMessages();

	int iNrEntries = ntohl(LogRequest.m_iLines);
	unsigned int iIDFrom = ntohl(LogRequest.m_iIDFrom);
	int iStart = pMessages->size();
	if (iNrEntries > 0)
	{
		if (iNrEntries > (int)pMessages->size())
		{
			iNrEntries = pMessages->size();
		}
		iStart = pMessages->size() - iNrEntries;
	}
	if (iIDFrom > 0 && !pMessages->empty())
	{
		iStart = iIDFrom - pMessages->front()->GetID();
		if (iStart < 0)
		{
			iStart = 0;
		}
		iNrEntries = pMessages->size() - iStart;
		if (iNrEntries < 0)
		{
			iNrEntries = 0;
		}
	}

	// calculate required buffer size
	int bufsize = iNrEntries * sizeof(SNZBLogResponseEntry);
	for (unsigned int i = (unsigned int)iStart; i < pMessages->size(); i++)
	{
		Message* pMessage = (*pMessages)[i];
		bufsize += strlen(pMessage->GetText()) + 1;
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;
	}

	char* buf = (char*) malloc(bufsize);
	char* bufptr = buf;
	for (unsigned int i = (unsigned int)iStart; i < pMessages->size(); i++)
	{
		Message* pMessage = (*pMessages)[i];
		SNZBLogResponseEntry* pLogAnswer = (SNZBLogResponseEntry*) bufptr;
		pLogAnswer->m_iID = htonl(pMessage->GetID());
		pLogAnswer->m_iKind = htonl(pMessage->GetKind());
		pLogAnswer->m_tTime = htonl((int)pMessage->GetTime());
		pLogAnswer->m_iTextLen = htonl(strlen(pMessage->GetText()) + 1);
		bufptr += sizeof(SNZBLogResponseEntry);
		strcpy(bufptr, pMessage->GetText());
		bufptr += ntohl(pLogAnswer->m_iTextLen);
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		if ((size_t)bufptr % 4 > 0)
		{
			pLogAnswer->m_iTextLen = htonl(ntohl(pLogAnswer->m_iTextLen) + 4 - (size_t)bufptr % 4);
			memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
			bufptr += 4 - (size_t)bufptr % 4;
		}
	}

	g_pLog->UnlockMessages();

	SNZBLogResponse LogResponse;
	LogResponse.m_MessageBase.m_iSignature = htonl(NZBMESSAGE_SIGNATURE);
	LogResponse.m_MessageBase.m_iStructSize = htonl(sizeof(LogResponse));
	LogResponse.m_iEntrySize = htonl(sizeof(SNZBLogResponseEntry));
	LogResponse.m_iNrTrailingEntries = htonl(iNrEntries);
	LogResponse.m_iTrailingDataLength = htonl(bufsize);

	// Send the request answer
	send(m_iSocket, (char*) &LogResponse, sizeof(LogResponse), 0);

	// Send the data
	if (bufsize > 0)
	{
		send(m_iSocket, buf, bufsize, 0);
	}

	free(buf);
}

void EditQueueBinCommand::Execute()
{
	SNZBEditQueueRequest EditQueueRequest;
	if (!ReceiveRequest(&EditQueueRequest, sizeof(EditQueueRequest)))
	{
		return;
	}

	int iNrEntries = ntohl(EditQueueRequest.m_iNrTrailingEntries);
	int iAction = ntohl(EditQueueRequest.m_iAction);
	int iOffset = ntohl(EditQueueRequest.m_iOffset);
	int iTextLen = ntohl(EditQueueRequest.m_iTextLen);
	bool bSmartOrder = ntohl(EditQueueRequest.m_bSmartOrder);
	unsigned int iBufLength = ntohl(EditQueueRequest.m_iTrailingDataLength);

	if (iNrEntries * sizeof(int32_t) + iTextLen != iBufLength)
	{
		error("Invalid struct size");
		return;
	}

	if (iNrEntries <= 0)
	{
		SendBoolResponse(false, "Edit-Command failed: no IDs specified");
		return;
	}

	char* pBuf = (char*)malloc(iBufLength);
	char* szText = NULL;
	int32_t* pIDs = NULL;

	// Read from the socket until nothing remains
	char* pBufPtr = pBuf;
	int NeedBytes = iBufLength;
	int iResult = 0;
	while (NeedBytes > 0)
	{
		iResult = recv(m_iSocket, pBufPtr, NeedBytes, 0);
		// Did the recv succeed?
		if (iResult <= 0)
		{
			error("invalid request");
			break;
		}
		pBufPtr += iResult;
		NeedBytes -= iResult;
	}
	bool bOK = NeedBytes == 0;

	if (bOK)
	{
		szText = iTextLen > 0 ? pBuf : NULL;
		pIDs = (int32_t*)(pBuf + iTextLen);
	}

	IDList cIDList;
	cIDList.reserve(iNrEntries);
	for (int i = 0; i < iNrEntries; i++)
	{
		cIDList.push_back(ntohl(pIDs[i]));
	}

	if (iAction < eRemoteEditActionPostMoveOffset)
	{
		bOK = g_pQueueCoordinator->GetQueueEditor()->EditList(&cIDList, bSmartOrder, (QueueEditor::EEditAction)iAction, iOffset, szText);
	}
	else
	{
		bOK = g_pPrePostProcessor->QueueEditList(&cIDList, (PrePostProcessor::EEditAction)iAction, iOffset);
	}

	free(pBuf);

	if (bOK)
	{
		SendBoolResponse(true, "Edit-Command completed successfully");
	}
	else
	{
		SendBoolResponse(false, "Edit-Command failed");
	}
}

void PostQueueBinCommand::Execute()
{
	SNZBPostQueueRequest PostQueueRequest;
	if (!ReceiveRequest(&PostQueueRequest, sizeof(PostQueueRequest)))
	{
		return;
	}

	SNZBPostQueueResponse PostQueueResponse;
	memset(&PostQueueResponse, 0, sizeof(PostQueueResponse));
	PostQueueResponse.m_MessageBase.m_iSignature = htonl(NZBMESSAGE_SIGNATURE);
	PostQueueResponse.m_MessageBase.m_iStructSize = htonl(sizeof(PostQueueResponse));
	PostQueueResponse.m_iEntrySize = htonl(sizeof(SNZBPostQueueResponseEntry));

	char* buf = NULL;
	int bufsize = 0;

	// Make a data structure and copy all the elements of the list into it
	PostQueue* pPostQueue = g_pQueueCoordinator->LockQueue()->GetPostQueue();

	int NrEntries = pPostQueue->size();

	// calculate required buffer size
	bufsize = NrEntries * sizeof(SNZBPostQueueResponseEntry);
	for (PostQueue::iterator it = pPostQueue->begin(); it != pPostQueue->end(); it++)
	{
		PostInfo* pPostInfo = *it;
		bufsize += strlen(pPostInfo->GetNZBInfo()->GetFilename()) + 1;
		bufsize += strlen(pPostInfo->GetParFilename()) + 1;
		bufsize += strlen(pPostInfo->GetInfoName()) + 1;
		bufsize += strlen(pPostInfo->GetNZBInfo()->GetDestDir()) + 1;
		bufsize += strlen(pPostInfo->GetProgressLabel()) + 1;
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;
	}

	time_t tCurTime = time(NULL);
	buf = (char*) malloc(bufsize);
	char* bufptr = buf;

	for (PostQueue::iterator it = pPostQueue->begin(); it != pPostQueue->end(); it++)
	{
		PostInfo* pPostInfo = *it;
		SNZBPostQueueResponseEntry* pPostQueueAnswer = (SNZBPostQueueResponseEntry*) bufptr;
		pPostQueueAnswer->m_iID				= htonl(pPostInfo->GetID());
		pPostQueueAnswer->m_iStage			= htonl(pPostInfo->GetStage());
		pPostQueueAnswer->m_iStageProgress	= htonl(pPostInfo->GetStageProgress());
		pPostQueueAnswer->m_iFileProgress	= htonl(pPostInfo->GetFileProgress());
		pPostQueueAnswer->m_iTotalTimeSec	= htonl((int)(pPostInfo->GetStartTime() ? tCurTime - pPostInfo->GetStartTime() : 0));
		pPostQueueAnswer->m_iStageTimeSec	= htonl((int)(pPostInfo->GetStageTime() ? tCurTime - pPostInfo->GetStageTime() : 0));
		pPostQueueAnswer->m_iNZBFilenameLen		= htonl(strlen(pPostInfo->GetNZBInfo()->GetFilename()) + 1);
		pPostQueueAnswer->m_iParFilename		= htonl(strlen(pPostInfo->GetParFilename()) + 1);
		pPostQueueAnswer->m_iInfoNameLen		= htonl(strlen(pPostInfo->GetInfoName()) + 1);
		pPostQueueAnswer->m_iDestDirLen			= htonl(strlen(pPostInfo->GetNZBInfo()->GetDestDir()) + 1);
		pPostQueueAnswer->m_iProgressLabelLen	= htonl(strlen(pPostInfo->GetProgressLabel()) + 1);
		bufptr += sizeof(SNZBPostQueueResponseEntry);
		strcpy(bufptr, pPostInfo->GetNZBInfo()->GetFilename());
		bufptr += ntohl(pPostQueueAnswer->m_iNZBFilenameLen);
		strcpy(bufptr, pPostInfo->GetParFilename());
		bufptr += ntohl(pPostQueueAnswer->m_iParFilename);
		strcpy(bufptr, pPostInfo->GetInfoName());
		bufptr += ntohl(pPostQueueAnswer->m_iInfoNameLen);
		strcpy(bufptr, pPostInfo->GetNZBInfo()->GetDestDir());
		bufptr += ntohl(pPostQueueAnswer->m_iDestDirLen);
		strcpy(bufptr, pPostInfo->GetProgressLabel());
		bufptr += ntohl(pPostQueueAnswer->m_iProgressLabelLen);
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		if ((size_t)bufptr % 4 > 0)
		{
			pPostQueueAnswer->m_iProgressLabelLen = htonl(ntohl(pPostQueueAnswer->m_iProgressLabelLen) + 4 - (size_t)bufptr % 4);
			memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
			bufptr += 4 - (size_t)bufptr % 4;
		}
	}

	g_pQueueCoordinator->UnlockQueue();

	PostQueueResponse.m_iNrTrailingEntries = htonl(NrEntries);
	PostQueueResponse.m_iTrailingDataLength = htonl(bufsize);

	// Send the request answer
	send(m_iSocket, (char*) &PostQueueResponse, sizeof(PostQueueResponse), 0);

	// Send the data
	if (bufsize > 0)
	{
		send(m_iSocket, buf, bufsize, 0);
	}

	free(buf);
}

void WriteLogBinCommand::Execute()
{
	SNZBWriteLogRequest WriteLogRequest;
	if (!ReceiveRequest(&WriteLogRequest, sizeof(WriteLogRequest)))
	{
		return;
	}

	char* pRecvBuffer = (char*)malloc(ntohl(WriteLogRequest.m_iTrailingDataLength) + 1);
	char* pBufPtr = pRecvBuffer;

	// Read from the socket until nothing remains
	int iResult = 0;
	int NeedBytes = ntohl(WriteLogRequest.m_iTrailingDataLength);
	pRecvBuffer[NeedBytes] = '\0';
	while (NeedBytes > 0)
	{
		iResult = recv(m_iSocket, pBufPtr, NeedBytes, 0);
		// Did the recv succeed?
		if (iResult <= 0)
		{
			error("invalid request");
			break;
		}
		pBufPtr += iResult;
		NeedBytes -= iResult;
	}

	if (NeedBytes == 0)
	{
		bool OK = true;
		switch ((Message::EKind)ntohl(WriteLogRequest.m_iKind))
		{
			case Message::mkDetail:
				detail(pRecvBuffer);
				break;
			case Message::mkInfo:
				info(pRecvBuffer);
				break;
			case Message::mkWarning:
				warn(pRecvBuffer);
				break;
			case Message::mkError:
				error(pRecvBuffer);
				break;
			case Message::mkDebug:
				debug(pRecvBuffer);
				break;
			default:
				OK = false;
		}
		SendBoolResponse(OK, OK ? "Message added to log" : "Invalid message-kind");
	}

	free(pRecvBuffer);
}

void ScanBinCommand::Execute()
{
	SNZBScanRequest ScanRequest;
	if (!ReceiveRequest(&ScanRequest, sizeof(ScanRequest)))
	{
		return;
	}

	g_pPrePostProcessor->ScanNZBDir();
	SendBoolResponse(true, "Scan-Command scheduled successfully");
}

void HistoryBinCommand::Execute()
{
	SNZBHistoryRequest HistoryRequest;
	if (!ReceiveRequest(&HistoryRequest, sizeof(HistoryRequest)))
	{
		return;
	}

	SNZBHistoryResponse HistoryResponse;
	memset(&HistoryResponse, 0, sizeof(HistoryResponse));
	HistoryResponse.m_MessageBase.m_iSignature = htonl(NZBMESSAGE_SIGNATURE);
	HistoryResponse.m_MessageBase.m_iStructSize = htonl(sizeof(HistoryResponse));
	HistoryResponse.m_iEntrySize = htonl(sizeof(SNZBHistoryResponseEntry));

	char* buf = NULL;
	int bufsize = 0;

	// Make a data structure and copy all the elements of the list into it
	DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();

	// calculate required buffer size for nzbs
	int iNrNZBEntries = pDownloadQueue->GetHistoryList()->size();
	bufsize += iNrNZBEntries * sizeof(SNZBHistoryResponseEntry);
	for (HistoryList::iterator it = pDownloadQueue->GetHistoryList()->begin(); it != pDownloadQueue->GetHistoryList()->end(); it++)
	{
		NZBInfo* pNZBInfo = *it;
		bufsize += strlen(pNZBInfo->GetFilename()) + 1;
		bufsize += strlen(pNZBInfo->GetDestDir()) + 1;
		bufsize += strlen(pNZBInfo->GetCategory()) + 1;
		bufsize += strlen(pNZBInfo->GetQueuedFilename()) + 1;
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		bufsize += bufsize % 4 > 0 ? 4 - bufsize % 4 : 0;
	}

	buf = (char*) malloc(bufsize);
	char* bufptr = buf;

	// write nzb entries
	for (NZBInfoList::iterator it = pDownloadQueue->GetHistoryList()->begin(); it != pDownloadQueue->GetHistoryList()->end(); it++)
	{
		unsigned long iSizeHi, iSizeLo;
		NZBInfo* pNZBInfo = *it;
		SNZBHistoryResponseEntry* pListAnswer = (SNZBHistoryResponseEntry*) bufptr;
		Util::SplitInt64(pNZBInfo->GetSize(), &iSizeHi, &iSizeLo);
		pListAnswer->m_iID					= htonl(pNZBInfo->GetID());
		pListAnswer->m_tTime				= htonl((int)pNZBInfo->GetHistoryTime());
		pListAnswer->m_iSizeLo				= htonl(iSizeLo);
		pListAnswer->m_iSizeHi				= htonl(iSizeHi);
		pListAnswer->m_iFileCount			= htonl(pNZBInfo->GetFileCount());
		pListAnswer->m_iParStatus			= htonl(pNZBInfo->GetParStatus());
		pListAnswer->m_iScriptStatus		= htonl(pNZBInfo->GetScriptStatus());
		pListAnswer->m_iFilenameLen			= htonl(strlen(pNZBInfo->GetFilename()) + 1);
		pListAnswer->m_iDestDirLen			= htonl(strlen(pNZBInfo->GetDestDir()) + 1);
		pListAnswer->m_iCategoryLen			= htonl(strlen(pNZBInfo->GetCategory()) + 1);
		pListAnswer->m_iQueuedFilenameLen	= htonl(strlen(pNZBInfo->GetQueuedFilename()) + 1);
		bufptr += sizeof(SNZBHistoryResponseEntry);
		strcpy(bufptr, pNZBInfo->GetFilename());
		bufptr += ntohl(pListAnswer->m_iFilenameLen);
		strcpy(bufptr, pNZBInfo->GetDestDir());
		bufptr += ntohl(pListAnswer->m_iDestDirLen);
		strcpy(bufptr, pNZBInfo->GetCategory());
		bufptr += ntohl(pListAnswer->m_iCategoryLen);
		strcpy(bufptr, pNZBInfo->GetQueuedFilename());
		bufptr += ntohl(pListAnswer->m_iQueuedFilenameLen);
		// align struct to 4-bytes, needed by ARM-processor (and may be others)
		if ((size_t)bufptr % 4 > 0)
		{
			pListAnswer->m_iQueuedFilenameLen = htonl(ntohl(pListAnswer->m_iQueuedFilenameLen) + 4 - (size_t)bufptr % 4);
			memset(bufptr, 0, 4 - (size_t)bufptr % 4); //suppress valgrind warning "uninitialized data"
			bufptr += 4 - (size_t)bufptr % 4;
		}
	}

	g_pQueueCoordinator->UnlockQueue();

	HistoryResponse.m_iNrTrailingEntries = htonl(iNrNZBEntries);
	HistoryResponse.m_iTrailingDataLength = htonl(bufsize);

	// Send the request answer
	send(m_iSocket, (char*) &HistoryResponse, sizeof(HistoryResponse), 0);

	// Send the data
	if (bufsize > 0)
	{
		send(m_iSocket, buf, bufsize, 0);
	}

	free(buf);
}



// Walf add for socket commands
void SocketProcessor::Execute()
{
    char cmdbuf[256];
    memset(cmdbuf, 0, sizeof(cmdbuf));
    memcpy(cmdbuf, (char*)&m_iSignature, 4);

    int iBytesReceived = recv(m_iSocket, cmdbuf+4, sizeof(cmdbuf)-4, 0);
    if (iBytesReceived < 0)
    {
        closesocket(m_iSocket);

        return;

    }

    cmd_parser(cmdbuf);

}

int checkDiskSpace(Log_struc log_s)
{
    const float EPSINON = 0.00001;
    long long lFreeSpace = Util::FreeDiskSize(g_pOptions->GetDestDir());
    //info("free space is %lld ,progress is %f\n",lFreeSpace,log_s.progress);
    if( log_s.progress >= -EPSINON && log_s.progress <= EPSINON )
    {
        //info("no progress \n");
        //TEST
        long long reserve_size = (long long)log_s.filesize * (1-log_s.progress);
        //info("reserve size is %lld \n",reserve_size);
        if (lFreeSpace > -1 && lFreeSpace - RESERVE_SPACE < reserve_size )
        {
           info("lFreeSpace - RESERVE_SPACE is %lld \n",lFreeSpace - RESERVE_SPACE);
        }

        if (lFreeSpace > -1 && lFreeSpace - RESERVE_SPACE < log_s.filesize )
        {
            return 1;
        }
        else
        {
            //info("space is enough\n");
            g_pOptions->SetPauseDownload(false);
        }

    }
    else
    {
        //info("has progress \n");
        long long reserve_size = (long long)log_s.filesize * (1- log_s.progress);

        if (lFreeSpace > -1 && lFreeSpace - RESERVE_SPACE < reserve_size )
        {
            return 1;
        }
        else
        {
            //info("space is enough\n");
            g_pOptions->SetPauseDownload(false);
        }
    }

    return 0;
}

int WriteInit4GBErrorCount(char *szFilename,int iCount)
{
    if(iCount == 0)
        return 0;

    FILE *fp = fopen(szFilename,"w");
    if(fp == NULL)
    {
        info("open %s fail",szFilename);
        return -1;
    }

    fprintf(fp,"%d",iCount);
    fclose(fp);
    return 0;
}

int check_4GB(NZBFile* pNZBFile)
{
    above_4GB = 0;
    //2013.09.16 gauss add{
    for (std::vector<FileInfo*>::iterator it = pNZBFile->GetFileInfos()->begin(); it != pNZBFile->GetFileInfos()->end(); it++)
    {
        FileInfo *pinfo = *it;
        if(pinfo->GetSize() >= SIZE_4G)
        {
            FILE *fp;
            char disk_type[8] = {0};
            fp = fopen("/tmp/asus_router.conf","r");
            if(fp)
            {
                char generalbuf[128];
                while(!feof(fp))
                {
                    fscanf(fp,"%[^\n]%*c",generalbuf);
                    if(strncmp(generalbuf,"DEVICE_TYPE=",11) == 0)
                    {
                        strcpy(disk_type,generalbuf+12);
                        if(!strncmp(disk_type,"vfat",4)||!strncmp(disk_type,"tfat",4))
                        {
                          fclose(fp);
                          char logName[256],nzb_name[256];
                          int id = pNZBFile->GetNZBInfo()->GetID();
                          pNZBFile->GetNZBInfo()->GetNiceNZBName(nzb_name,sizeof(nzb_name));

                          sprintf(logName, "%snzb_%d", g_pOptions->GetLogsDir(), id);

                          fp = fopen(logName,"w");

                          if(fp == NULL)
                          {
                              info("open %s fail");
                              return -1;
                          }

                          Log_struc log_s;
                          log_s.status = S_TYPE_OF_ERROR;
                          log_s.download_type = NZB;
                          log_s.ifile.inzb.rate = 0.0;
                          log_s.error = 1;
                          log_s.filesize = pNZBFile->GetNZBInfo()->GetSize();
                          strcpy(log_s.filename,nzb_name);

                          fwrite(&log_s,LOG_SIZE,1,fp);
                          fclose(fp);

                          iInit4GBErrorCount++;
                          WriteInit4GBErrorCount(szRecord4GBFile,iInit4GBErrorCount);

                          return 1;
                        }
                        else
                            above_4GB = 1;
                        break;
                    }
                }
            }
            fclose(fp);
        }
    }
    return 0;
    //2013.09.16 gauss add{
}

int write_init_log(char *fullname)
{
    int id = 0,pre_id=0;
    Log_struc log_s;
    char log_name[256],filename[256];

    NZBInfo *pInfo = new NZBInfo();
    id = pInfo->GetID();
    pre_id = id - 1;
    pInfo->SetID(pre_id);
    delete pInfo;

    //info("******** id=%d ********",id);

    char *p = strrchr(fullname,'/');
    if(p)
    {
        strcpy(filename,p+1);
        //info("******** filename=%s ********",filename);
        sprintf(log_name,"%snzb_%d",g_pOptions->GetLogsDir(),id);
        FILE *fp = fopen(log_name, "w");
        if(fp)
        {
            sprintf(log_s.id, "%d", id);
            strcpy(log_s.filename,filename);
            log_s.download_type = NZB ;
            log_s.status = S_INITIAL;
            log_s.ifile.inzb.rate = 0 ;
            fwrite(&log_s,  LOG_SIZE, 1,fp);
            fclose(fp);
        }
    }
}

int DelInitQueueFiles()
{
    GroupQueue groupQueue;
    int i = 0;
    groupQueue.clear();
    DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
    pDownloadQueue->BuildGroups(&groupQueue);
    g_pQueueCoordinator->UnlockQueue();

    for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
           i++;

    if(i==0)
    {
        char path[1024];
        snprintf(path, 1024, "%s", g_pOptions->GetQueueDir());
        path[1024-1] = '\0';

        struct dirent* ent = NULL;
        DIR *pDir;
        pDir=opendir(path);
        if(pDir != NULL )
        {
            while (NULL != (ent=readdir(pDir)))
            {
                    if( ent->d_name[0] == '.' || !strcmp(ent->d_name,"queue"))
                        continue;

                    char t_src[1024];
                    memset(t_src, 0, sizeof(t_src));
                    sprintf(t_src,"%s%s",path,ent->d_name);
                    remove(t_src);

            }
            closedir(pDir);
        }

        return 1;
    }

    return 0;
}

// Walf add for socket commands
void SocketProcessor::cmd_parser(char* str_cmd)
{
    info("******** socket command is %s ********",str_cmd);
    char cmd_name[50];
    char cmd_param[512];
    char *ch;

    memset(cmd_name, 0, sizeof(cmd_name));
    memset(cmd_param, 0, sizeof(cmd_param));

    ch = str_cmd;
    int i = 0;
    while(*ch != '@')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, str_cmd, i);

    if(strcmp(cmd_name, "add") == 0)
    {
        bParseSeedEnd = false;
        ch++; // pass @ ch
        i++;
        if(*ch == '1') // torrent path
        {
            ch += 2;
            i += 2; // pass @ ch
            memset(cmd_param, 0, sizeof(cmd_param));
            strcpy(cmd_param, str_cmd+i);
            write_init_log(cmd_param);
            int len = Util::FileSize(cmd_param);
            char* pRecvBuffer = (char*)malloc(len + 1);
            int re = Util::LoadFileIntoBuffer(cmd_param, &pRecvBuffer, &len);
            NZBFile* pNZBFile = NZBFile::CreateFromBuffer(cmd_param, "", pRecvBuffer, len);

            if (pNZBFile)
            {
                if(check_4GB(pNZBFile) == 1)
                {
                   info("single file size above 4GB");
                   delete pNZBFile;
                   bParseSeedEnd = true;
                   return;
                }
                //add by gauss for sem{
                char log_name[256],nzb_name[256];
                int logid = pNZBFile->GetNZBInfo()->GetID();

                sprintf(log_name,"%snzb_%d",g_pOptions->GetLogsDir(),logid);
                //info("sem is %s",sem_name);
#ifdef SEMA
                char sem_name[1024];
                sprintf(sem_name,"%ssem.nzb_%d",g_pOptions->GetSemsDir(),logid);
                sema_init(sem_name);
#endif
                //add by gauss }

                //create init log
                Log_struc log_s;

                pNZBFile->GetNZBInfo()->GetNiceNZBName(nzb_name,sizeof(nzb_name));

                FILE *fp = fopen(log_name, "w");
                if(fp)
                {
                    sprintf(log_s.id, "%d", logid);
                    strcpy(log_s.filename,nzb_name);
                    log_s.filesize = pNZBFile->GetNZBInfo()->GetSize();
                    log_s.download_type = NZB ;

                    //if()

                    long long lFreeSpace = Util::FreeDiskSize(g_pOptions->GetDestDir());
                    if (lFreeSpace > -1 && lFreeSpace - RESERVE_SPACE < log_s.filesize )
                    {
                            //warn("Low disk space. Pausing download");
                            //g_pOptions->SetPauseDownload(true);
                            log_s.status = S_DISKFULL;
                    }
                    else
                    {
                        log_s.status = S_PROCESSING;
                    }


                    log_s.ifile.inzb.rate = 0 ;

                    fwrite(&log_s,  LOG_SIZE, 1,fp);
                    fclose(fp);

                }

                g_pQueueCoordinator->AddNZBFileToQueue(pNZBFile, 0);

                if(log_s.status == S_DISKFULL)
                {
                    g_pOptions->SetPauseDownload(true);
                    disk_full_paused = 1;
                }

                force_cancle_nzb = 0;
                error_time = 0;
                have_update_log = 0;

                delete pNZBFile;

                char tmp[1024];
                snprintf(tmp, 1024, "Collection %s added to queue", Util::BaseFileName(cmd_param));
                tmp[1024-1] = '\0';

                send(m_iSocket, (char*) &tmp, sizeof(tmp), 0);
            }
            else
            {
                char tmp[1024];
                snprintf(tmp, 1024, "Download Request failed for %s", Util::BaseFileName(cmd_param));
                tmp[1024-1] = '\0';
                send(m_iSocket, (char*) &tmp, sizeof(tmp), 0);
            }

            free(pRecvBuffer);
        }
        else if(*ch == '0') // url
        {
        }
        else if(*ch == '2') // torrent data
        {
        }
        bParseSeedEnd = true;
    }
    else if(strcmp(cmd_name, "pause") == 0 || strcmp(cmd_name, "start") == 0 || strcmp(cmd_name, "cancel") == 0 ||
            strcmp(cmd_name,"all_paused") == 0 || strcmp(cmd_name,"all_start") == 0 || strcmp(cmd_name, "clean_completed") == 0 )
    {
        ch++; // pass @ ch
        i++;
        memset(cmd_param, 0, sizeof(cmd_param));
        strcpy(cmd_param, str_cmd+i);


        int id = atoi(cmd_param);
        int task_id = 0;
        char path[256];
#ifdef SEMA
        int semUse = 1;
        char sem_name[1024];
#endif
        Log_struc log_s;
        FILE *fp;

        //info("******id is %d******",id);

        if( id > 0 ) //can obtain id from command
        {
            sprintf(path, "%snzb_%d", g_pOptions->GetLogsDir(), id); // use NZBInfo ID
            //add by gauss for sem
#ifdef SEMA
            sprintf(sem_name,"%ssem.nzb_%d",g_pOptions->GetSemsDir(),id);

            sema_t sema;
            if(sema_open(&sema, sem_name, O_RDONLY) == -1){
                    //printf("nzbget sem open error\n");
                    semUse = 0;
                    }


            if(semUse)
                sema_wait(&sema);
#endif

                fp = fopen(path, "r");

                if(fp)
                {
                    fread(&log_s, 1, LOG_SIZE, fp);
                    task_id = atoi(log_s.id);
                    fclose(fp);

                    if(log_s.status == S_INITIAL && !strcmp(cmd_name, "cancel"))
                    {
                        bDelInit = true;
                        //while(!bDelQueueEnd)
                        while(!bParseSeedEnd)
                           usleep(500);


                        bDelInit = false;

                        if(DelInitQueueFiles())
                        {
                            remove(path);
                            return;
                        }
                        //bDelQueueEnd = false;
                        //return;
                    }
                }
#ifdef SEMA
                if(semUse)
                {
                    sema_post(&sema);
                    sema_close(&sema);
                }
#endif
        }
        else    //obtain id from read log
        {
            char src_path[256],t_src[256];
            memset(src_path,'\0',sizeof(src_path));

            strcpy(src_path,g_pOptions->GetLogsDir());
            //info("******* src_path is %s ******",src_path);
            struct dirent* ent = NULL;
            DIR *pDir;
            pDir=opendir(src_path);
            if(pDir != NULL )
            {
                while (NULL != (ent=readdir(pDir)))
                {
                    char *p = NULL ;
                    p = strstr(ent->d_name,"nzb_");
                    if(p)
                    {
                        memset(t_src, 0, sizeof(t_src));
                        sprintf(t_src,"%s%s",src_path,ent->d_name);
                        //info("******* t_src is %s ******",t_src);

                        Log_struc log_ts;
                        memset(&log_ts, 0, sizeof(Log_struc));

                        FILE *fp = fopen(t_src, "r");
                        if(fp)
                        {
                            if( fread(&log_ts, 1, LOG_SIZE, fp) > 0 )
                            {
                                if(log_ts.status == S_INITIAL)
                                {
                                    fclose(fp);
                                    return;
                                }

                                if(strcmp(cmd_name,"all_paused") == 0)
                                {
                                    if(log_ts.status == S_PROCESSING)
                                    {
                                        task_id = atol(log_ts.id);
                                        log_ts.status = S_PAUSED;
                                        log_ts.ifile.inzb.rate = 0.0;

                                        fp = fopen(t_src, "w");
                                        if(fp)
                                        {
                                            fwrite(&log_ts, 1, LOG_SIZE, fp);
                                            fclose(fp);
                                            break;
                                        }

                                    }
                                }
                                else if(strcmp(cmd_name,"all_start") == 0)
                                {

                                    int diskfull;
                                    diskfull = checkDiskSpace(log_s);

                                    task_id = atol(log_ts.id);

                                    if(diskfull == 1)
                                    {
                                        if( log_ts.status == S_DISKFULL )
                                        {
                                            return ;
                                        }
                                        else if(log_ts.status == S_PAUSED )
                                        {
                                            log_ts.status = S_DISKFULL;

                                            fp = fopen(t_src, "w");

                                            if(fp)
                                            {
                                                fwrite(&log_ts, 1, LOG_SIZE, fp);
                                                fclose(fp);
                                            }

                                            return ;
                                        }


                                    }
                                    else
                                    {
                                        //info("******* change status to progress ******");
                                        //info("******* log_s status is %d ******",log_ts.status);
                                        if(log_ts.status == S_PAUSED || log_ts.status == S_DISKFULL)
                                        {

                                            log_ts.status = S_PROCESSING;
                                            auto_paused = 0 ;

                                            fp = fopen(t_src, "w");

                                            if(fp)
                                            {
                                                //if( fwrite(&log_ts, 1, LOG_SIZE, fp) > 0 )
                                                   fwrite(&log_ts, 1, LOG_SIZE, fp);
                                                    //info("******* wirte log ok ******");
                                                fclose(fp);
                                            }
                                        }
                                    }

                                }
                                else if(strcmp(cmd_name, "clean_completed") == 0)
                                {
                                    if(log_ts.status == S_COMPLETED)
                                        remove(t_src);
                                }
                            }
                                //info("******* wirte log ok ******");
                            fclose(fp);


                        }

                    }

                }
            }

            closedir(pDir);
        }

        int iNrEntries = 1;
        int iAction = 0;
        int iOffset = 0;
        //int iTextLen = 0;
        bool bSmartOrder = 1;
        //unsigned int iBufLength = 4;

        bool bOK = 0;

        if(strcmp(cmd_name, "pause") == 0 )
        {
            iAction = 12; // pause
            force_pause_nzb = 1;
            //g_pOptions->SetPauseDownload(true);

            if(log_s.status == S_PROCESSING)
            {
                log_s.status = S_PAUSED;
                log_s.ifile.inzb.rate = 0.0;
            }

            fp = fopen(path, "w");

            if(fp)
            {
                fwrite(&log_s, 1, LOG_SIZE, fp);
                fclose(fp);
            }
        }
        else if(strcmp(cmd_name, "all_paused") == 0 )
        {
            iAction = 12;
            force_pause_nzb = 1;
        }
        else if(strcmp(cmd_name, "start") == 0 )
        {
            iAction = 13; // start
            force_pause_nzb = 0;

            int diskfull;
            diskfull = checkDiskSpace(log_s);

            if(diskfull == 1)
            {
                if(log_s.status == S_PAUSED )
                {
                    log_s.status = S_DISKFULL;

                    fp = fopen(path, "w");

                    if(fp)
                    {
                        fwrite(&log_s, 1, LOG_SIZE, fp);
                        fclose(fp);
                    }

                    return ;
                }

                if( log_s.status == S_DISKFULL )
                    return ;
            }
            else
            {
                if(log_s.status == S_PAUSED || log_s.status == S_DISKFULL)
                {
                    log_s.status = S_PROCESSING;
                    auto_paused = 0;

                    fp = fopen(path, "w");

                    if(fp)
                    {
                        fwrite(&log_s, 1, LOG_SIZE, fp);
                        fclose(fp);
                    }
                }
            }
        }
        else if(strcmp(cmd_name, "all_start") == 0 )
        {

            iAction = 13;
            force_pause_nzb = 0;
            //info("iaction is %d,force_pause_nzb is %d",iAction,force_pause_nzb);
        }
        else if(strcmp(cmd_name, "cancel") == 0 )
        {
            if(disk_full_paused == 1)
            {
                g_pOptions->SetPauseDownload(false); // disk full  set true ##add by gauss
                disk_full_paused = 0 ;
            }

            iAction = 55; // cancel history

            have_error_log = 0 ; // init error log

            auto_paused = 0 ;   //reset disk full global value

            force_cancle_nzb = 1 ;

            GroupQueue groupQueue;
            groupQueue.clear();
            DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
            pDownloadQueue->BuildGroups(&groupQueue);
            g_pQueueCoordinator->UnlockQueue();

            for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
            {
                    GroupInfo* pGroupInfo = *it;
                    if( pGroupInfo->GetNZBInfo()->GetID() == task_id )
                    {
                        iAction = 14; // cancel current
                        break;
                    }
             }
        }
        else if(strcmp(cmd_name, "clean_completed") == 0)
        {
            return;
        }

        IDList cIDList;
        cIDList.reserve(iNrEntries);
        char szNZBNicename[1024],szNZBNicename_completed[1024],NZBFileName_completed[1024];

        GroupQueue groupQueue;
        groupQueue.clear();
        DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
        pDownloadQueue->BuildGroups(&groupQueue);
        g_pQueueCoordinator->UnlockQueue();

        for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
        {
            GroupInfo* pGroupInfo = *it;
            if( pGroupInfo->GetNZBInfo()->GetID() == task_id )
            {
                pGroupInfo->GetNZBInfo()->GetNiceNZBName(szNZBNicename, sizeof(szNZBNicename));
            }

        }

        for (int i = 0; i < iNrEntries; i++)
        {
            if (iAction < eRemoteEditActionPostMoveOffset)
            {
                GroupQueue groupQueue;
                groupQueue.clear();
                DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
                pDownloadQueue->BuildGroups(&groupQueue);
                g_pQueueCoordinator->UnlockQueue();

                for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
                {
                    GroupInfo* pGroupInfo = *it;
                    if( pGroupInfo->GetNZBInfo()->GetID() == task_id )
                    {
                        cIDList.push_back(pGroupInfo->GetLastID());
                        break;
                    }

                }
            }
            else
            {
                cIDList.push_back(task_id);
            }

        }

        //test
        //for(IDList::iterator iter = cIDList.begin(); iter != cIDList.end(); iter++)
            //info("IDlist is %d",*iter);

        if (iAction < eRemoteEditActionPostMoveOffset && iAction > 0)
        {
            //info("iAction is %d",iAction);
            bOK = g_pQueueCoordinator->GetQueueEditor()->EditList(&cIDList, bSmartOrder, (QueueEditor::EEditAction)iAction, iOffset, NULL);

        }
        else // del history
        {
            DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
            for (HistoryList::iterator it = pDownloadQueue->GetHistoryList()->begin(); it != pDownloadQueue->GetHistoryList()->end(); it++)
            {
                NZBInfo* pNZBInfo = *it;
                //info("nzb id is %d",pNZBInfo->GetID());
                if(pNZBInfo->GetID() == task_id )
                {
                    pNZBInfo->GetNiceNZBName(szNZBNicename_completed, sizeof(szNZBNicename_completed));
                    sprintf(NZBFileName_completed,"%s%s.nzb",g_pOptions->GetDestDir(), szNZBNicename_completed);
                    //info("NZBFilename is %s",NZBFileName_completed);

                }
            }
            g_pQueueCoordinator->UnlockQueue();

            //info("iAction is %d",iAction);

            bOK = g_pPrePostProcessor->QueueEditList(&cIDList, (PrePostProcessor::EEditAction)iAction, iOffset);
        }

        //    printf("cancel bOK: %d\n", bOK);


        if ( iAction != 12 && iAction != 13 )
        {
            // delete log
            int er = remove(path);

            if(er < 0 )
            {
                //info("can't remove logname is %s \n",path);
                //printf("can't remove logname is %s \n",path);
            }
#ifdef SEMA
            sema_unlink(sem_name);
#endif

            //delete incomplete files add by gauss

	    char src_path[256],t_src[256], NZBFilename[256];

	    memset(src_path,'\0',sizeof(src_path));
	    memset(NZBFilename,'\0',sizeof(NZBFilename));
	    memset(t_src, 0, sizeof(t_src));

            if(strlen(szNZBNicename) != 0)
            {
                    //g_pOptions->

                    sprintf(src_path,"%s%s",g_pOptions->GetDestDir(), szNZBNicename);
                    sprintf(NZBFilename,"%s%s.nzb",g_pOptions->GetDestDir(), szNZBNicename);

                    struct dirent* ent = NULL;
                    DIR *pDir;
                    pDir=opendir(src_path);
                    if(pDir != NULL )
                    {
                        while (NULL != (ent=readdir(pDir)))
                        {
                                if( strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 )
                                    continue;

                                //strcpy(t_src,src_path);
                                //strcat(t_src,ent->d_name);
                                memset(t_src, 0, sizeof(t_src));
                                sprintf(t_src,"%s/%s",src_path,ent->d_name);
                                remove(t_src);
                                //memset(t_src, 0, sizeof(t_src));

                        }
                        closedir(pDir);
                        //info("src_path is %s",src_path);

                        while(1)
                            if ((rmdir(src_path)) == 0)
                                  break;
                    }

                    FILE *fp2 = fopen(NZBFilename, "r");

                    if(fp2)
                    {
                       remove(NZBFilename);  //delete nzb file
                       fclose(fp2);
                    }

            }
            else {

                FILE *fp2 = fopen(NZBFileName_completed, "r");

                if(fp2)
                {
                   remove(NZBFileName_completed);  //delete nzb file
                   fclose(fp2);
                }
            }

            
        //    printf("remove file er: %d\n", er)
        //info("delete %s",path);

            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));
            snprintf(tmp, 1024, "Edit-Command completed successfully");
            tmp[1024-1] = '\0';
            send(m_iSocket, (char*) &tmp, sizeof(tmp), 0);
        }
        else
        {
            //info("#########can't into delete file function############");
            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));
            snprintf(tmp, 1024, "Edit-Command failed");
            tmp[1024-1] = '\0';
            send(m_iSocket, (char*) &tmp, sizeof(tmp), 0);
        }

    }
}
