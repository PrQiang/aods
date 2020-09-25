/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../util/CJson.h"
#include "AoCfgModule.h"
#include "../util/AoFile.h"
#include "../util/AoFileControl.h"
#include "../util/AoLock.h"
#include "../util/CJson.h"
#include "AoHash.h"
CAoCfgModule* CAoCfgModule::ms_pInstance;
CAoLock CAoCfgModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAoCfgModule, CModule)
	ON_AO_MSG_FUN(EN_AO_MSG_CFG_START, &CAoCfgModule::StartLoad)
	ON_AO_MSG_FUN(EN_AO_MSG_SAVE_CFG, &CAoCfgModule::SaveCfg)
END_AO_MSG_MAP()
CAoCfgModule::CAoCfgModule()
	:CModule(EN_MODULE_ID_CONFIG, 1024*1024, "AoCfgModule"){}
CAoCfgModule::~CAoCfgModule(){}
CAoCfgModule* CAoCfgModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CAoCfgModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CAoCfgModule::Release(){SAFE_DELETE(ms_pInstance);}
void CAoCfgModule::StartLoad(const MT_MSG* ){
	char* pszJsonBuf = GetConfigBuf();
	if (NULL == pszJsonBuf){return;}
	// 格式形如{"ModuleID":{"ModuleName":{"SubModuleId":{SubModulePara}}}}
	cJSON* pJson = cJSON_Parse(pszJsonBuf);
	SAFE_FREE(pszJsonBuf);
	if (NULL == pJson){
		LogErr("CAodsCfgModule::Load", "Failed to Parse config");		
		return;
	}
	// 遍历模块，并向各模块发送加载数据
	int nModuleId = 0;
	for (cJSON* pJsonModule = pJson->child; NULL != pJsonModule; pJsonModule = pJsonModule->next ){
		if (cJSON_Array != pJsonModule->type && cJSON_Object != pJsonModule->type){continue;}
        sscanf(pJsonModule->string, "%d", &nModuleId);
		pszJsonBuf = cJSON_PrintUnformatted(pJsonModule);
		SendOutMsg(nModuleId, (nModuleId == EN_MODULE_ID_BROADCAST) ? EN_AO_MSG_LOAD_CCF : EN_AO_MSG_LOAD_CFG, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf) + 1);
		cJSON_FreeBuf(pszJsonBuf);
	}
	cJSON_Delete(pJson);
}
void CAoCfgModule::SaveCfg(const MT_MSG* pMM){
	if (pMM->unDataLen < EN_AO_MSG_SAVE_CFG_SIZE){return;}
	AO_MSG_SAVE_CFG* pAMSC = (AO_MSG_SAVE_CFG*)pMM->ucData;
	cJSON* pJson = cJSON_Parse(pAMSC->szCfgJson);
	if (NULL == pJson){return;}
	char szModuleId[32] = {0};
	sprintf(szModuleId, "%u", pAMSC->unSrcModuleId);
	// 从配置文件中加载数据, 并将段发给指定的模块
	char* pszJsonBuf = GetConfigBuf();
	if (NULL == pszJsonBuf){
		cJSON* pNewCfg = cJSON_CreateObject();
		cJSON_AddItemToObject(pNewCfg, szModuleId, pJson);
		char* pszJsonCfg = cJSON_PrintUnformatted(pNewCfg);
		SaveConfigBuf(pszJsonCfg, (int)strlen(pszJsonCfg));
		cJSON_FreeBuf(pszJsonCfg);
		cJSON_Delete(pNewCfg);
		return;
	}
	cJSON* pOldJsonCfg = cJSON_Parse(pszJsonBuf);
	if (NULL == pOldJsonCfg){pOldJsonCfg = cJSON_CreateObject();}
	cJSON_DeleteItemFromObject(pOldJsonCfg, szModuleId); // 先移除旧项
	cJSON_AddItemToObject(pOldJsonCfg, szModuleId, pJson); // 添加新项
	char* pszJsonCfg = cJSON_PrintUnformatted(pOldJsonCfg); // 打印出json文本内容
	SaveConfigBuf(pszJsonCfg, (int)strlen(pszJsonCfg));
	cJSON_FreeBuf(pszJsonCfg);
	cJSON_Delete(pOldJsonCfg);
	SAFE_FREE(pszJsonBuf);
}
void CAoCfgModule::GetConfigFile(char* pszFile, int){
	char szDir[EN_MAX_PATH_LEN] = {0};
	CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
	char szFileName[EN_MAX_PATH_LEN] = {0};
	CAoFileControl::GetCurModuleFileName(szFileName, EN_MAX_PATH_LEN);	
	for (int n = (int)strlen(szFileName) - 1; n > 0; --n){// 去除文件后缀名
		if (szFileName[n] == '.'){szFileName[n] = '\0';	break;}
	}
	sprintf(pszFile, "%s/config/%scfg.json", szDir, szFileName);
}
void CAoCfgModule::SaveConfigBuf(char* pszBuf, int nLen){
	char szFileName[EN_MAX_PATH_LEN] = {0};
	GetConfigFile(szFileName, EN_MAX_PATH_LEN);
	CAoFile af;
	if (0 == af.Open(szFileName, "wb")) {af.Write(pszBuf, nLen);}
}
char* CAoCfgModule::GetConfigBuf() {
	// 从配置文件中加载数据, 并将段发给指定的模块
	char szFileName[EN_MAX_PATH_LEN] = { 0 };
	GetConfigFile(szFileName, EN_MAX_PATH_LEN);
	CAoFile af;
	if (0 != af.Open(szFileName, "rb")) { return NULL; }
	af.Seek(0, EN_SEEK_END);
	ao_size_t astLen = af.Tell();
	char* pszJsonBuf = (char*)malloc(astLen + 1);
	if (NULL == pszJsonBuf) { LogErr("CAodsCfgModule::Load", "Failed to load config(%s)", szFileName); return NULL; }
	af.Seek(0, EN_SEEK_SET);
	af.Read(pszJsonBuf, astLen);
	pszJsonBuf[astLen] = '\0';
	return pszJsonBuf;
}