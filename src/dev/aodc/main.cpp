/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include "../util/AoDef.h"
#include "../util/AoService.h"
#include "../util/AoCfgModule.h"
#include "../util/AoLogModule.h"
#include "../util/CommuModule.h"
#include "../util/AoFileControl.h"
#include "../util/ModuleDispatcher.h"
#include "UpdateModule.h"
#include "AodsAuthModule.h"
#include "AodsActiveModule.h"
#include "AoKafkaModule.h"
void OnServiceStop();
#ifdef WIN32
#include  <dbghelp.h>
#pragma comment(lib,  "dbghelp.lib")
LONG WINAPI windmp(struct _EXCEPTION_POINTERS* ExceptionInfo){
    char szDir[EN_MAX_PATH_LEN] = { 0 };
    char szFileName[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    sprintf_s(szFileName, EN_MAX_PATH_LEN, "%s/aodc.dmp", szDir);
    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE){
        MINIDUMP_EXCEPTION_INFORMATION  ExInfo;
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
		if(strcmp(*(argv + 1) ,("-h")) == 0){// 提示帮助信息
			printf("-i filepath: Install Service.\r\n-u Uninstall Service.\r\n-v: Show Version.\r\n");
			return 0;
		}else if(strcmp(*(argv + 1) , ("-i")) == 0){// 安装服务并启动运行
			char szFile[1024] = {0};
#ifdef WIN32
			if (argv[0][1] == ':'){strcpy(szFile, argv[0]);} // 待盘符说明为绝对路径
			else{ // 相对路径
				char szDir[EN_MAX_PATH_LEN] = {0};CAoFileControl::GetCwd(szDir, EN_MAX_PATH_LEN);sprintf(szFile, "%s\\%s", szDir, argv[0]);
			}
#else
			if(szFile[0] == '/') {strcpy(szFile, argv[0]);}//绝对路径			
			else{ // 相对路径
				char szDir[EN_MAX_PATH_LEN] = {0};CAoFileControl::GetCwd(szDir, EN_MAX_PATH_LEN);sprintf(szFile, "%s/%s", szDir, argv[0]);
			}
#endif
			printf(szFile);
			if(0 == CAoService::Install((3 == argc) ? *(argv + 2) : "aodc", szFile, "Automatic Operation Deploy Center", "部署中心服务")){
				printf("服务aodc安装成功\r\n");return CAoService::Start("aodc");
			}
			printf("服务aodc安装失败\r\n");return 1;
		}
		else if(strcmp(*(argv + 1) ,("-u")) == 0){return  CAoService::Uninstall("aodc");}// 卸载服务并停止运行
		else if(strcmp(*(argv + 1), ("-v")) == 0){printf("aodc version: 0.0803.0001");return 0;}// 版本信息
	}
	else if(argc == 1){
		int n = (int)strlen(argv[0]) - 1;
		for (; n >= 0 && argv[0][n] != '\\' && argv[0][n] != '/' ; --n);
		char szWorkDir[EN_MAX_PATH_LEN] = {0};
		memcpy(szWorkDir, argv[0], n);
		CAoFileControl::SetCurAppPath(szWorkDir);
		CAoFileControl::SetCurModuleFileName(argv[0] + ((0 == n) ? 0 : (n + 1)));
	}else{
#ifdef WIN32
		char szWorkDir[EN_MAX_PATH_LEN] = {0};
		GetModuleFileName(NULL, szWorkDir, EN_MAX_PATH_LEN);
		int n = (int)strlen(szWorkDir) - 1;
		for (; n >= 0 && szWorkDir[n] != '\\' && szWorkDir[n] != '/' ; --n);
		szWorkDir[n] = 0;
		CAoFileControl::SetCurAppPath(szWorkDir);
		CAoFileControl::SetCurModuleFileName(argv[0] + ((0 == n) ? 0 : (n + 1)));
#else
#endif
	}
	// 模块集
	CModule* pModules[] = {CCommuModule::Instance(),CAoCfgModule::Instance(),CAoLogModule::Instance(),CAodsActiveModule::Instance(),	CAodsAuthModule::Instance(),CUpdateModule::Instance(), CAoKafkaModule::Instance(),NULL};
	for (int n = 0; NULL != pModules[n]; ++n){pModules[n]->Start();CModuleDispatcher::Instance()->AppendModule(pModules[n]);}	
	CAoCfgModule::Instance()->PostMsg(EN_AO_MSG_CFG_START);// 加载配置文件
	CAoService::OnDispatch("aods", &OnServiceStop, NULL, NULL);
	for (int n = 0; n < sizeof(pModules) / sizeof(pModules[0]); ++n){pModules[n]->Join();}
	return 0;
}
void OnServiceStop(){
	CModule* pModules[] = {CCommuModule::Instance(),CAoCfgModule::Instance(),CAoLogModule::Instance(),CAodsActiveModule::Instance(),        CAodsAuthModule::Instance(),CUpdateModule::Instance(), CAoKafkaModule::Instance(),NULL};
	for (int n = 0; NULL != pModules[n]; ++n){pModules[n]->PostMsg(EN_AO_MSG_EXIT);}
    for (int n = 0; NULL != pModules[n]; ++n){pModules[n]->Join();}
}