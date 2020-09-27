/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <map>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "AoDef.h"
#include "AoLogModule.h"
#include "AoTime.h"
#include "AoFileControl.h"
// 静态成员变量声明
CAoLogModule* CAoLogModule::ms_pInstance = NULL;
CAoLock CAoLogModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAoLogModule, CModule)
	ON_AO_MSG_FUN(EN_LOG_MODULE_LOG_INFO, &CAoLogModule::LogInfo)
	ON_AO_MSG_FUN(EN_LOG_MODULE_LOG_DEBUG, &CAoLogModule::LogDebug)
	ON_AO_MSG_FUN(EN_LOG_MODULE_LOG_ERR, &CAoLogModule::LogError)
	ON_AO_MSG_FUN(EN_LOG_MODULE_LOG_WARN, &CAoLogModule::LogWarn)
	ON_AO_MSG_FUN(EN_LOG_MODULE_LOG_DATA, &CAoLogModule::LogData)
END_AO_MSG_MAP()
CAoLogModule::CAoLogModule():CModule(EN_BASE_MODULE_ID_LOG, 1024*1024, "LogModule"){
	m_mapLogLevlText[EN_LOG_LEVEL_INFO] = "Info";
	m_mapLogLevlText[EN_LOG_LEVEL_DATA] = "Data";
	m_mapLogLevlText[EN_LOG_LEVEL_DEBUG] = "Debug";
	m_mapLogLevlText[EN_LOG_LEVEL_WARN] = "Warn";
	m_mapLogLevlText[EN_LOG_LEVEL_ERROR] = "Error";
}
CAoLogModule::~CAoLogModule(){
	for (SLI_MAP_IT it = m_mapSli.begin(); it != m_mapSli.end(); ++it){
		SAFE_DELETE(it->second);
	}
	m_mapSli.clear();
}
CAoLogModule* CAoLogModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CAoLogModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CAoLogModule::Release(){	SAFE_DELETE(ms_pInstance);}
void CAoLogModule::LogInfo(const MT_MSG* pMM){
	const char* pszModuleName = (const char*)pMM->ucData;
	const char* pszSubModuleName = pszModuleName + strlen(pszModuleName) + 1;
	const char* pszLogInfo = pszSubModuleName + strlen(pszSubModuleName) + 1;
	LogString(EN_LOG_LEVEL_INFO, pszModuleName, pszSubModuleName, pszLogInfo);
}
void CAoLogModule::LogDebug(const MT_MSG* pMM){
	const char* pszModuleName = (const char*)pMM->ucData;
	const char* pszSubModuleName = pszModuleName + strlen(pszModuleName) + 1;
	const char* pszLogInfo = pszSubModuleName + strlen(pszSubModuleName) + 1;
	LogString(EN_LOG_LEVEL_DEBUG, pszModuleName, pszSubModuleName, pszLogInfo);
}
void CAoLogModule::LogError(const MT_MSG* pMM){
	const char* pszModuleName = (const char*)pMM->ucData;
	const char* pszSubModuleName = pszModuleName + strlen(pszModuleName) + 1;
	const char* pszLogInfo = pszSubModuleName + strlen(pszSubModuleName) + 1;
	LogString(EN_LOG_LEVEL_ERROR, pszModuleName, pszSubModuleName, pszLogInfo);
}
void CAoLogModule::LogWarn(const MT_MSG* pMM){
	const char* pszModuleName = (const char*)pMM->ucData;
	const char* pszSubModuleName = pszModuleName + strlen(pszModuleName) + 1;
	const char* pszLogInfo = pszSubModuleName + strlen(pszSubModuleName) + 1;
	LogString(EN_LOG_LEVEL_WARN, pszModuleName, pszSubModuleName, pszLogInfo);
}
void CAoLogModule::LogData(const MT_MSG* ){}
void CAoLogModule::LogString(int nLogLevel, const char* pszModuleName, const char* pszSubModuleName, const char* pszLogInfo){
	STRUCT_LOG_INFO* pSLI = NULL;
	std::string strModuleName(pszModuleName);
	SLI_MAP_IT it = m_mapSli.find(strModuleName);
	if (m_mapSli.end() == it){m_mapSli[strModuleName] = pSLI = new STRUCT_LOG_INFO();}
	else{pSLI = it->second;}
	if (0 == (pSLI->nLogLevel & nLogLevel)){return;}
	CAoTime tm;tm.GetTime();
	if (!pSLI->m_bOpened){
		char szDir[EN_MAX_PATH_LEN] = {0};
		char szPath[EN_MAX_PATH_LEN] = {0};
		CAoFileControl::GetCurAppPath(szPath, EN_MAX_PATH_LEN);
		sprintf(szDir, "%s/log", szPath);
		CAoFileControl::CreateDirs(szDir);
		char szLogFile[EN_MAX_PATH_LEN] = {0};
		sprintf(szLogFile, "%s/%s%04u%02u%02u%02u%02u%02u%03u.log", szDir, strModuleName.c_str(), tm.m_nYear, tm.m_nMonth, tm.m_nDay, tm.m_nHour, tm.m_nMonth, tm.m_nSecond, tm.m_nMilliSecond);
		pSLI->m_bOpened = (EN_NOERROR == pSLI->af.Open(szLogFile, "wb"));
	}
	if (!pSLI->m_bOpened){return;}
	char szTitle[256] = { 0 };
	sprintf(szTitle, "[%04u-%02u-%02u %02u:%02u:%02u.%03u][%s][%s]", tm.m_nYear, tm.m_nMonth, tm.m_nDay, tm.m_nHour, tm.m_nMinute, tm.m_nSecond, tm.m_nMilliSecond, pszSubModuleName, m_mapLogLevlText[nLogLevel].c_str());
	pSLI->af.Write(szTitle, (ao_size_t)strlen(szTitle));
	pSLI->af.Write(pszLogInfo, (ao_size_t)strlen(pszLogInfo));
	pSLI->af.Write("\r\n", 2);
	pSLI->af.Flush();
}
