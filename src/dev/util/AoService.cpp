/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stddef.h>
#ifdef WIN32
#include <Windows.h>
#else
#include<unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "AoDef.h"
#include "AoFile.h"
#include "AoProcess.h"
#endif
#include <string>
#include "AoHelper.h"
#include "AoService.h"
#include "AoFileControl.h"

std::string g_strServiceName;
pfnService CAoService::m_pfnStop = NULL;
pfnService CAoService::m_pfnSuspend = NULL;
pfnService CAoService::m_pfnResumn = NULL;
#ifdef WIN32
SERVICE_STATUS g_ServiceStatus;
SERVICE_STATUS_HANDLE g_hServiceStatusHanlde = NULL;
void WINAPI ServiceHandler(DWORD Opcode){
	switch(Opcode) { 
	case SERVICE_CONTROL_PAUSE:
		if (NULL != CAoService::m_pfnSuspend){(*CAoService::m_pfnSuspend)();}
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);		
		break; 
	case SERVICE_CONTROL_CONTINUE:
		if (NULL != CAoService::m_pfnResumn){(*CAoService::m_pfnResumn)();}
		g_ServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
		break; 
	case SERVICE_CONTROL_STOP:
		if (NULL != CAoService::m_pfnStop){(*CAoService::m_pfnStop)();}
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		::SetServiceStatus(g_hServiceStatusHanlde, &g_ServiceStatus);
		return; 
	case SERVICE_CONTROL_INTERROGATE:
	default:break;
	}

	::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
}
void WINAPI ServiceMain(DWORD ,LPTSTR* ){
	memset(&g_ServiceStatus ,0 ,sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	g_hServiceStatusHanlde = ::RegisterServiceCtrlHandler(g_strServiceName.c_str(), ServiceHandler);
	if(NULL == g_hServiceStatusHanlde){return; }
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWaitHint = 0;
	::SetServiceStatus(g_hServiceStatusHanlde ,&g_ServiceStatus);
}
#else
#endif
CAoService::CAoService(){}
CAoService::~CAoService(){}
int CAoService::Install( const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc ){
    char szStandardFile[1024] = { 0 };
#ifdef WIN32
    CAoFileControl::ToStandardlPath(pszExe, szStandardFile, "\\");
	SC_HANDLE hScm = ::OpenSCManager(NULL ,SERVICES_ACTIVE_DATABASE ,SC_MANAGER_ALL_ACCESS);
	if(hScm == NULL){return GetLastError();}
    SC_HANDLE hService = ::CreateService(hScm, pszServiceName, pszDisplay, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_SEVERE, szStandardFile, NULL, NULL, NULL, NULL, NULL);
	int nErr = GetLastError();
	if(ERROR_SERVICE_EXISTS == nErr){
		hService = ::OpenService(hScm , pszServiceName, SERVICE_ALL_ACCESS);
		nErr = GetLastError();
	}
	if(NULL == hService){::CloseServiceHandle(hScm);return nErr;}
	ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE , SERVICE_NO_CHANGE , NULL, NULL, NULL, NULL, NULL, NULL, pszDisplay);
	SERVICE_DESCRIPTION sd;
	sd.lpDescription = const_cast<char*>(pszDesc);
	ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hScm);
#else
    CAoFileControl::ToStandardlPath(pszExe, szStandardFile, "/");
    CAoFile af;
    if(EN_NOERROR != af.Open("/etc/redhat-release", "rb")){
		if (EN_NOERROR != af.Open("/etc/lsb-release", "rb")){
			printf("打开文件失败/etc/redhat-release\r\n");
			return InstallCentos6xService(pszServiceName, szStandardFile, pszDisplay, pszDesc);
		}
    }
    af.Seek(0, EN_SEEK_END);
    ao_size_t nLen = af.Tell();
    if (nLen < 10){
        return InstallCentos6xService(pszServiceName, szStandardFile, pszDisplay, pszDesc);
    }
    af.Seek(0, EN_SEEK_SET);
    char* pszBuf = (char*)MALLOC(nLen + 1);
    pszBuf[nLen] = '\0';
    af.Read(pszBuf, nLen);
	CAoHelper::Toupper(pszBuf);
	if (strstr(pszBuf, "UBUNTU") != NULL){
		return InstallUbuntuService(pszServiceName, szStandardFile, pszDisplay, pszDesc);
	}else{
        for (ao_size_t n = 14; n < nLen; ++n){
            if (pszBuf[n] >= '0' && pszBuf[n] <= '9'){
                if (pszBuf[n] < '7'){
                    return InstallCentos6xService(pszServiceName, szStandardFile, pszDisplay, pszDesc);
                }else{
                    return InstallCentos7xService(pszServiceName, szStandardFile, pszDisplay, pszDesc);
                }
			}
		}
    }
	SAFE_FREE(pszBuf);
#endif
	return 0;
}
int CAoService::Start( const char* pszServiceName ){
#ifdef WIN32
	SC_HANDLE hScm = ::OpenSCManager(NULL ,SERVICES_ACTIVE_DATABASE ,SC_MANAGER_ALL_ACCESS);
	if(hScm == NULL){return GetLastError();	}
	SC_HANDLE hService = ::OpenService(hScm , pszServiceName, SERVICE_ALL_ACCESS);
	if(NULL == hService){return GetLastError();}
	if (FALSE == ::StartService(hService , 0, NULL)){
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hScm);
		return GetLastError();
	}
	return 0;
#else
	char szCmd[1024] = {0};
	sprintf(szCmd, "service %s start", pszServiceName);
	system(szCmd);
	return 0;
#endif
}
int CAoService::Stop(const char* pszServiceName){
#ifdef WIN32
    SC_HANDLE hScm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
    if (hScm == NULL){return GetLastError();}
    SC_HANDLE hService = ::OpenService(hScm, pszServiceName, SERVICE_ALL_ACCESS);
    if (NULL == hService){return GetLastError();}
	SERVICE_STATUS s;
	::ControlService(hService ,SERVICE_CONTROL_STOP ,&s);
	::CloseServiceHandle(hService);
	CloseServiceHandle(hScm);
    return 0;
#else
    char szCmd[1024] = { 0 };
    sprintf(szCmd, "service %s stop", pszServiceName);
    system(szCmd);
    return 0;
#endif
}
int CAoService::Uninstall(const char* pszServiceName){
#ifdef WIN32
	SC_HANDLE hScm = ::OpenSCManager(NULL ,SERVICES_ACTIVE_DATABASE ,SC_MANAGER_ALL_ACCESS);
	if(hScm == NULL){return GetLastError();}
	SC_HANDLE hService = ::OpenService(hScm , pszServiceName, SERVICE_ALL_ACCESS);	
	if(hService == NULL){
		CloseServiceHandle(hScm);
		return GetLastError();
	}
	SERVICE_STATUS s;
	::ControlService(hService ,SERVICE_CONTROL_STOP ,&s);
	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hScm);
	return 0;
#else
	char szCmd[1024] = {0};
	sprintf(szCmd, "service %s stop", pszServiceName);
	system(szCmd);
	sprintf(szCmd, "rm -f /etc/init.d/%s", pszServiceName);
	return system(szCmd);
#endif
}
void CAoService::OnDispatch( const char* pszServiceName, pfnService pfnStop, pfnService pfnSuspend, pfnService pfnResum ){
	m_pfnStop = pfnStop;
	m_pfnSuspend = pfnSuspend;
	m_pfnResumn = pfnResum;
	g_strServiceName = pszServiceName;
#ifdef WIN32
	SERVICE_TABLE_ENTRY DispatchTable[] ={{(LPSTR)pszServiceName ,ServiceMain},{NULL ,NULL}};
	::StartServiceCtrlDispatcher(DispatchTable);
#else
	//daemon(0, 1);
#endif
}
#ifdef LINUX
int CAoService::InstallCentos6xService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc){
    // 将服务批处理文件发送至服务端
    const char* pszFileFormat = "#!/bin/bash \n \
#chkconfig:2345  80  05\n\
. /etc/init.d/functions \n\
SNAME=%s\n\
PROG=\"%s\"\n\
start() { \n\
    if [ -f /var/lock/subsys/$SNAME ]\n\
       then\n\
           echo \"$SNAME is already started!\"  \n\
           exit 0;  \n\
    else  \n\
        action \"Starting %s ...\" $PROG -daemon\n\
        [ $? -eq 0 ] && touch /var/lock/subsys/$SNAME  \n\
        exit 0;  \n\
    fi\n\
}\n\
stop() {  \n\
 	 echo \"Stopping %s ...\"  \n\
     killproc $SNAME  \n\
     rm -rf /var/lock/subsys/$SNAME  \n\
}  \n\
case \"$1\" in  \n\
    start)  \n\
        start  \n\
        ;;  \n\
    stop)  \n\
        stop  \n\
        ;;  \n\
    reload|restart)  \n\
        stop  \n\
        start  \n\
        ;;  \n\
    status)  \n\
        status $SNAME  \n\
        ;;  \n\
    *)  \n\
        echo $\"Usage: $0 {start|stop|restart|status}\"  \n\
        exit 1  \n\
 esac";
    char* pszFile = (char*)MALLOC(1024 * 1024);
    sprintf(pszFile, pszFileFormat, pszServiceName, pszExe, pszDesc, pszDesc);
    char szFileName[256] = { 0 };
    sprintf(szFileName, "/etc/init.d/%s", pszServiceName);
    FILE* pFile = fopen(szFileName, "wb");
    if (NULL == pFile){ FREE(pszFile);    return 1;    }
    fwrite(pszFile, 1, strlen(pszFile), pFile);
    fclose(pFile);
    SAFE_FREE(pszFile);
    int nRet = chmod(szFileName, S_IRGRP | S_IWGRP | S_IXGRP);
    CAoProcess ap;
    sprintf(szFileName, "chkconfig --add %s", pszServiceName);
    int nRet2 = ap.RunCmd(szFileName);
    return (nRet == 0) ? nRet2 : nRet;
}
int CAoService::InstallCentos7xService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc){
   // 将服务批处理文件发送至服务端
    const char* pszFileFormat = "[Unit]\n\
Description=%s\n\
\n\
[Service]\n\
Type=simple\n\
ExecStart=%s\n\
ExecReload=/bin/kill -HUP $MAINPID\n\
ExecStop=/bin/kill $MAINPID\n\
PrivateTmp=true\n\
Restart=on-failure\n\
KillMode=process\n\
\n\
[Install]\n\
WantedBy=multi-user.target"; // 开机启动
    char* pszFile = (char*)MALLOC(1024 * 1024);
    sprintf(pszFile, pszFileFormat, pszDesc, pszExe);
    char szFileName[256] = { 0 };
    sprintf(szFileName, "/usr/lib/systemd/system/%s.service", pszServiceName);
    FILE* pFile = fopen(szFileName, "wb");
    if (NULL == pFile)    {        FREE(pszFile);        return 1;    }
    fwrite(pszFile, 1, strlen(pszFile), pFile);
    fclose(pFile);
    SAFE_FREE(pszFile);
    chmod(szFileName, S_IRGRP | S_IWGRP | S_IXGRP | S_IRUSR | S_IROTH | S_IXOTH); 
    CAoProcess ap2;
    sprintf(szFileName, "systemctl enable %s.service", pszServiceName);
    ap2.RunCmd(szFileName);
    CAoProcess ap;
    return ap.RunCmd("systemctl daemon-reload");
}
int CAoService::InstallUbuntuService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc){
	// 将服务批处理文件发送至服务端
	const char* pszFileFormat = "[Unit]\n\
Description=%s\n\
\n\
[Service]\n\
Type=simple\n\
ExecStart=%s\n\
ExecReload=/bin/kill -HUP $MAINPID\n\
ExecStop=/bin/kill -9 $MAINPID\n\
PrivateTmp=true\n\
Restart=on-failure\n\
KillMode=process\n\
\n\
[Install]\n\
WantedBy=multi-user.target"; // 开机启动
	char* pszFile = (char*)MALLOC(1024 * 1024);
	sprintf(pszFile, pszFileFormat, pszDesc, pszExe);
	char szFileName[256] = { 0 };
	sprintf(szFileName, "/lib/systemd/system/%s.service", pszServiceName);
	FILE* pFile = fopen(szFileName, "wb");
	if (NULL == pFile)	{		FREE(pszFile);		return 1;	}
	fwrite(pszFile, 1, strlen(pszFile), pFile);
	fclose(pFile);
	SAFE_FREE(pszFile);
	chmod(szFileName, S_IRGRP | S_IWGRP | S_IXGRP | S_IRUSR | S_IROTH | S_IXOTH);
	CAoProcess ap2;
	sprintf(szFileName, "systemctl enable %s.service", pszServiceName);
	ap2.RunCmd(szFileName);
	CAoProcess ap;
	return ap.RunCmd("systemctl daemon-reload");
}
#endif
