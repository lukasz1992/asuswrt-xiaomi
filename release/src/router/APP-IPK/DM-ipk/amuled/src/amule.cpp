//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2011 aMule Team ( admin@amule.org / http://www.amule.org )
// Copyright (c) 2002-2011 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
// Any parts of this program derived from the xMule, lMule or eMule project,
// or contributed by third-party developers are copyrighted by their
// respective authors.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//


#include "amule.h"			// Interface declarations.
#include <csignal>
#include <cstring>
#include <wx/process.h>
#include <wx/sstream.h>	

#ifdef HAVE_CONFIG_H
	#include "config.h"		// Needed for HAVE_GETRLIMIT, HAVE_SETRLIMIT,
					//   HAVE_SYS_RESOURCE_H, HAVE_SYS_STATVFS_H, VERSION
					//   and ENABLE_NLS
#endif

#include <common/ClientVersion.h>

#include <wx/cmdline.h>			// Needed for wxCmdLineParser
#include <wx/config.h>			// Do_not_auto_remove (win32)
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>

#include <common/Format.h>		// Needed for CFormat
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/kademlia/Prefs.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "CanceledFileList.h"
#include "ClientCreditsList.h"		// Needed for CClientCreditsList
#include "ClientList.h"			// Needed for CClientList
#include "ClientUDPSocket.h"		// Needed for CClientUDPSocket & CMuleUDPSocket
#include "ExternalConn.h"		// Needed for ExternalConn & MuleConnection
#include <common/FileFunctions.h>	// Needed for CDirIterator
#include "FriendList.h"			// Needed for CFriendList
#include "HTTPDownload.h"		// Needed for CHTTPDownloadThread
#include "InternalEvents.h"		// Needed for CMuleInternalEvent
#include "IPFilter.h"			// Needed for CIPFilter
#include "KnownFileList.h"		// Needed for CKnownFileList
#include "ListenSocket.h"		// Needed for CListenSocket
#include "Logger.h"			// Needed for CLogger // Do_not_auto_remove
#include "MagnetURI.h"			// Needed for CMagnetURI
#include "OtherFunctions.h"
#include "PartFile.h"			// Needed for CPartFile
#include "PlatformSpecific.h"   // Needed for PlatformSpecific::AllowSleepMode();
#include "Preferences.h"		// Needed for CPreferences
#include "SearchList.h"			// Needed for CSearchList
#include "Server.h"			// Needed for GetListName
#include "ServerList.h"			// Needed for CServerList
#include "ServerConnect.h"              // Needed for CServerConnect
#include "ServerUDPSocket.h"		// Needed for CServerUDPSocket
#include "Statistics.h"			// Needed for CStatistics
#include "TerminationProcessAmuleweb.h"	// Needed for CTerminationProcessAmuleweb
#include "ThreadTasks.h"
#include "UploadQueue.h"		// Needed for CUploadQueue
#include "UploadBandwidthThrottler.h"
#include "UserEvents.h"
#include "ScopedPtr.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#ifdef ENABLE_UPNP
#include "UPnPBase.h"			// Needed for UPnP
#endif

#ifdef __WXMAC__
#include <wx/sysopt.h>			// Do_not_auto_remove
#endif

#ifndef AMULE_DAEMON
	#ifdef __WXMAC__
		#include <CoreFoundation/CFBundle.h>  // Do_not_auto_remove
		#if wxCHECK_VERSION(2, 9, 0)
			#include <wx/osx/core/cfstring.h>  // Do_not_auto_remove
		#else
			#include <wx/mac/corefoundation/cfstring.h>  // Do_not_auto_remove
		#endif
	#endif
	#include <wx/msgdlg.h>

	#include "amuleDlg.h"
#endif


#ifdef HAVE_SYS_RESOURCE_H
	#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
	#include <sys/statvfs.h>  // Do_not_auto_remove
#endif

char dm2_basedir[128]; //2012.02 magic added
char *dm2_server_msg="initializing";  //2012.0619 magic added
char server_ip[16];
char dldir[1024];

#ifdef __GLIBC__
# define RLIMIT_RESOURCE __rlimit_resource
#else
# define RLIMIT_RESOURCE int
#endif

#ifdef AMULE_DAEMON
CamuleDaemonApp *theApp;
#else
CamuleGuiApp *theApp;
#endif

static void UnlimitResource(RLIMIT_RESOURCE resType)
{
#if defined(HAVE_GETRLIMIT) && defined(HAVE_SETRLIMIT)
	struct rlimit rl;
//The following line is kept to allow checking the values of rl using strace.
	getrlimit(resType, &rl);
//	rl.rlim_cur = rl.rlim_max;
//setrlimit is buggy in uClibc-0.9.28 for mips.
//Its usage leads to a crash while downloading files above 1Gb.
//	setrlimit(resType, &rl);
#endif
}


static void SetResourceLimits()
{
#ifdef HAVE_SYS_RESOURCE_H
	UnlimitResource(RLIMIT_DATA);
#ifndef __UCLIBC__
	UnlimitResource(RLIMIT_FSIZE);
#endif
	UnlimitResource(RLIMIT_NOFILE);
#ifdef RLIMIT_RSS
	UnlimitResource(RLIMIT_RSS);
#endif
#endif
}

// We store the received signal in order to avoid race-conditions
// in the signal handler.
bool g_shutdownSignal = false;

Diskinfo disksave;

void OnShutdownSignal( int /* sig */ ) 
{
    //signal(SIGINT, SIG_DFL);
    //signal(SIGTERM, SIG_DFL);
	
	g_shutdownSignal = true;

#ifdef AMULE_DAEMON
	theApp->ExitMainLoop();
#endif
}

void getdmbasepath()
{
    memset(dm2_basedir, '\0', sizeof(dm2_basedir));
    if (access("/tmp/asus_router.conf",0) == 0)
    {
        int fd, len, i=0;
        char ch, tmp[256], name[256], content[256] ,result[64];
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        memset(result, 0, sizeof(result));

        if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0) {
        } else {
            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    if(!strcmp(name, "BASE_PATH"))
                    {
                        snprintf(result, 64, "%s", content);
                    }
                    continue;
                }
                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            strcpy(dm2_basedir,result);
            return ;
        }
    }
    else
        return ;

}

CamuleApp::CamuleApp()
{
	// Madcat - Initialize timer as the VERY FIRST thing to avoid any issues later.
	// Kry - I love to init the vars on init, even before timer.
	StartTickTimer();
	
	// Initialization
	m_app_state = APP_STATE_STARTING;

	theApp = &wxGetApp();
	getdmbasepath();
	//fprintf(stderr,"\ndm2_basedir=%s\n",dm2_basedir);
	clientlist	= NULL;
	searchlist	= NULL;
	knownfiles	= NULL;
	canceledfiles	= NULL;
	serverlist	= NULL;
	serverconnect	= NULL;
	sharedfiles	= NULL;
	listensocket	= NULL;
	clientudp	= NULL;
	clientcredits	= NULL;
	friendlist	= NULL;
	downloadqueue	= NULL;
	uploadqueue	= NULL;
	ipfilter	= NULL;
	ECServerHandler = NULL;
	glob_prefs	= NULL;
	m_statistics	= NULL;
	uploadBandwidthThrottler = NULL;
#ifdef ENABLE_UPNP
	m_upnp		= NULL;
	m_upnpMappings.resize(4);
#endif
	core_timer	= NULL;
	
	m_localip	= 0;
	m_dwPublicIP	= 0;
	webserver_pid	= 0;

	enable_daemon_fork = false;

	strFullMuleVersion = NULL;
	strOSDescription = NULL;
	
	// Apprently needed for *BSD
	SetResourceLimits();

#ifdef _MSC_VER
	_CrtSetDbgFlag(0);		// Disable useless memleak debugging
#endif
}

CamuleApp::~CamuleApp()
{
	// Closing the log-file as the very last thing, since
	// wxWidgets log-events are saved in it as well.
	theLogger.CloseLogfile();

	if (strFullMuleVersion) {
		free(strFullMuleVersion);
	}
	if (strOSDescription) {
		free(strOSDescription);
	}
}

int CamuleApp::OnExit()
{
	if (m_app_state!=APP_STATE_STARTING) {
		AddLogLineNS(_("Now, exiting main app..."));
	}

	// From wxWidgets docs, wxConfigBase:
	// ...
	// Note that you must delete this object (usually in wxApp::OnExit)
	// in order to avoid memory leaks, wxWidgets won't do it automatically.
	// 
	// As it happens, you may even further simplify the procedure described
	// above: you may forget about calling Set(). When Get() is called and
	// there is no current object, it will create one using Create() function.
	// To disable this behaviour DontCreateOnDemand() is provided.
	delete wxConfigBase::Set((wxConfigBase *)NULL);

	// Save credits
	clientcredits->SaveList();

	// Kill amuleweb if running
	if (webserver_pid) {
		AddLogLineNS(CFormat(_("Terminating amuleweb instance with pid '%ld' ... ")) % webserver_pid);
		wxKillError rc;
		if (wxKill(webserver_pid, wxSIGTERM, &rc) == -1) {
			AddLogLineNS(CFormat(_("Killing amuleweb instance with pid '%ld' ... ")) % webserver_pid);
			if (wxKill(webserver_pid, wxSIGKILL, &rc) == -1) {
				AddLogLineNS(_("Failed"));
			}
		}
	}

    disksave.free_disk_struc(&(disksave.follow_disk_info_start));//neal move to here to solve the error

	if (m_app_state!=APP_STATE_STARTING) {
		AddLogLineNS(_("aMule OnExit: Terminating core."));
	}

    delete serverlist; //at here saver the server list neal
	serverlist = NULL;

	delete searchlist;
	searchlist = NULL;

	delete clientcredits;
	clientcredits = NULL;

	delete friendlist;
	friendlist = NULL;

	// Destroying CDownloadQueue calls destructor for CPartFile
	// calling CSharedFileList::SafeAddKFile occasionally.
	delete sharedfiles;
	sharedfiles = NULL;

	delete serverconnect;
	serverconnect = NULL;

	delete listensocket;
	listensocket = NULL;

	delete clientudp;
	clientudp = NULL;

	delete knownfiles;
	knownfiles = NULL;

	delete canceledfiles;
	canceledfiles = NULL;

	delete clientlist;
	clientlist = NULL;

	delete uploadqueue;
	uploadqueue = NULL;

	delete downloadqueue;
	downloadqueue = NULL;

	delete ipfilter;
	ipfilter = NULL;

#ifdef ENABLE_UPNP
	delete m_upnp;
	m_upnp = NULL;
#endif

	delete ECServerHandler;
	ECServerHandler = NULL;

	delete m_statistics;
	m_statistics = NULL;

	delete glob_prefs;
	glob_prefs = NULL;
	CPreferences::EraseItemList();

	delete uploadBandwidthThrottler;
	uploadBandwidthThrottler = NULL;

	if (m_app_state!=APP_STATE_STARTING) {
		AddLogLineNS(_("aMule shutdown completed."));
	}

#if wxUSE_MEMORY_TRACING
	AddLogLineNS(_("Memory debug results for aMule exit:"));
	// Log mem debug mesages to wxLogStderr
	wxLog* oldLog = wxLog::SetActiveTarget(new wxLogStderr);
	//AddLogLineNS(wxT("**************Classes**************");
	//wxDebugContext::PrintClasses();
	//AddLogLineNS(wxT("***************Dump***************");
	//wxDebugContext::Dump();
	AddLogLineNS(wxT("***************Stats**************"));
	wxDebugContext::PrintStatistics(true);

	// Set back to wxLogGui
	delete wxLog::SetActiveTarget(oldLog);
#endif

	StopTickTimer();

	// Return 0 for succesful program termination
	return AMULE_APP_BASE::OnExit();
}


int CamuleApp::InitGui(bool, wxString &)
{
	return 0;
}


struct disk_info * Diskinfo::initial_disk_data(struct disk_info **disk)
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

void Diskinfo::free_disk_struc(struct disk_info **disk)
{
    struct disk_info **follow_disk, **old_disk;

    if(disk == NULL)
        return;

    follow_disk = disk;
    while((*follow_disk) != NULL)
    {
        if((*follow_disk)->diskname != NULL)
            free((*follow_disk)->diskname);
        if((*follow_disk)->diskpartition != NULL)
            free((*follow_disk)->diskpartition);
        if((*follow_disk)->mountpath != NULL)
            free((*follow_disk)->mountpath);
        if((*follow_disk)->serialnum != NULL)
            free((*follow_disk)->serialnum);
        if((*follow_disk)->product != NULL)
            free((*follow_disk)->product);
        if((*follow_disk)->vendor != NULL)
            free((*follow_disk)->vendor);

        old_disk = follow_disk;
        (*follow_disk) = (*follow_disk)->next;
        free(*old_disk);
        *old_disk = NULL;
    }
}
void createconfigfile()
{
    FILE *fp;
    if(access("/opt/etc/dm2_amule",0) != 0)
        mkdir("/opt/etc/dm2_amule",0777);
    //fprintf(stderr,"\nconfig file is not exists\n");
	char sbuf1[] = "[eMule]\nAppVersion=SVN\nNick=http://www.aMule.org\nQueueSizePref=50\nMaxUpload=150\nMaxDownload=0\nSlotAllocation=2\n"
                  "Port=4662\nUDPPort=4672\nUDPEnable=1\nAddress=\nAutoconnect=1\nMaxSourcesPerFile=300\nMaxConnections=500\nMaxConnectionsPerFiveSeconds=20\n"
                  "RemoveDeadServer=1\nDeadServerRetry=3\nServerKeepAliveTimeout=0\nReconnect=1\nScoresystem=1\nServerlist=0\nAddServerListFromServer=0\n"
                  "AddServerListFromClient=0\nSafeServerConnect=0\nAutoConnectStaticOnly=0\nUPnPEnabled=0\nUPnPTCPPort=50000\nSmartIdCheck=1\nConnectToKad=1\n"
                  "ConnectToED2K=1\nTempDir=";
    char sbuf2[] = "IncomingDir=/opt/tmp\nICH=1\nAICHTrust=0\nCheckDiskspace=1\nMinFreeDiskSpace=1\nAddNewFilesPaused=0\nPreviewPrio=0\nManualHighPrio=0\nStartNextFile=0\nStartNextFileSameCat=0\n"
                   "StartNextFileAlpha=0\nFileBufferSizePref=16\nDAPPref=1\nUAPPref=1\nAllocateFullFile=0\nOSDirectory=/opt/etc/dm2_amule/\nOnlineSignature=0\n"
                   "OnlineSignatureUpdate=5\nEnableTrayIcon=0\nMinToTray=0\nConfirmExit=1\nStartupMinimized=0\n3DDepth=10\nToolTipDelay=1\nShowOverhead=0\n"
                   "ShowInfoOnCatTabs=1\nVerticalToolbar=0\nGeoIPEnabled=1\nVideoPlayer=\nStatGraphsInterval=3\nstatsInterval=30\nDownloadCapacity=300\n"
                   "UploadCapacity=100\nStatsAverageMinutes=5\nVariousStatisticsMaxValue=100\nSeeShare=2\nFilterLanIPs=1\nParanoidFiltering=1\nIPFilterAutoLoad=1\n"
                   "IPFilterURL=\nFilterLevel=127\nIPFilterSystem=0\nFilterMessages=1\nFilterAllMessages=0\nMessagesFromFriendsOnly=0\nMessageFromValidSourcesOnly=1\nFilterWordMessages=0\n"
                   "MessageFilter=\nShowMessagesInLog=1\nFilterComments=0\nCommentFilter=\nShareHiddenFiles=0\nAutoSortDownloads=0\nNewVersionCheck=1\nAdvancedSpamFilter=1\n"
                   "MessageUseCaptchas=1\nLanguage=\nSplitterbarPosition=75\nYourHostname=\nDateTimeFormat=%A, %x, %X\nAllcatType=0\nShowAllNotCats=0\nSmartIdState=0\nDropSlowSources=0\n"
                   "KadNodesUrl=http://download.tuxfamily.org/technosalad/utils/nodes.dat\nEd2kServersUrl=http://gruk.org/server.met.gz\nShowRatesOnTitle=0\nGeoLiteCountryUpdateUrl=http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz\n"
                   "StatsServerName=Shorty's ED2K stats\nStatsServerURL=http://ed2k.shortypower.dyndns.org/?hash=\n[Browser]\nOpenPageInTab=1\nCustomBrowserString=\n[Proxy]\nProxyEnableProxy=0\nProxyType=0\nProxyName=\nProxyPort=1080\nProxyEnablePassword=0\n"
                   "ProxyUser=\nProxyPassword=\n[ExternalConnect]\nUseSrcSeeds=0\nAcceptExternalConnections=1\nECAddress=\nECPort=4712\nECPassword=21232f297a57a5a743894a0e4a801fc3\nUPnPECEnabled=0\nShowProgressBar=1\nShowPercent=1\nUseSecIdent=1\n"
                   "IpFilterClients=1\nIpFilterServers=1\nTransmitOnlyUploadingClients=0\n[WebServer]\nEnabled=0\nPassword=\nPasswordLow=\nPort=4711\nWebUPnPTCPPort=50001\nUPnPWebServerEnabled=0\nUseGzip=1\nUseLowRightsUser=0\nPageRefreshTime=120\n"
                   "Template=\nPath=amuleweb\n[Razor_Preferences]\nFastED2KLinksHandler=1\n[SkinGUIOptions]\nSkin=\n[Statistics]\nMaxClientVersions=0\nTotalDownloadedBytes=5459968\nTotalUploadedBytes=264354\n[Obfuscation]\nIsClientCryptLayerSupported=1\n"
                   "IsCryptLayerRequested=1\nIsClientCryptLayerRequired=0\nCryptoPaddingLenght=254\nCryptoKadUDPKey=1836875778\n[PowerManagement]\nPreventSleepWhileDownloading=0\n[UserEvents]\n[UserEvents/DownloadCompleted]\nCoreEnabled=0\n"
                   "CoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/NewChatSession]\nCoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/OutOfDiskSpace]\nCoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/ErrorOnCompletion]\n"
                   "CoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[HTTPDownload]\nURL_1=\nURL_4=http://amule.sourceforge.net/lastversion";
    unsigned int buflen = strlen(sbuf1) + strlen(sbuf2) + 50 + strlen(dm2_basedir);
    char sbuf[buflen];
    memset(sbuf,'\0',sizeof(sbuf));
    sprintf(sbuf,"%s%s/Download2/InComplete\n%s",sbuf1,dm2_basedir,sbuf2);
    fp = fopen("/opt/etc/dm2_amule/dm2_amule.conf","w");
    if(fp)
    {
        fprintf(fp,"%s",sbuf);
        fclose(fp);
    }
}

int Diskinfo::init_diskinfo_struct()
{
    int len = 0;
    FILE *fp;
    if(access("/tmp/usbinfo",0)==0)
    {
        fp =fopen("/tmp/usbinfo","r");
        if(fp)
        {
            fseek(fp,0,SEEK_END);
            len = ftell(fp);
            fseek(fp,0,SEEK_SET);
        }
    }
    else
        return -1;

    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    fread(buf,1,len,fp);
    fclose(fp);
    //fprintf(stderr,"\nbuf=%s\n",buf);

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

               follow_disk_info->next =  follow_disk_tmp;
               follow_disk_info = follow_disk_tmp;
           }
       }
    }
    fclose(fp);
	return 0;
}

//
// Application initialization
//
bool CamuleApp::OnInit()
{
#if wxUSE_MEMORY_TRACING
	// any text before call of Localize_mule needs not to be translated.
	AddLogLineNS(wxT("Checkpoint set on app init for memory debug"));	// debug output
	wxDebugContext::SetCheckpoint();
#endif

	// Forward wxLog events to CLogger
	wxLog::SetActiveTarget(new CLoggerTarget);
	
	m_localip = StringHosttoUint32(::wxGetFullHostName());

#ifndef __WXMSW__
	// get rid of sigpipe
	signal(SIGPIPE, SIG_IGN);
#else
	// Handle CTRL-Break
	signal(SIGBREAK, OnShutdownSignal);
#endif
	// Handle sigint and sigterm
	signal(SIGINT, OnShutdownSignal);
	signal(SIGTERM, OnShutdownSignal);

#ifdef __WXMAC__
	// For listctrl's to behave on Mac
	wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"), 1);
#endif

	// Handle uncaught exceptions
	InstallMuleExceptionHandler();
	FILE *fp;
            fp = fopen("/opt/etc/dm2_general.conf","r");
            char a[50];
            char dldirtmp[200];
            char *p;
            memset(dldirtmp,'\0',sizeof(dldirtmp));
            memset(dldir,'\0',sizeof(dldir));
            //memset(disksave.generalbuf,'\0',sizeof(disksave.generalbuf));
            memset(disksave.serialtmp,'\0',sizeof(disksave.serialtmp));
            memset(disksave.producttmp,'\0',sizeof(disksave.producttmp));
            memset(disksave.vondertmp,'\0',sizeof(disksave.vondertmp));
            while(!feof(fp))
            {
                memset(a,'\0',sizeof(a));
                fscanf(fp,"%[^\n]%*c",a);
                if(p = strstr(a,"$EX_MAINDIR"))
                {
                    p = p + 12;
                    strncpy(dldirtmp,p,strlen(p));
                    continue;
                }
                else if(p = strstr(a,"EX_DOWNLOAD_PATH"))
                {
                    p = p + 17;
                    strcat(dldirtmp,"/");
                    strcat(dldirtmp,p);
                    continue;
                }
               else if(p = strstr(a,"Download_dir="))
                {
                    p = p + 13;
                    strncpy(dldir,p,strlen(p));
                    continue;
                }
                else if(strncmp(a,"serial=",7) == 0)
                {
                    strcpy(disksave.serialtmp,a+7);
                    continue;
                }
                else if(strncmp(a,"vonder=",7) == 0)
                {
                    strcpy(disksave.vondertmp,a+7);
                    continue;
                }
                else if(strncmp(a,"product=",8) == 0)
                {
                    strcpy(disksave.producttmp,a+8);
                    continue;
                }
                else if(strncmp(a,"partition=",10) == 0)
                {
                    disksave.partitiontmp = atoi(a+10);
                    continue;
                }
            }
            fclose(fp);
        //eric added for have no config file exist 2012.11.28{
        char configpath[] = "/opt/etc/dm2_amule/dm2_amule.conf";
        int length;
        int err = 0;
        char *begin,*q;
        char *checkbuf;
        if(access(configpath,0) == 0)
        {
            //fprintf(stderr,"\nconfig file is exists\n");
            fp = fopen(configpath,"r");
            if(fp)
            {
                fseek(fp,0,SEEK_END);
                length = ftell(fp);
                fseek(fp,0,SEEK_SET);
                checkbuf = (char *)malloc(length + 1);
                memset(checkbuf,'\0',sizeof(checkbuf));
                fread(checkbuf,1,length,fp);
                fclose(fp);
            }
            begin = checkbuf;
            p = strstr(checkbuf,"TempDir=");
            if(p == NULL)
                err = 1;
			else if(strncmp(checkbuf,"[eMule]\nAppVersion=SVN\nNick=http://www.aMule.org\nQueueSizePref=50\nMaxUpload=150\nMaxDownload=0\n"
                            "SlotAllocation=2\nPort=4662\nUDPPort=4672\nUDPEnable=1\nAddress=\nAutoconnect=1\nMaxSourcesPerFile=300\nMaxConnections=500\n"
                            "MaxConnectionsPerFiveSeconds=20\nRemoveDeadServer=1\nDeadServerRetry=3\nServerKeepAliveTimeout=0\nReconnect=1\nScoresystem=1\n"
                            "Serverlist=0\nAddServerListFromServer=0\nAddServerListFromClient=0\nSafeServerConnect=0\nAutoConnectStaticOnly=0\nUPnPEnabled=0\n"
                            "UPnPTCPPort=50000\nSmartIdCheck=1\nConnectToKad=1\nConnectToED2K=1\n",strlen(begin) - strlen(p)) != 0)
                err = 2;

            q = strstr(checkbuf,"IncomingDir=");
            if(q == NULL)
                err = 3;
            else if(strncmp(q,"IncomingDir=/opt/tmp\nICH=1\nAICHTrust=0\nCheckDiskspace=1\nMinFreeDiskSpace=1\nAddNewFilesPaused=0\nPreviewPrio=0\nManualHighPrio=0\n"
                            "StartNextFile=0\nStartNextFileSameCat=0\nStartNextFileAlpha=0\nFileBufferSizePref=16\nDAPPref=1\nUAPPref=1\nAllocateFullFile=0\nOSDirectory=/opt/etc/dm2_amule/\n"
                            "OnlineSignature=0\nOnlineSignatureUpdate=5\nEnableTrayIcon=0\nMinToTray=0\nConfirmExit=1\nStartupMinimized=0\n3DDepth=10\nToolTipDelay=1\nShowOverhead=0\nShowInfoOnCatTabs=1\nVerticalToolbar=0\n"
                            "GeoIPEnabled=1\nVideoPlayer=\nStatGraphsInterval=3\nstatsInterval=30\nDownloadCapacity=300\nUploadCapacity=100\nStatsAverageMinutes=5\nVariousStatisticsMaxValue=100\nSeeShare=2\nFilterLanIPs=1\nParanoidFiltering=1\n"
                            "IPFilterAutoLoad=1\nIPFilterURL=\nFilterLevel=127\nIPFilterSystem=0\nFilterMessages=1\nFilterAllMessages=0\nMessagesFromFriendsOnly=0\nMessageFromValidSourcesOnly=1\nFilterWordMessages=0\nMessageFilter=\nShowMessagesInLog=1\nFilterComments=0\n"
                            "CommentFilter=\nShareHiddenFiles=0\nAutoSortDownloads=0\nNewVersionCheck=1\nAdvancedSpamFilter=1\nMessageUseCaptchas=1\nLanguage=\nSplitterbarPosition=75\nYourHostname=\nDateTimeFormat=%A, %x, %X\nAllcatType=0\nShowAllNotCats=0\nSmartIdState=0\n"
                            "DropSlowSources=0\nKadNodesUrl=http://download.tuxfamily.org/technosalad/utils/nodes.dat\nEd2kServersUrl=http://gruk.org/server.met.gz\nShowRatesOnTitle=0\nGeoLiteCountryUpdateUrl=http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz\n"
                            "StatsServerName=Shorty's ED2K stats\nStatsServerURL=http://ed2k.shortypower.dyndns.org/?hash=\n[Browser]\nOpenPageInTab=1\nCustomBrowserString=\n[Proxy]\nProxyEnableProxy=0\nProxyType=0\nProxyName=\nProxyPort=1080\nProxyEnablePassword=0\nProxyUser=\nProxyPassword=\n"
                            "[ExternalConnect]\nUseSrcSeeds=0\nAcceptExternalConnections=1\nECAddress=\nECPort=4712\nECPassword=21232f297a57a5a743894a0e4a801fc3\nUPnPECEnabled=0\nShowProgressBar=1\nShowPercent=1\nUseSecIdent=1\nIpFilterClients=1\nIpFilterServers=1\nTransmitOnlyUploadingClients=0\n"
                            "[WebServer]\nEnabled=0\nPassword=\nPasswordLow=\nPort=4711\nWebUPnPTCPPort=50001\nUPnPWebServerEnabled=0\nUseGzip=1\nUseLowRightsUser=0\nPageRefreshTime=120\nTemplate=\nPath=amuleweb\n[Razor_Preferences]\nFastED2KLinksHandler=1\n[SkinGUIOptions]\nSkin=\n[Statistics]\nMaxClientVersions=0\n"
                            "TotalDownloadedBytes=5459968\nTotalUploadedBytes=264354\n[Obfuscation]\nIsClientCryptLayerSupported=1\nIsCryptLayerRequested=1\nIsClientCryptLayerRequired=0\nCryptoPaddingLenght=254\nCryptoKadUDPKey=1836875778\n[PowerManagement]\nPreventSleepWhileDownloading=0\n[UserEvents]\n[UserEvents/DownloadCompleted]\n"
                            "CoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/NewChatSession]\nCoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/OutOfDiskSpace]\nCoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n[UserEvents/ErrorOnCompletion]\nCoreEnabled=0\nCoreCommand=\nGUIEnabled=0\nGUICommand=\n"
                            "[HTTPDownload]\nURL_1=\nURL_4=http://amule.sourceforge.net/lastversion",(strlen(q)-1)) != 0)
                err = 4;

            char IncomingDir[128];
            if( ( p != NULL ) && ( q != NULL ) )
            {
                memset(IncomingDir,'\0',sizeof(IncomingDir));
                p = p + 8;
                snprintf(IncomingDir,strlen(p)-strlen(q),p);
                fprintf(stderr,"\nIncomingDir=%s\n",IncomingDir);
                if((strncmp(IncomingDir,dm2_basedir,strlen(dm2_basedir)) != 0) && (strstr(IncomingDir,"/Download2/InComplete") != NULL))
                    err = 5;
            }
            //fprintf(stderr,"\n###########err=%d\n",err);
            free(checkbuf);

            if((err != 0) && (remove(configpath) == 0))
                createconfigfile();
        }
        else
        {
            createconfigfile();
        }//eric added for have no config file exist 2012.11.28}


	if (!InitCommon(AMULE_APP_BASE::argc, AMULE_APP_BASE::argv)) {
		return false;
	}

	glob_prefs = new CPreferences();

        /*****************eric added for diskinfo**************/
        if(disksave.init_diskinfo_struct()==-1)
            printf("\ninit_diskinfo_struct failed\n");
        CPath outDir;//eric debug for load .part file 2013.3.1
	if (CheckMuleDirectory(wxT("temp"), thePrefs::GetTempDir(), ConfigDir + wxT("Temp"), outDir)) {
		thePrefs::SetTempDir(outDir);
	} else {
		return false;
	}
	
	if (CheckMuleDirectory(wxT("incoming"), thePrefs::GetIncomingDir(), ConfigDir + wxT("Incoming"), outDir)) {
		thePrefs::SetIncomingDir(outDir);
	} else {
		return false;
	}

	// Some sanity check
	if (!thePrefs::UseTrayIcon()) {
		thePrefs::SetMinToTray(false);
	}

	// Build the filenames for the two OS files
	SetOSFiles(thePrefs::GetOSDir().GetRaw());

#ifdef ENABLE_NLS
	// Load localization settings
	Localize_mule();
#endif

	// Configure EC for amuled when invoked with ec-config
	if (ec_config) {
		AddLogLineNS(_("\nEC configuration"));
		thePrefs::SetECPass(GetPassword());
		thePrefs::EnableExternalConnections(true);
		AddLogLineNS(_("Password set and external connections enabled."));
	}
	
/*#ifndef __WXMSW__
	if (getuid() == 0) {
		wxString msg = 
			wxT("Warning! You are running aMule as root.\n")
			wxT("Doing so is not recommended for security reasons,\n")
			wxT("and you are advised to run aMule as an normal\n")
			wxT("user instead.");
		
		ShowAlert(msg, _("WARNING"), wxCENTRE | wxOK | wxICON_ERROR);
	
		fprintf(stderr, "\n--------------------------------------------------\n");
		fprintf(stderr, "%s", (const char*)unicode2UTF8(msg));
		fprintf(stderr, "\n--------------------------------------------------\n\n");
	}
#endif*/
	
	// Display notification on new version or first run
	wxTextFile vfile( ConfigDir + wxT("lastversion") );
	wxString newMule(wxT( VERSION ));
	
	if ( !wxFileExists( vfile.GetName() ) ) {
		vfile.Create();
	}

	if ( vfile.Open() ) {
		// Check if this version has been run before
		bool found = false;
		for ( size_t i = 0; i < vfile.GetLineCount(); i++ ) {
			// Check if this version has been run before
			if ( vfile.GetLine(i) == newMule ) {
				found = true;
				break;
			}
		}

		// We havent run this version before?
		if ( !found ) {
			// Insert new at top to provide faster searches
			vfile.InsertLine( newMule, 0 );
			
			Trigger_New_version( newMule );
		}
		
		// Keep at most 10 entires
		while ( vfile.GetLineCount() > 10 )
			vfile.RemoveLine( vfile.GetLineCount() - 1 );
			
		vfile.Write();
		vfile.Close();
	}

	// Check if we have the old style locale config
	wxString langId = thePrefs::GetLanguageID();
	if (!langId.IsEmpty() && (langId.GetChar(0) >= '0' && langId.GetChar(0) <= '9')) {
		wxString info(_("Your locale has been changed to System Default due to a configuration change. Sorry."));
		thePrefs::SetLanguageID(wxLang2Str(wxLANGUAGE_DEFAULT));
		ShowAlert(info, _("Info"), wxCENTRE | wxOK | wxICON_ERROR);
	}

	m_statistics = new CStatistics();
	
	clientlist	= new CClientList();
	friendlist	= new CFriendList();
	searchlist	= new CSearchList();
	knownfiles	= new CKnownFileList();
	canceledfiles	= new CCanceledFileList;
	serverlist	= new CServerList();
	
	sharedfiles	= new CSharedFileList(knownfiles);
	clientcredits	= new CClientCreditsList();
	
	// bugfix - do this before creating the uploadqueue
	downloadqueue	= new CDownloadQueue();
	uploadqueue	= new CUploadQueue();
	ipfilter	= new CIPFilter();

	// Creates all needed listening sockets
	wxString msg;
	if (!ReinitializeNetwork(&msg)) {
		AddLogLineNS(wxT("\n"));
		AddLogLineNS(msg);
	}

	// Test if there's any new version
	if (thePrefs::GetCheckNewVersion()) {
		// We use the thread base because I don't want a dialog to pop up.
		CHTTPDownloadThread* version_check = 
			new CHTTPDownloadThread(wxT("http://amule.sourceforge.net/lastversion"),
				theApp->ConfigDir + wxT("last_version_check"), theApp->ConfigDir + wxT("last_version"), HTTP_VersionCheck, false, true);
		version_check->Create();
		version_check->Run();
	}

	// Create main dialog, or fork to background (daemon).
	InitGui(m_geometryEnabled, m_geometryString);
	
#ifdef AMULE_DAEMON
	// Need to refresh wxSingleInstanceChecker after the daemon fork() !
	if (enable_daemon_fork) {
		RefreshSingleInstanceChecker();
		// No need to check IsAnotherRunning() - we've done it before.
	}
#endif

	// Has to be created after the call to InitGui, as fork 
	// (when using posix threads) only replicates the mainthread,
	// and the UBT constructor creates a thread.
	uploadBandwidthThrottler = new UploadBandwidthThrottler();
	
	// Start performing background tasks
	// This will start loading the IP filter. It will start right away.
	// Log is confusing, because log entries from background will only be printed
	// once foreground becomes idle, and that will only be after loading 
	// of the partfiles has finished.
	CThreadScheduler::Start();
	
	// These must be initialized after the gui is loaded.
	if (thePrefs::GetNetworkED2K()) {
		serverlist->Init();
	}

	downloadqueue->LoadMetFiles(thePrefs::GetTempDir());
	sharedfiles->Reload();
	
	// Ensure that the up/down ratio is used
	CPreferences::CheckUlDlRatio();

	// Load saved friendlist (now, so it can update in GUI right away)
	friendlist->LoadList();

	// The user can start pressing buttons like mad if he feels like it.
	m_app_state = APP_STATE_RUNNING;
	
	if (!serverlist->GetServerCount() && thePrefs::GetNetworkED2K()) {
		// There are no servers and ED2K active -> ask for download.
		// As we cannot ask in amuled, we just update there
		// Kry TODO: Store server.met URL on preferences and use it here and in GUI.
#ifndef AMULE_DAEMON
		if (wxYES == wxMessageBox(
			wxString(
				_("You don't have any server in the server list.\nDo you want aMule to download a new list now?")),
			wxString(_("Server list download")),
			wxYES_NO,
			static_cast<wxWindow*>(theApp->amuledlg)))
#endif
		{
		// workaround amuled crash
#ifndef AMULE_DAEMON
			serverlist->UpdateServerMetFromURL(
				wxT("http://gruk.org/server.met.gz"));
#endif
		}
	}
	
	
	// Autoconnect if that option is enabled
	if (thePrefs::DoAutoConnect()) {
		// IP filter is still loading and will be finished on event.
		// Tell it to autoconnect.
		if (thePrefs::GetNetworkED2K()) {
			ipfilter->ConnectToAnyServerWhenReady();
		}
		if (thePrefs::GetNetworkKademlia()) {
			ipfilter->StartKADWhenReady();
		}
	}

	// Enable GeoIP
#ifdef ENABLE_IP2COUNTRY
	theApp->amuledlg->EnableIP2Country();
#endif

	// Run webserver?
	if (thePrefs::GetWSIsEnabled()) {
		wxString aMuleConfigFile = ConfigDir + m_configFile;
		wxString amulewebPath = thePrefs::GetWSPath();

#if defined(__WXMAC__) && !defined(AMULE_DAEMON)
		// For the Mac GUI application, look for amuleweb in the bundle
		CFURLRef amulewebUrl = CFBundleCopyAuxiliaryExecutableURL(
			CFBundleGetMainBundle(), CFSTR("amuleweb"));

		if (amulewebUrl) {
			CFURLRef absoluteUrl = CFURLCopyAbsoluteURL(amulewebUrl);
			CFRelease(amulewebUrl);

			if (absoluteUrl) {
				CFStringRef amulewebCfstr = CFURLCopyFileSystemPath(absoluteUrl, kCFURLPOSIXPathStyle);
				CFRelease(absoluteUrl);
	#if wxCHECK_VERSION(2, 9, 0)
				amulewebPath = wxCFStringRef(amulewebCfstr).AsString(wxLocale::GetSystemEncoding());
	#else
				amulewebPath = wxMacCFStringHolder(amulewebCfstr).AsString(wxLocale::GetSystemEncoding());
	#endif
			}
		}
#endif

#ifdef __WXMSW__
#	define QUOTE	wxT("\"")
#else
#	define QUOTE	wxT("\'")
#endif

		wxString cmd =
			QUOTE +
			amulewebPath +
			QUOTE wxT(" ") QUOTE wxT("--amule-config-file=") +
			aMuleConfigFile +
			QUOTE;
		CTerminationProcessAmuleweb *p = new CTerminationProcessAmuleweb(cmd, &webserver_pid);
		webserver_pid = wxExecute(cmd, wxEXEC_ASYNC, p);
		bool webserver_ok = webserver_pid > 0;
		if (webserver_ok) {
			AddLogLineC(CFormat(_("web server running on pid %d")) % webserver_pid);
		} else {
			delete p;
			ShowAlert(_(
				"You requested to run web server on startup, but the amuleweb binary cannot be run. Please install the package containing aMule web server, or compile aMule using --enable-webserver and run make install"),
				_("ERROR"), wxOK | wxICON_ERROR);
		}
	}

	return true;
}


bool CamuleApp::ReinitializeNetwork(wxString* msg)
{
	bool ok = true;
	static bool firstTime = true;
	
	if (!firstTime) {
		// TODO: Destroy previously created sockets
	}
	firstTime = false;
	
	// Some sanity checks first
	if (thePrefs::ECPort() == thePrefs::GetPort()) {
		// Select a random usable port in the range 1025 ... 2^16 - 1
		uint16 port = thePrefs::ECPort();
		while ( port < 1024 || port  == thePrefs::GetPort() ) {
			port = (uint16)rand();
		}
		thePrefs::SetECPort( port );
		
		wxString err =
			wxT("Network configuration failed! You cannot use the same port\n")
			wxT("for the main TCP port and the External Connections port.\n") 
			wxT("The EC port has been changed to avoid conflict, see the\n")
			wxT("preferences for the new value.\n");
		*msg << err;

		AddLogLineN(wxEmptyString );
		AddLogLineC(err );
		AddLogLineN(wxEmptyString );

		ok = false;
	}
	
	if (thePrefs::GetUDPPort() == thePrefs::GetPort() + 3) {
		// Select a random usable value in the range 1025 ... 2^16 - 1
		uint16 port = thePrefs::GetUDPPort();
		while ( port < 1024 || port == thePrefs::GetPort() + 3 ) {
			port = (uint16)rand();
		}
		thePrefs::SetUDPPort( port );

		wxString err = 
			wxT("Network configuration failed! You set your UDP port to\n")
			wxT("the value of the main TCP port plus 3.\n")
			wxT("This port has been reserved for the Server-UDP port. The\n")
			wxT("port value has been changed to avoid conflict, see the\n")
			wxT("preferences for the new value\n");
		*msg << err;

		AddLogLineN(wxEmptyString );
		AddLogLineC(err );
		AddLogLineN(wxEmptyString );
		
		ok = false;
	}
	
	// Create the address where we are going to listen
	// TODO: read this from configuration file
	amuleIPV4Address myaddr[4];

	// Create the External Connections Socket.
	// Default is 4712.
	// Get ready to handle connections from apps like amulecmd
	if (thePrefs::GetECAddress().IsEmpty() || !myaddr[0].Hostname(thePrefs::GetECAddress())) {
		myaddr[0].AnyAddress();
	}
	myaddr[0].Service(thePrefs::ECPort());
	ECServerHandler = new ExternalConn(myaddr[0], msg);
	
	// Create the UDP socket TCP+3.
	// Used for source asking on servers.
	if (thePrefs::GetAddress().IsEmpty()) {
		myaddr[1].AnyAddress();
	} else if (!myaddr[1].Hostname(thePrefs::GetAddress())) {
		myaddr[1].AnyAddress();
		AddLogLineC(CFormat(_("Could not bind ports to the specified address: %s"))
			% thePrefs::GetAddress());				
	}

	wxString ip = myaddr[1].IPAddress();
	myaddr[1].Service(thePrefs::GetPort()+3);
	serverconnect = new CServerConnect(serverlist, myaddr[1]);
	*msg << CFormat( wxT("*** Server UDP socket (TCP+3) at %s:%u\n") )
		% ip % ((unsigned int)thePrefs::GetPort() + 3u);
	
	// Create the ListenSocket (aMule TCP socket).
	// Used for Client Port / Connections from other clients,
	// Client to Client Source Exchange.
	// Default is 4662.
	myaddr[2] = myaddr[1];
	myaddr[2].Service(thePrefs::GetPort());
	listensocket = new CListenSocket(myaddr[2]);
	*msg << CFormat( wxT("*** TCP socket (TCP) listening on %s:%u\n") )
		% ip % (unsigned int)(thePrefs::GetPort());
	// This command just sets a flag to control maximum number of connections.
	// Notify(true) has already been called to the ListenSocket, so events may
	// be already comming in.
	if (listensocket->Ok()) {
		listensocket->StartListening();
	} else {
		// If we wern't able to start listening, we need to warn the user
		wxString err;
		err = CFormat(_("Port %u is not available. You will be LOWID\n")) %
			(unsigned int)(thePrefs::GetPort());
		*msg << err;
		AddLogLineC(err);
		err.Clear();
		err = CFormat(
			_("Port %u is not available!\n\nThis means that you will be LOWID.\n\nCheck your network to make sure the port is open for output and input.")) % 
			(unsigned int)(thePrefs::GetPort());
		ShowAlert(err, _("ERROR"), wxOK | wxICON_ERROR);
	}

	// Create the UDP socket.
	// Used for extended eMule protocol, Queue Rating, File Reask Ping.
	// Also used for Kademlia.
	// Default is port 4672.
	myaddr[3] = myaddr[1];
	myaddr[3].Service(thePrefs::GetUDPPort());
	clientudp = new CClientUDPSocket(myaddr[3], thePrefs::GetProxyData());
	if (!thePrefs::IsUDPDisabled()) {
		*msg << CFormat( wxT("*** Client UDP socket (extended eMule) at %s:%u") )
			% ip % thePrefs::GetUDPPort();
	} else {
		*msg << wxT("*** Client UDP socket (extended eMule) disabled on preferences");
	}	

#ifdef ENABLE_UPNP
	if (thePrefs::GetUPnPEnabled()) {
		try {
			m_upnpMappings[0] = CUPnPPortMapping(
				myaddr[0].Service(),
				"TCP",
				thePrefs::GetUPnPECEnabled(),
				"aMule TCP External Connections Socket");
			m_upnpMappings[1] = CUPnPPortMapping(
				myaddr[1].Service(),
				"UDP",
				thePrefs::GetUPnPEnabled(),
				"aMule UDP socket (TCP+3)");
			m_upnpMappings[2] = CUPnPPortMapping(
				myaddr[2].Service(),
				"TCP",
				thePrefs::GetUPnPEnabled(),
				"aMule TCP Listen Socket");
			m_upnpMappings[3] = CUPnPPortMapping(
				myaddr[3].Service(),
				"UDP",
				thePrefs::GetUPnPEnabled(),
				"aMule UDP Extended eMule Socket");
			m_upnp = new CUPnPControlPoint(thePrefs::GetUPnPTCPPort());
			m_upnp->AddPortMappings(m_upnpMappings);
		} catch(CUPnPException &e) {
			wxString error_msg;
			error_msg << e.what();
			AddLogLineC(error_msg);
			fprintf(stderr, "%s\n", (const char *)unicode2char(error_msg));
		}
	}
#endif
	return ok;
}

/* Original implementation by Bouc7 of the eMule Project.
   aMule Signature idea was designed by BigBob and implemented
   by Un-Thesis, with design inputs and suggestions from bothie.
*/
void CamuleApp::OnlineSig(bool zero /* reset stats (used on shutdown) */)
{
	// Do not do anything if online signature is disabled in Preferences
	if (!thePrefs::IsOnlineSignatureEnabled() || m_emulesig_path.IsEmpty()) {
		// We do not need to check m_amulesig_path because if m_emulesig_path is empty,
		// that means m_amulesig_path is empty too.
		return;
	}

	// Remove old signature files
	if ( wxFileExists( m_emulesig_path ) ) { wxRemoveFile( m_emulesig_path ); }
	if ( wxFileExists( m_amulesig_path ) ) { wxRemoveFile( m_amulesig_path ); }


	wxTextFile amulesig_out;
	wxTextFile emulesig_out;
	
	// Open both files if needed
	if ( !emulesig_out.Create( m_emulesig_path) ) {
		AddLogLineC(_("Failed to create OnlineSig File"));
		// Will never try again.
		m_amulesig_path.Clear();
		m_emulesig_path.Clear();
		return;
	}

	if ( !amulesig_out.Create(m_amulesig_path) ) {
		AddLogLineC(_("Failed to create aMule OnlineSig File"));
		// Will never try again.
		m_amulesig_path.Clear();
		m_emulesig_path.Clear();
		return;
	}

	wxString emulesig_string;
	wxString temp;
	
	if (zero) {
		emulesig_string = wxT("0\xA0.0|0.0|0");
		amulesig_out.AddLine(wxT("0\n0\n0\n0\n0\n0\n0.0\n0.0\n0\n0"));
	} else {
		if (IsConnectedED2K()) {

			temp = CFormat(wxT("%d")) % serverconnect->GetCurrentServer()->GetPort();
			
			// We are online
			emulesig_string =
				// Connected
				wxT("1|")
				//Server name
				+ serverconnect->GetCurrentServer()->GetListName()
				+ wxT("|")
				// IP and port of the server
				+ serverconnect->GetCurrentServer()->GetFullIP()
				+ wxT("|")
				+ temp;


			// Now for amule sig

			// Connected. State 1, full info
			amulesig_out.AddLine(wxT("1"));
			// Server Name
			amulesig_out.AddLine(serverconnect->GetCurrentServer()->GetListName());
			// Server IP
			amulesig_out.AddLine(serverconnect->GetCurrentServer()->GetFullIP());
			// Server Port
			amulesig_out.AddLine(temp);

			if (serverconnect->IsLowID()) {
				amulesig_out.AddLine(wxT("L"));
			} else {
				amulesig_out.AddLine(wxT("H"));
			}

		} else if (serverconnect->IsConnecting()) {
			emulesig_string = wxT("0");

			// Connecting. State 2, No info.
			amulesig_out.AddLine(wxT("2\n0\n0\n0\n0"));
		} else {
			// Not connected to a server
			emulesig_string = wxT("0");

			// Not connected, state 0, no info
			amulesig_out.AddLine(wxT("0\n0\n0\n0\n0"));
		}
		if (IsConnectedKad()) {
			if(Kademlia::CKademlia::IsFirewalled()) {
				// Connected. Firewalled. State 1.
				amulesig_out.AddLine(wxT("1"));
			} else {
				// Connected. State 2.
				amulesig_out.AddLine(wxT("2"));
			}
		} else {
			// Not connected.State 0.
			amulesig_out.AddLine(wxT("0"));
		}
		emulesig_string += wxT("\xA");

		// Datarate for downloads
		temp = CFormat(wxT("%.1f")) % (theStats::GetDownloadRate() / 1024.0);

		emulesig_string += temp + wxT("|");
		amulesig_out.AddLine(temp);

		// Datarate for uploads
		temp = CFormat(wxT("%.1f")) % (theStats::GetUploadRate() / 1024.0);

		emulesig_string += temp + wxT("|");
		amulesig_out.AddLine(temp);

		// Number of users waiting for upload
		temp = CFormat(wxT("%d")) % theStats::GetWaitingUserCount();

		emulesig_string += temp; 
		amulesig_out.AddLine(temp);

		// Number of shared files (not on eMule)
		amulesig_out.AddLine(CFormat(wxT("%d")) % theStats::GetSharedFileCount());
	}

	// eMule signature finished here. Write the line to the wxTextFile.
	emulesig_out.AddLine(emulesig_string);

	// Now for aMule signature extras

	// Nick on the network
	amulesig_out.AddLine(thePrefs::GetUserNick());

	// Total received in bytes
	amulesig_out.AddLine( CFormat( wxT("%llu") ) % (theStats::GetSessionReceivedBytes() + thePrefs::GetTotalDownloaded()) );

	// Total sent in bytes
	amulesig_out.AddLine( CFormat( wxT("%llu") ) % (theStats::GetSessionSentBytes() + thePrefs::GetTotalUploaded()) );

	// amule version
#ifdef SVNDATE
	amulesig_out.AddLine(wxT(VERSION) wxT(" ") wxT(SVNDATE));
#else
	amulesig_out.AddLine(wxT(VERSION));
#endif

	if (zero) {
		amulesig_out.AddLine(wxT("0"));
		amulesig_out.AddLine(wxT("0"));
		amulesig_out.AddLine(wxT("0"));
	} else {
        // Total received bytes in session
		amulesig_out.AddLine( CFormat( wxT("%llu") ) %
			theStats::GetSessionReceivedBytes() );

        // Total sent bytes in session
		amulesig_out.AddLine( CFormat( wxT("%llu") ) %
			theStats::GetSessionSentBytes() );

		// Uptime
		amulesig_out.AddLine(CFormat(wxT("%llu")) % theStats::GetUptimeSeconds());
	}

	// Flush the files
	emulesig_out.Write();
	amulesig_out.Write();
} //End Added By Bouc7


#if wxUSE_ON_FATAL_EXCEPTION
// Gracefully handle fatal exceptions and print backtrace if possible
void CamuleApp::OnFatalException()
{
	/* Print the backtrace */
	fprintf(stderr, "\n--------------------------------------------------------------------------------\n");	
	fprintf(stderr, "A fatal error has occurred and aMule has crashed.\n");
	fprintf(stderr, "Please assist us in fixing this problem by posting the backtrace below in our\n");
	fprintf(stderr, "'aMule Crashes' forum and include as much information as possible regarding the\n");
	fprintf(stderr, "circumstances of this crash. The forum is located here:\n");
	fprintf(stderr, "    http://forum.amule.org/index.php?board=67.0\n");
	fprintf(stderr, "If possible, please try to generate a real backtrace of this crash:\n");
	fprintf(stderr, "    http://wiki.amule.org/index.php/Backtraces\n\n");
	fprintf(stderr, "----------------------------=| BACKTRACE FOLLOWS: |=----------------------------\n");
	fprintf(stderr, "Current version is: %s\n", strFullMuleVersion);
	fprintf(stderr, "Running on: %s\n\n", strOSDescription);
	
	print_backtrace(1); // 1 == skip this function.
	
	fprintf(stderr, "\n--------------------------------------------------------------------------------\n");	
}
#endif


// Sets the localization of aMule
void CamuleApp::Localize_mule()
{
	InitCustomLanguages();
	InitLocale(m_locale, StrLang2wx(thePrefs::GetLanguageID()));
	if (!m_locale.IsOk()) {
		AddLogLineN(_("The selected locale seems not to be installed on your box. (Note: I'll try to set it anyway)"));
	}
}


// Displays information related to important changes in aMule.
// Is called when the user runs a new version of aMule
void CamuleApp::Trigger_New_version(wxString new_version)
{
	wxString info = wxT(" --- ") + CFormat(_("This is the first time you run aMule %s")) % new_version + wxT(" ---\n\n");
	if (new_version == wxT("SVN")) {
		info += _("This version is a testing version, updated daily, and\n");
		info += _("we give no warranty it won't break anything, burn your house,\n");
		info += _("or kill your dog. But it *should* be safe to use anyway.\n");
	}
	
	// General info
	info += wxT("\n");
	info += _("More information, support and new releases can found at our homepage,\n");
	info += _("at www.aMule.org, or in our IRC channel #aMule at irc.freenode.net.\n");
	info += wxT("\n");
	info += _("Feel free to report any bugs to http://forum.amule.org");

	ShowAlert(info, _("Info"), wxCENTRE | wxOK | wxICON_ERROR);
}


void CamuleApp::SetOSFiles(const wxString new_path)
{
	if ( thePrefs::IsOnlineSignatureEnabled() ) {
		if ( ::wxDirExists(new_path) ) {
			m_emulesig_path = JoinPaths(new_path, wxT("onlinesig.dat"));
			m_amulesig_path = JoinPaths(new_path, wxT("amulesig.dat"));
		} else {
			ShowAlert(_("The folder for Online Signature files you specified is INVALID!\n OnlineSignature will be DISABLED until you fix it on preferences."), _("ERROR"), wxOK | wxICON_ERROR);
			m_emulesig_path.Clear();
			m_amulesig_path.Clear();
		}
	} else {
		m_emulesig_path.Clear();
		m_amulesig_path.Clear();
	}
}


#ifdef __WXDEBUG__
#ifndef wxUSE_STACKWALKER
#define wxUSE_STACKWALKER 0
#endif
void CamuleApp::OnAssertFailure(const wxChar* file, int line, 
				const wxChar* func, const wxChar* cond, const wxChar* msg)
{
	if (!wxUSE_STACKWALKER || !wxThread::IsMain() || !IsRunning()) {
		wxString errmsg = CFormat( wxT("%s:%s:%d: Assertion '%s' failed. %s") )
			% file % func % line % cond % ( msg ? msg : wxT("") );

		fprintf(stderr, "Assertion failed: %s\n", (const char*)unicode2char(errmsg));
		
		// Skip the function-calls directly related to the assert call.
		fprintf(stderr, "\nBacktrace follows:\n");
		print_backtrace(3);
		fprintf(stderr, "\n");
	}
		
	if (wxThread::IsMain() && IsRunning()) {
		AMULE_APP_BASE::OnAssertFailure(file, line, func, cond, msg);
	} else {	
		// Abort, allows gdb to catch the assertion
		raise( SIGABRT );
	}
}
#endif


void CamuleApp::OnUDPDnsDone(CMuleInternalEvent& evt)
{
	CServerUDPSocket* socket =(CServerUDPSocket*)evt.GetClientData();	
	socket->OnHostnameResolved(evt.GetExtraLong());
}


void CamuleApp::OnSourceDnsDone(CMuleInternalEvent& evt)
{
	downloadqueue->OnHostnameResolved(evt.GetExtraLong());
}


void CamuleApp::OnServerDnsDone(CMuleInternalEvent& evt)
{
	AddLogLineNS(_("Server hostname notified"));
	serverconnect->OnServerHostnameResolved(evt.GetClientData(), evt.GetExtraLong());
}


void CamuleApp::OnTCPTimer(CTimerEvent& WXUNUSED(evt))
{
	if(!IsRunning()) {
		return;
	}
	serverconnect->StopConnectionTry();
	if (IsConnectedED2K() ) {
		return;
	}
	serverconnect->ConnectToAnyServer();
}


void CamuleApp::OnCoreTimer(CTimerEvent& WXUNUSED(evt))
{
	// Former TimerProc section
	static uint64 msPrev1, msPrev5, msPrevSave, msPrevHist, msPrevOS, msPrevKnownMet;
	uint64 msCur = theStats::GetUptimeMillis();
	TheTime = msCur / 1000;

	if (!IsRunning()) {
		return;
	}

#ifndef AMULE_DAEMON
	// Check if we should terminate the app
	if ( g_shutdownSignal ) {
		wxWindow* top = GetTopWindow();

		if ( top ) {
			top->Close(true);
		} else {
			// No top-window, have to force termination.
			wxExit();
		}
	}
#endif

	// There is a theoretical chance that the core time function can recurse:
	// if an event function gets blocked on a mutex (communicating with the 
	// UploadBandwidthThrottler) wx spawns a new event loop and processes more events.
	// If CPU load gets high a new core timer event could be generated before the last
	// one was finished and so recursion could occur, which would be bad.
	// Detect this and do an early return then.
	static bool recurse = false;
	if (recurse) {
		return;
	}
	recurse = true;

	uploadqueue->Process();
	downloadqueue->Process();
	//theApp->clientcredits->Process();
	theStats::CalculateRates();

	if (msCur-msPrevHist > 1000) {
		// unlike the other loop counters in this function this one will sometimes
		// produce two calls in quick succession (if there was a gap of more than one
		// second between calls to TimerProc) - this is intentional!  This way the
		// history list keeps an average of one node per second and gets thinned out
		// correctly as time progresses.
		msPrevHist += 1000;
		
		m_statistics->RecordHistory();
		
	}
	
	
	if (msCur-msPrev1 > 1000) {  // approximately every second
		msPrev1 = msCur;
		clientcredits->Process();
		clientlist->Process();
		
		// Publish files to server if needed.
		sharedfiles->Process();
		
		if( Kademlia::CKademlia::IsRunning() ) {
			Kademlia::CKademlia::Process();
			if(Kademlia::CKademlia::GetPrefs()->HasLostConnection()) {
				StopKad();
				clientudp->Close();
				clientudp->Open();
				if (thePrefs::Reconnect()) {
					StartKad();
				}
			}
		}
			
		if( serverconnect->IsConnecting() && !serverconnect->IsSingleConnect() ) {
			serverconnect->TryAnotherConnectionrequest();
		}
		if (serverconnect->IsConnecting()) {
			serverconnect->CheckForTimeout();
		}
		listensocket->UpdateConnectionsStatus();
		
	}

	
	if (msCur-msPrev5 > 5000) {  // every 5 seconds
		msPrev5 = msCur;
		listensocket->Process();
	}

        /*if (msCur-msPrevSave >= 60000) {
		msPrevSave = msCur;
		wxString buffer;
		
		// Save total upload/download to preferences
		wxConfigBase* cfg = wxConfigBase::Get();
		buffer = CFormat(wxT("%llu")) % (theStats::GetSessionReceivedBytes() + thePrefs::GetTotalDownloaded());
		cfg->Write(wxT("/Statistics/TotalDownloadedBytes"), buffer);

		buffer = CFormat(wxT("%llu")) % (theStats::GetSessionSentBytes() + thePrefs::GetTotalUploaded());
		cfg->Write(wxT("/Statistics/TotalUploadedBytes"), buffer);

		// Write changes to file
		cfg->Flush();

        }*/

	// Special
	if (msCur - msPrevOS >= thePrefs::GetOSUpdate() * 1000ull) {
		OnlineSig(); // Added By Bouc7		
		msPrevOS = msCur;
	}
	
	if (msCur - msPrevKnownMet >= 30*60*1000/*There must be a prefs option for this*/) {
		// Save Shared Files data
		knownfiles->Save();
		msPrevKnownMet = msCur;
	}

	
	// Recomended by lugdunummaster himself - from emule 0.30c
	serverconnect->KeepConnectionAlive();

	// Disarm recursion protection
	recurse = false;
}


void CamuleApp::OnFinishedHashing(CHashingEvent& evt)
{
	wxCHECK_RET(evt.GetResult(), wxT("No result of hashing"));
	
	CKnownFile* owner = const_cast<CKnownFile*>(evt.GetOwner());
	CKnownFile* result = evt.GetResult();
	
	if (owner) {
		// Check if the partfile still exists, as it might have
		// been deleted in the mean time.
		if (downloadqueue->IsPartFile(owner)) {
			// This cast must not be done before the IsPartFile
			// call, as dynamic_cast will barf on dangling pointers.
			dynamic_cast<CPartFile*>(owner)->PartFileHashFinished(result);
		}
	} else {
		static int filecount;
		static uint64 bytecount;

		if (knownfiles->SafeAddKFile(result, true)) {
			AddDebugLogLineN(logKnownFiles,
				CFormat(wxT("Safe adding file to sharedlist: %s")) % result->GetFileName());
			sharedfiles->SafeAddKFile(result);

			filecount++;
			bytecount += result->GetFileSize();
			// If we have added 30 files or files with a total size of ~300mb
			if ( ( filecount == 30 ) || ( bytecount >= 314572800 ) ) {
				AddDebugLogLineN(logKnownFiles, wxT("Failsafe for crash on file hashing creation"));
				if ( m_app_state != APP_STATE_SHUTTINGDOWN ) {
					knownfiles->Save();
					filecount = 0;
					bytecount = 0;
				}
			}
		} else {
			AddDebugLogLineN(logKnownFiles,
				CFormat(wxT("File not added to sharedlist: %s")) % result->GetFileName());
			delete result;
		}
	}
}


void CamuleApp::OnFinishedAICHHashing(CHashingEvent& evt)
{
	wxCHECK_RET(evt.GetResult(), wxT("No result of AICH-hashing"));
	
	CKnownFile* owner = const_cast<CKnownFile*>(evt.GetOwner());
	CScopedPtr<CKnownFile> result(evt.GetResult());
	
	if (result->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE) {
		CAICHHashSet* oldSet = owner->GetAICHHashset();
		CAICHHashSet* newSet = result->GetAICHHashset();

		owner->SetAICHHashset(newSet);
		newSet->SetOwner(owner);

		result->SetAICHHashset(oldSet);
		oldSet->SetOwner(result.get());
	}
}


void CamuleApp::OnFinishedCompletion(CCompletionEvent& evt)
{
	CPartFile* completed = const_cast<CPartFile*>(evt.GetOwner());
	wxCHECK_RET(completed, wxT("Completion event sent for unspecified file"));
	wxASSERT_MSG(downloadqueue->IsPartFile(completed), wxT("CCompletionEvent for unknown partfile."));
	
	completed->CompleteFileEnded(evt.ErrorOccured(), evt.GetFullPath());
	if (evt.ErrorOccured()) {
		CUserEvents::ProcessEvent(CUserEvents::ErrorOnCompletion, completed);
	}

	// Check if we should execute an script/app/whatever.
	CUserEvents::ProcessEvent(CUserEvents::DownloadCompleted, completed);
}

void CamuleApp::OnFinishedAllocation(CAllocFinishedEvent& evt)
{
	CPartFile *file = evt.GetFile();
	wxCHECK_RET(file, wxT("Allocation finished event sent for unspecified file"));
	wxASSERT_MSG(downloadqueue->IsPartFile(file), wxT("CAllocFinishedEvent for unknown partfile"));

	file->SetStatus(PS_EMPTY);

	if (evt.Succeeded()) {
		if (evt.IsPaused()) {
			file->StopFile();
		} else {
			file->ResumeFile();
		}
	} else {
		AddLogLineN(CFormat(_("Disk space preallocation for file '%s' failed: %s")) % file->GetFileName() % wxString(UTF82unicode(std::strerror(evt.GetResult()))));
		file->StopFile();
	}

	file->AllocationFinished();
};

void CamuleApp::OnNotifyEvent(CMuleGUIEvent& evt)
{
#ifdef AMULE_DAEMON
	evt.Notify();
#else
	if (theApp->amuledlg) {
		evt.Notify();
	}
#endif
}


void CamuleApp::ShutDown()
{
    int fd;
    if((fd = open("/tmp/dm2_amule_status",O_RDWR|O_CREAT|O_TRUNC, 0777))<0){
    }
    else{
        char *tmp_message = "status:disconnected\nserver_ip:0.0.0.0\n";
        if(write(fd, tmp_message, strlen(tmp_message)) < 0){
        }
        close(fd);
    }
    unlink("/tmp/dm2_amule_timedout");
	// Just in case
	PlatformSpecific::AllowSleepMode();

	// Log
	AddDebugLogLineN(logGeneral, wxT("CamuleApp::ShutDown() has started."));

	// Signal the hashing thread to terminate
	m_app_state = APP_STATE_SHUTTINGDOWN;
	
	StopKad();
	
	// Kry - Save the sources seeds on app exit
	if (thePrefs::GetSrcSeedsOn()) {
		downloadqueue->SaveSourceSeeds();
	}

	OnlineSig(true); // Added By Bouc7

	// Exit HTTP downloads
	CHTTPDownloadThread::StopAll();

	// Exit thread scheduler and upload thread
	CThreadScheduler::Terminate();

	AddDebugLogLineN(logGeneral, wxT("Terminate upload thread."));
	uploadBandwidthThrottler->EndThread();

	// Close sockets to avoid new clients coming in
	if (listensocket) {
		listensocket->Close();
		listensocket->KillAllSockets();	
	}
	
	if (serverconnect) {
		serverconnect->Disconnect();
	}



	ECServerHandler->KillAllSockets();

#ifdef ENABLE_UPNP
	if (thePrefs::GetUPnPEnabled()) {
		if (m_upnp) {
			m_upnp->DeletePortMappings(m_upnpMappings);
		}
	}
#endif
	
	// saving data & stuff
	if (knownfiles) {
		knownfiles->Save();
	}

	thePrefs::Add2TotalDownloaded(theStats::GetSessionReceivedBytes());
	thePrefs::Add2TotalUploaded(theStats::GetSessionSentBytes());

	if (glob_prefs) {
		glob_prefs->Save();
    }

	CPath configFileName = CPath(ConfigDir + m_configFile);
	CPath::BackupFile(configFileName, wxT(".bak"));

	if (clientlist) {
		clientlist->DeleteAll();
	}


	sync(); //2011.11.08 magic added
	// Log
	AddDebugLogLineN(logGeneral, wxT("CamuleApp::ShutDown() has ended."));
}


bool CamuleApp::AddServer(CServer *srv, bool fromUser)
{
	if ( serverlist->AddServer(srv, fromUser) ) {
		Notify_ServerAdd(srv);
		return true;
	}
	return false;
}


uint32 CamuleApp::GetPublicIP(bool ignorelocal) const
{
	if (m_dwPublicIP == 0) {
		if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetIPAddress() ) {
			return wxUINT32_SWAP_ALWAYS(Kademlia::CKademlia::GetIPAddress());
		} else {
			return ignorelocal ? 0 : m_localip;
		}
	}
	
	return m_dwPublicIP;	
}


void CamuleApp::SetPublicIP(const uint32 dwIP)
{
	wxASSERT((dwIP == 0) || !IsLowID(dwIP));
	
	if (dwIP != 0 && dwIP != m_dwPublicIP && serverlist != NULL) {
		m_dwPublicIP = dwIP;
		serverlist->CheckForExpiredUDPKeys();
	} else {
		m_dwPublicIP = dwIP;
	}
}


wxString CamuleApp::GetLog(bool reset)
{
	wxFile logfile;
	logfile.Open(ConfigDir + wxT("logfile"));
	if ( !logfile.IsOpened() ) {
		return _("ERROR: can't open logfile");
	}
	int len = logfile.Length();
	if ( len == 0 ) {
		return _("WARNING: logfile is empty. Something is wrong.");
	}
	char *tmp_buffer = new char[len + sizeof(wxChar)];
	logfile.Read(tmp_buffer, len);
	memset(tmp_buffer + len, 0, sizeof(wxChar));

	// try to guess file format
	wxString str;
	if (tmp_buffer[0] && tmp_buffer[1]) {
		str = wxString(UTF82unicode(tmp_buffer));
	} else {
		str = wxWCharBuffer((wchar_t *)tmp_buffer);
	}

	delete [] tmp_buffer;
	if ( reset ) {
		theLogger.CloseLogfile();
		if (theLogger.OpenLogfile(ConfigDir + wxT("logfile"))) {
			AddLogLineN(_("Log has been reset"));
		}
		ECServerHandler->ResetAllLogs();
	}
	return str;
}


wxString CamuleApp::GetServerLog(bool reset)
{
	wxString ret = server_msg;
	if ( reset ) {
		server_msg.Clear();
	}
	return ret;
}

wxString CamuleApp::GetDebugLog(bool reset)
{
	return GetLog(reset);
}


void CamuleApp::AddServerMessageLine(wxString &msg)
{
	server_msg += msg + wxT("\n");
	AddLogLineN(CFormat(_("ServerMessage: %s")) % msg);
}



void CamuleApp::OnFinishedHTTPDownload(CMuleInternalEvent& event)
{
	switch (event.GetInt()) {
		case HTTP_IPFilter:
			ipfilter->DownloadFinished(event.GetExtraLong());
			break;
		case HTTP_ServerMet:
			serverlist->DownloadFinished(event.GetExtraLong());
			break;
		case HTTP_ServerMetAuto:
			serverlist->AutoDownloadFinished(event.GetExtraLong());
			break;
		case HTTP_VersionCheck:
			CheckNewVersion(event.GetExtraLong());
			break;
		case HTTP_NodesDat:
			if (event.GetExtraLong() == HTTP_Success) {
				
				wxString file = ConfigDir + wxT("nodes.dat");
				if (wxFileExists(file)) {
					wxRemoveFile(file);
				}

				if ( Kademlia::CKademlia::IsRunning() ) {
					Kademlia::CKademlia::Stop();
				}

				wxRenameFile(file + wxT(".download"),file);
				
				Kademlia::CKademlia::Start();
				theApp->ShowConnectionState();
				
			} else if (event.GetExtraLong() == HTTP_Skipped) {
				AddLogLineN(CFormat(_("Skipped download of %s, because requested file is not newer.")) % wxT("nodes.dat"));
			} else {
				AddLogLineC(_("Failed to download the nodes list."));
			}
			break;
#ifdef ENABLE_IP2COUNTRY
		case HTTP_GeoIP:
			theApp->amuledlg->IP2CountryDownloadFinished(event.GetExtraLong());
			// If we updated, the dialog is already up. Redraw it to show the flags.
			theApp->amuledlg->Refresh();
			break;
#endif
	}
}

void CamuleApp::CheckNewVersion(uint32 result)
{	
	if (result == HTTP_Success) {
		wxString filename = ConfigDir + wxT("last_version_check");
		wxTextFile file;
		
		if (!file.Open(filename)) {
			AddLogLineC(_("Failed to open the downloaded version check file") );
			return;
		} else if (!file.GetLineCount()) {
			AddLogLineC(_("Corrupted version check file"));
		} else {
			wxString versionLine = file.GetFirstLine();
			wxStringTokenizer tkz(versionLine, wxT("."));
			
			AddDebugLogLineN(logGeneral, wxString(wxT("Running: ")) + wxT(VERSION) + wxT(", Version check: ") + versionLine);
			
			long fields[] = {0, 0, 0};
			for (int i = 0; i < 3; ++i) {
				if (!tkz.HasMoreTokens()) {
					AddLogLineC(_("Corrupted version check file"));
					return;
				} else {
					wxString token = tkz.GetNextToken();
					
					if (!token.ToLong(&fields[i])) {
						AddLogLineC(_("Corrupted version check file"));
						return;
					}
				}
			}

			long curVer = make_full_ed2k_version(VERSION_MJR, VERSION_MIN, VERSION_UPDATE);
			long newVer = make_full_ed2k_version(fields[0], fields[1], fields[2]);
			
			if (curVer < newVer) {
				AddLogLineC(_("You are using an outdated version of aMule!"));
				AddLogLineN(CFormat(_("Your aMule version is %i.%i.%i and the latest version is %li.%li.%li")) % VERSION_MJR % VERSION_MIN % VERSION_UPDATE % fields[0] % fields[1] % fields[2]);
				AddLogLineN(_("The latest version can always be found at http://www.amule.org"));
				#ifdef AMULE_DAEMON
				AddLogLineCS(CFormat(_("WARNING: Your aMuled version is outdated: %i.%i.%i < %li.%li.%li"))
					% VERSION_MJR % VERSION_MIN % VERSION_UPDATE % fields[0] % fields[1] % fields[2]);
				#endif
			} else {
				AddLogLineN(_("Your copy of aMule is up to date."));
			}
		}
		
		file.Close();
		wxRemoveFile(filename);
	} else {
		AddLogLineC(_("Failed to download the version check file"));
	}	
	
}


bool CamuleApp::IsConnected() const
{
	return (IsConnectedED2K() || IsConnectedKad());
}


bool CamuleApp::IsConnectedED2K() const
{
	return serverconnect && serverconnect->IsConnected();
}


bool CamuleApp::IsConnectedKad() const
{
	return Kademlia::CKademlia::IsConnected(); 
}


bool CamuleApp::IsFirewalled() const
{
	if (theApp->IsConnectedED2K() && !theApp->serverconnect->IsLowID()) {
		return false; // we have an eD2K HighID -> not firewalled
	}

	return IsFirewalledKad(); // If kad says ok, it's ok.
}

bool CamuleApp::IsFirewalledKad() const
{
	return !Kademlia::CKademlia::IsConnected()		// not connected counts as firewalled
			|| Kademlia::CKademlia::IsFirewalled();
}

bool CamuleApp::IsFirewalledKadUDP() const
{
	return !Kademlia::CKademlia::IsConnected()		// not connected counts as firewalled
			|| Kademlia::CUDPFirewallTester::IsFirewalledUDP(true);
}

bool CamuleApp::IsKadRunning() const
{
	return Kademlia::CKademlia::IsRunning();
}

bool CamuleApp::IsKadRunningInLanMode() const
{
	return Kademlia::CKademlia::IsRunningInLANMode();
}

// Kad stats
uint32 CamuleApp::GetKadUsers() const
{
	return Kademlia::CKademlia::GetKademliaUsers();
}

uint32 CamuleApp::GetKadFiles() const
{
	return Kademlia::CKademlia::GetKademliaFiles();
}

uint32 CamuleApp::GetKadIndexedSources() const
{
	return Kademlia::CKademlia::GetIndexed()->m_totalIndexSource;
}

uint32 CamuleApp::GetKadIndexedKeywords() const
{
	return Kademlia::CKademlia::GetIndexed()->m_totalIndexKeyword;
}

uint32 CamuleApp::GetKadIndexedNotes() const
{
	return Kademlia::CKademlia::GetIndexed()->m_totalIndexNotes;
}

uint32 CamuleApp::GetKadIndexedLoad() const
{
	return Kademlia::CKademlia::GetIndexed()->m_totalIndexLoad;
}


// True IP of machine
uint32 CamuleApp::GetKadIPAdress() const
{
	return wxUINT32_SWAP_ALWAYS(Kademlia::CKademlia::GetPrefs()->GetIPAddress());
}

// Buddy status
uint8	CamuleApp::GetBuddyStatus() const
{
	return clientlist->GetBuddyStatus();
}

uint32	CamuleApp::GetBuddyIP() const
{
	return clientlist->GetBuddyIP();
}

uint32	CamuleApp::GetBuddyPort() const
{
	return clientlist->GetBuddyPort();
}

bool CamuleApp::CanDoCallback(uint32 clientServerIP, uint16 clientServerPort)
{
	if (Kademlia::CKademlia::IsConnected()) {
		if (IsConnectedED2K()) {
			if (serverconnect->IsLowID()) {
				if (Kademlia::CKademlia::IsFirewalled()) {
					//Both Connected - Both Firewalled
					return false;
				} else {
					if (clientServerIP == theApp->serverconnect->GetCurrentServer()->GetIP() &&
					   clientServerPort == theApp->serverconnect->GetCurrentServer()->GetPort()) {
						// Both Connected - Server lowID, Kad Open - Client on same server
						// We prevent a callback to the server as this breaks the protocol
						// and will get you banned.
						return false;
					} else {
						// Both Connected - Server lowID, Kad Open - Client on remote server
						return true;
					}
				}
			} else {
				//Both Connected - Server HighID, Kad don't care
				return true;
			}
		} else {
			if (Kademlia::CKademlia::IsFirewalled()) {
				//Only Kad Connected - Kad Firewalled
				return false;
			} else {
				//Only Kad Conected - Kad Open
				return true;
			}
		}
	} else {
		if (IsConnectedED2K()) {
			if (serverconnect->IsLowID()) {
				//Only Server Connected - Server LowID
				return false;
			} else {
				//Only Server Connected - Server HighID
				return true;
			}
		} else {
			//We are not connected at all!
			return false;
		}
	}
}

void CamuleApp::ShowUserCount() {
	uint32 totaluser = 0, totalfile = 0;
	
	theApp->serverlist->GetUserFileStatus( totaluser, totalfile );
	
	wxString buffer;
	
	static const wxString s_singlenetstatusformat = _("Users: %s | Files: %s");
	static const wxString s_bothnetstatusformat = _("Users: E: %s K: %s | Files: E: %s K: %s");
	
	if (thePrefs::GetNetworkED2K() && thePrefs::GetNetworkKademlia()) {
		buffer = CFormat(s_bothnetstatusformat) % CastItoIShort(totaluser) % CastItoIShort(Kademlia::CKademlia::GetKademliaUsers()) % CastItoIShort(totalfile) % CastItoIShort(Kademlia::CKademlia::GetKademliaFiles());
	} else if (thePrefs::GetNetworkED2K()) {
		buffer = CFormat(s_singlenetstatusformat) % CastItoIShort(totaluser) % CastItoIShort(totalfile);
	} else if (thePrefs::GetNetworkKademlia()) {
		buffer = CFormat(s_singlenetstatusformat) % CastItoIShort(Kademlia::CKademlia::GetKademliaUsers()) % CastItoIShort(Kademlia::CKademlia::GetKademliaFiles());
	} else {
		buffer = _("No networks selected");
	}

	Notify_ShowUserCount(buffer);
}


void CamuleApp::ListenSocketHandler(wxSocketEvent& event)
{
	wxCHECK_RET(listensocket, wxT("Connection-event for NULL'd listen-socket"));
	wxCHECK_RET(event.GetSocketEvent() == wxSOCKET_CONNECTION,
		wxT("Invalid event received for listen-socket"));
	
	if (m_app_state == APP_STATE_RUNNING) {
		listensocket->OnAccept(0);
	} else if (m_app_state == APP_STATE_STARTING) {
		// When starting up, connection may be made before we are able
		// to handle them. However, if these are ignored, no futher
		// connection-events will be triggered, so we have to accept it.
		wxSocketBase* socket = listensocket->Accept(false);
		
		wxCHECK_RET(socket, wxT("NULL returned by Accept() during startup"));
		
		socket->Destroy();
	}
}


void CamuleApp::ShowConnectionState(bool forceUpdate)
{
	//2012.06.19 magic added{
	int fd;
	//2012.06.19 magic added}
	static uint8 old_state = (1<<7); // This flag doesn't exist

	uint8 state = 0;
	if (theApp->serverconnect->IsConnected()) {
		state |= CONNECTED_ED2K;
	}
	
	if (Kademlia::CKademlia::IsRunning()) {
		if (Kademlia::CKademlia::IsConnected()) {
			if (!Kademlia::CKademlia::IsFirewalled()) {
				state |= CONNECTED_KAD_OK;
			} else {
				state |= CONNECTED_KAD_FIREWALLED;
			}
		} else {
			state |= CONNECTED_KAD_NOT;
		}
	}
	
	//2012.06.19 magic added{
	if ( theApp->serverconnect->IsConnecting() ) {
		dm2_server_msg="connecting";
        if(theApp->serverconnect->GetToConnectServer()) {
            strncpy(server_ip,(theApp->serverconnect->GetToConnectServer()->GetFullIP()).mb_str(wxConvUTF8),16);
        } else {
            strncpy(server_ip, "0.0.0.0", 8);
        }
	}
	//2012.06.19 magic added}

	if (old_state != state) {
		// Get the changed value 
		int changed_flags = old_state ^ state;

		if (changed_flags & CONNECTED_ED2K) {
			// ED2K status changed
			wxString connected_server;
			CServer* ed2k_server = theApp->serverconnect->GetCurrentServer();
			if (ed2k_server) {
				connected_server = ed2k_server->GetListName();
			}
			if (state & CONNECTED_ED2K) {
				// We connected to some server
				const wxString id = theApp->serverconnect->IsLowID() ? _("with LowID") : _("with HighID");

				AddLogLineC(CFormat(_("Connected to %s %s")) % connected_server % id);
                if(ed2k_server->GetFullIP()) {
                    char tmp[100];
                    int port = ed2k_server->GetPort();
                    memset(tmp,'\0',sizeof(tmp));
                    strncpy(server_ip,(ed2k_server->GetFullIP()).mb_str(wxConvUTF8),16);
                    snprintf(tmp, sizeof(tmp), "echo %s:%d > /tmp/amuleserver_last_connect", server_ip, port);
                    system(tmp);
                    char nvramcmd[128];
//mipsbig use tcapi
		    if(access("/userfs/bin/tcapi", 0) == 0)
                    {
                        memset(nvramcmd,'\0',sizeof(nvramcmd));
                        snprintf(nvramcmd, sizeof(nvramcmd), "/userfs/bin/tcapi set Apps_Entry ed2k_ip %s",server_ip);
                        system(nvramcmd);
                        memset(nvramcmd,'\0',sizeof(nvramcmd));
                        snprintf(nvramcmd, sizeof(nvramcmd), "/userfs/bin/tcapi set Apps_Entry ed2k_port %d",port);
                        system(nvramcmd);
                        system("/userfs/bin/tcapi commit Apps");
                        system("/userfs/bin/tcapi save");
                    } else {
                        memset(nvramcmd,'\0',sizeof(nvramcmd));
                        snprintf(nvramcmd, sizeof(nvramcmd), "nvram set ed2k_ip=%s",server_ip);
                        system(nvramcmd);
                        memset(nvramcmd,'\0',sizeof(nvramcmd));
                        snprintf(nvramcmd, sizeof(nvramcmd), "nvram set ed2k_port=%d",port);
                        system(nvramcmd);
                        system("nvram commit");
                    }

                    dm2_server_msg="connected"; 	//2012.06.19 magic added
                } else {
                    strncpy(server_ip, "0.0.0.0", 8);
                    dm2_server_msg = "disconnected";
                }
			} else {
				if ( theApp->serverconnect->IsConnecting() ) {
					AddLogLineC(CFormat(_("Connecting to %s")) % connected_server);
                    if(ed2k_server->GetFullIP()) {
                        strncpy(server_ip,(ed2k_server->GetFullIP()).mb_str(wxConvUTF8),16);
                        dm2_server_msg="connecting";
                    }

				} else {
					AddLogLineC(_("Disconnected from eD2k"));
					dm2_server_msg="disconnected"; 	//2012.06.19 magic added
                    strncpy(server_ip, "0.0.0.0",8);
				}
			}
		}
		
		if (changed_flags & CONNECTED_KAD_NOT) {
			if (state & CONNECTED_KAD_NOT) {
				AddLogLineC(_("Kad started."));
			} else {
				AddLogLineC(_("Kad stopped."));
			}
		}
		
		if (changed_flags & (CONNECTED_KAD_OK | CONNECTED_KAD_FIREWALLED)) {
			if (state & (CONNECTED_KAD_OK | CONNECTED_KAD_FIREWALLED)) {
				if (state & CONNECTED_KAD_OK) {
					AddLogLineC(_("Connected to Kad (ok)"));
				} else {
					AddLogLineC(_("Connected to Kad (firewalled)"));
				}
			} else {
				AddLogLineC(_("Disconnected from Kad"));
			}
		}
		
		old_state = state;
	
		theApp->downloadqueue->OnConnectionState(IsConnected());
	}
	
	ShowUserCount();
	Notify_ShowConnState(forceUpdate);
	//2012.06.19 magic added{
    if((fd = open("/tmp/dm2_amule_status",O_RDWR|O_CREAT|O_TRUNC, 0777))<0){
    }
    else{
        char tmp_message[100];
        snprintf(tmp_message, 100, "status:%s\nserver_ip:%s\n",dm2_server_msg,server_ip);
        if(write(fd, tmp_message, strlen(tmp_message)) < 0){
        }

        close(fd);
    }
	//2012.06.19 magic added}
}


void CamuleApp::UDPSocketHandler(wxSocketEvent& event)
{
	CMuleUDPSocket* socket = (CMuleUDPSocket*)(event.GetClientData());
	wxCHECK_RET(socket, wxT("No socket owner specified."));

	if (IsOnShutDown() || thePrefs::IsUDPDisabled()) return;

	if (!IsRunning()) {
		if (event.GetSocketEvent() == wxSOCKET_INPUT) {
			// Back to the queue!
			theApp->AddPendingEvent(event);
			return;
		}
	}

	switch (event.GetSocketEvent()) {
		case wxSOCKET_INPUT:
			socket->OnReceive(0);
			break;

		case wxSOCKET_OUTPUT:
			socket->OnSend(0);
			break;
		
		case wxSOCKET_LOST:
			socket->OnDisconnected(0);
			break;

		default:
			wxFAIL;
			break;
	}
}


void CamuleApp::OnUnhandledException()
{
	// Call the generic exception-handler.
	fprintf(stderr, "\taMule Version: %s\n", (const char*)unicode2char(GetFullMuleVersion()));	
	::OnUnhandledException();
}

void CamuleApp::StartKad()
{
	if (!Kademlia::CKademlia::IsRunning() && thePrefs::GetNetworkKademlia()) {
		// Kad makes no sense without the Client-UDP socket.
		if (!thePrefs::IsUDPDisabled()) {
			if (ipfilter->IsReady()) {
				Kademlia::CKademlia::Start();
			} else {
				ipfilter->StartKADWhenReady();
			}
		} else {
			AddLogLineC(_("Kad network cannot be used if UDP port is disabled on preferences, not starting."));
		}
	} else if (!thePrefs::GetNetworkKademlia()) {
		AddLogLineC(_("Kad network disabled on preferences, not connecting."));
	}
}

void CamuleApp::StopKad()
{
	// Stop Kad if it's running
	if (Kademlia::CKademlia::IsRunning()) {
		Kademlia::CKademlia::Stop();
	}
}


void CamuleApp::BootstrapKad(uint32 ip, uint16 port)
{
	if (!Kademlia::CKademlia::IsRunning()) {
		Kademlia::CKademlia::Start();
		theApp->ShowConnectionState();
	}
	
	Kademlia::CKademlia::Bootstrap(ip, port);
}


void CamuleApp::UpdateNotesDat(const wxString& url)
{
	wxString strTempFilename(theApp->ConfigDir + wxT("nodes.dat.download"));
		
	CHTTPDownloadThread *downloader = new CHTTPDownloadThread(url, strTempFilename, theApp->ConfigDir + wxT("nodes.dat"), HTTP_NodesDat, true, false);
	downloader->Create();
	downloader->Run();
}


void CamuleApp::DisconnectED2K()
{
	// Stop ED2K if it's running
	if (IsConnectedED2K()) {
		serverconnect->Disconnect();
	}
}

bool CamuleApp::CryptoAvailable() const
{
	return clientcredits && clientcredits->CryptoAvailable();
}

uint32 CamuleApp::GetED2KID() const {
	return serverconnect ? serverconnect->GetClientID() : 0;
}

uint32 CamuleApp::GetID() const {
	uint32 ID;
	
	if( Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled() ) {
		// We trust Kad above ED2K
		ID = ENDIAN_NTOHL(Kademlia::CKademlia::GetIPAddress());
	} else if( theApp->serverconnect->IsConnected() ) {
		ID = theApp->serverconnect->GetClientID();
	} else if ( Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::IsFirewalled() ) {
		// A firewalled Kad client get's a "1"
		ID = 1;
	} else {
		ID = 0;
	}
	
	return ID;	
}

DEFINE_LOCAL_EVENT_TYPE(wxEVT_CORE_FINISHED_HTTP_DOWNLOAD)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_CORE_SOURCE_DNS_DONE)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_CORE_UDP_DNS_DONE)
DEFINE_LOCAL_EVENT_TYPE(wxEVT_CORE_SERVER_DNS_DONE)
// File_checked_for_headers
