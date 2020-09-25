/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#elif defined LINUX
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#endif
#include "../util/AoService.h"
#include "../util/AoFileControl.h"
#include "../util/AoLogModule.h"
#include "../util/AoCfgModule.h"
#include "../util/CommuModule.h"
#include "UpdateModuleMgr.h"
#include "../util/ModuleDispatcher.h"
#include "AodsAuthModule.h"
#include "AodsDef.h"
void OnServiceStop();
#ifdef WIN32
#include <Windows.h>
#include  <dbghelp.h>
#pragma comment(lib,  "dbghelp.lib")
LONG WINAPI windmp(struct _EXCEPTION_POINTERS* ExceptionInfo){
    char szDir[EN_MAX_PATH_LEN] = { 0 };
    char szFileName[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    sprintf_s(szFileName, EN_MAX_PATH_LEN, "%s/aods.dmp", szDir);
    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE){
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = ::GetCurrentThreadId();
        ExInfo.ExceptionPointers = ExceptionInfo;
        ExInfo.ClientPointers = NULL;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &ExInfo, NULL, NULL);
        CloseHandle(hFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
int main(int argc, char* argv[]){
#ifdef WIN32
    SetUnhandledExceptionFilter(&windmp);
#endif
	if (argc > 1){
		if(strcmp(*(argv + 1) ,("-u")) == 0){return  CAoService::Uninstall("aods");}// 卸载服务并停止运行
		else if(strcmp(*(argv + 1), ("-v")) == 0){printf("aods version: 0.0000.0001\r\n");return 0;}// 版本信息
        else if (strcmp(*(argv + 1), ("-daemon")) == 0){
#ifdef LINUX
            int nRet = daemon(1, 1);
            if (nRet < 0){perror("error daemon.../n");return 1;}
            else if (nRet != 0){perror("error daemon 1.../n");return 1;}
            int n = (int)strlen(argv[0]) - 1;
            for (; n >= 0 && argv[0][n] != '\\' && argv[0][n] != '/'; --n);
            char szWorkDir[EN_MAX_PATH_LEN] = { 0 };
            memcpy(szWorkDir, argv[0], n);
            if (0 == n || strcmp(szWorkDir, "./") == 0){getcwd(szWorkDir, EN_MAX_PATH_LEN);}
            CAoFileControl::SetCurAppPath(szWorkDir);
			CAoFileControl::SetCurModuleFileName(argv[0] + ((0 == n) ? 0 : (n + 1)));
#endif
        }
	}
	else if(argc == 1){
		int n = (int)strlen(argv[0]) - 1;
		for (; n >= 0 && argv[0][n] != '\\' && argv[0][n] != '/' ; --n);
		char szWorkDir[EN_MAX_PATH_LEN] = {0};
		memcpy(szWorkDir, argv[0], n);
        if (0 == n || strcmp(szWorkDir, "./") == 0){getcwd(szWorkDir, EN_MAX_PATH_LEN);}
		CAoFileControl::SetCurAppPath(szWorkDir);
		CAoFileControl::SetCurModuleFileName(argv[0] + ((0 == n) ? 0 : (n + 1)));
	}else{
        char szWorkDir[EN_MAX_PATH_LEN] = { 0 };
#ifdef WIN32		
		GetModuleFileName(NULL, szWorkDir, EN_MAX_PATH_LEN);
		int n = (int)strlen(szWorkDir) - 1;		
		for (; n >= 0 && szWorkDir[n] != '\\' && szWorkDir[n] != '/' ; --n);
		szWorkDir[n] = 0;
		CAoFileControl::SetCurAppPath(szWorkDir);
		CAoFileControl::SetCurModuleFileName(argv[0] + ((0 == n) ? 0 : (n + 1)));
#else
        getcwd(szWorkDir, EN_MAX_PATH_LEN);
		CAoFileControl::SetCurAppPath(szWorkDir);
		CAoFileControl::SetCurModuleFileName("aods");
#endif
	}
    // 修改aods服务的工作目录为执行文件所在目录
    char szWorkDir[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szWorkDir, EN_MAX_PATH_LEN);
    CAoFileControl::SetCwd(szWorkDir);	
	CModule* pModules[] = {CCommuModule::Instance(), CAoCfgModule::Instance(), CAoLogModule::Instance(), CAodsAuthModule::Instance(), CUpdateModuleMgr::Instance(), NULL};
	for (int n = 0; NULL != pModules[n]; ++n){
		pModules[n]->Start();
		CModuleDispatcher::Instance()->AppendModule(pModules[n]);
	}
	CAoCfgModule::Instance()->PostMsg(EN_AO_MSG_CFG_START);
	CAoService::OnDispatch("aods", &OnServiceStop, NULL, NULL);
	for (int n = 0; NULL != pModules[n]; ++n){pModules[n]->Join();}
	return 0;
}
void OnServiceStop(){CModuleDispatcher::Instance()->Dispatch(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT, NULL, 0, NULL, NULL, 0);}
