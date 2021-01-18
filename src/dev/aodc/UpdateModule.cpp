/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include <string.h>
#include "AodcDef.h"
#include "UpdateModule.h"
#include "../util/AoDef.h"
#include "../util/CJson.h"
#include "../util/AoMemCache.h"
CUpdateModule* CUpdateModule::ms_pInstance = NULL;
CAoLock CUpdateModule::ms_Lock;
BEGIN_AO_MSG_MAP(CUpdateModule, CModule)
	ON_AO_MSG_FUN(EN_AO_MSG_LOAD_CFG, &CUpdateModule::OnLoadCfg)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CUpdateModule::OnRecvChannelData)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, &CUpdateModule::OnSendChannelDataFail)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_FREED, &CUpdateModule::OnChannelFreed)
	ON_AO_MSG_FUN(EN_AO_MSG_AUTH_SUC, &CUpdateModule::OnAuthSuc)
	ON_AO_MSG_FUN(EN_AO_MSG_RCVFKAFKA, &CUpdateModule::OnRecvFromKafka)
END_AO_MSG_MAP()
CUpdateModule::CUpdateModule(): CModule(EN_MODULE_ID_UPDATE, 1024 * 1024, "UpdateModule"){
	m_pszAodsChannelName = "AodsServer";
	m_unAodsChannelNameLen = (unsigned int)strlen(m_pszAodsChannelName) + 1;
	m_pAMC = new CAoMemCache(1024, 1024);
}
CUpdateModule::~CUpdateModule(){SAFE_DELETE(m_pAMC);}
CUpdateModule* CUpdateModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CUpdateModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CUpdateModule::Release(){SAFE_DELETE(ms_pInstance);}
bool CUpdateModule::Start(){
    if (CModule::Start()){
        SetTimer(EN_TI_KEEPALIVE, EN_TT_KEEPALIVE, 0xFFFFFFFF, this);
        return true;
    }
    return false;
}
void CUpdateModule::OnLoadCfg(const MT_MSG*){}
void CUpdateModule::OnRecvChannelData(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	unsigned int unChannelNameLen = (unsigned int)strlen(pszChannelName) + 1;
	if (pMM->unDataLen < unChannelNameLen){return;}	
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
		OnRecvAodsData(pszChannelName, pMM->ucData + unChannelNameLen, pMM->unDataLen - unChannelNameLen);
	}
}
void CUpdateModule::OnSendChannelDataFail(const MT_MSG* pMM){
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
        CUO_MAP_IT cit = m_mapCuo.find(pszChannelName);
        if (m_mapCuo.end() == cit){return;}
        CUO_MAP_IT rit = m_mapRuo.find(cit->second.strResId);
        if (rit != m_mapRuo.end()){m_mapRuo.erase(rit);}
        m_mapCuo.erase(cit);
    }
}
void CUpdateModule::OnChannelFreed(const MT_MSG* pMM){
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
        CUO_MAP_IT cit = m_mapCuo.find(pszChannelName);
        if (m_mapCuo.end() == cit){return;}
        CUO_MAP_IT rit = m_mapRuo.find(cit->second.strResId);
        if (rit != m_mapRuo.end()){m_mapRuo.erase(rit);}
        m_mapCuo.erase(cit);
    }
}
void CUpdateModule::OnAuthSuc(const MT_MSG* pMM){
	if (pMM->unDataLen < EN_RES_ID_LEN || pMM->ucData[pMM->unDataLen - 1] != '\0'){return;}
	char szResId[16] = {0};
	strcpy(szResId, (char*)pMM->ucData);	
	UPDATE_OBJ& cuo = m_mapRuo[szResId]; // 资源号与更新对象之间的映射关系
    if (cuo.strChannelName.length() > 0){LogWarn("CUpdateModule::OnAuthSuc", "相同资源号%s使用两个不同的通道", szResId);}
	cuo.strChannelName = (const char*)(pMM->ucData + strlen(szResId) + 1);
	cuo.strResId = szResId;
	// 通讯通道和更新对象之间的映射关系
	UPDATE_OBJ& ruo = m_mapCuo[cuo.strChannelName];
	ruo.strChannelName = cuo.strChannelName;
	ruo.strResId = cuo.strResId;
}
void CUpdateModule::OnRecvFromKafka(const MT_MSG* pMM){// 处理kafka消息
	std::string strJson((char*)pMM->ucData, pMM->unDataLen);
	cJSON* pObj = cJSON_Parse(strJson.c_str());
	if (NULL == pObj) { return; }
	cJSON* pUpdate = cJSON_GetObjectItem(pObj, "update");
	if (NULL == pUpdate) { cJSON_Delete(pObj); return; }
	cJSON* pRid = cJSON_GetObjectItem(pUpdate, "rid");
	CUO_MAP_IT it = m_mapRuo.find(pRid->valuestring);
	if (m_mapRuo.end() == it){cJSON_Delete(pObj);return;}
	unsigned char ucData[1024] = { 0 };
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucData;
	pAAM->unMsgCode = EN_AODC_AODS_UMI;
	unsigned int unOffset = 0;
	cJSON* pProperty = cJSON_GetObjectItem(pUpdate, "project");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "module");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "version");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "hash");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "url");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "code");
	strcpy(pAAM->szMsg + unOffset, pProperty->valuestring);
	unOffset += (unsigned int)strlen(pAAM->szMsg + unOffset) + 1;
	pProperty = cJSON_GetObjectItem(pUpdate, "force");
	pAAM->szMsg[unOffset++] = (char)pProperty->valueint;
	m_pAMC->Reset();
	m_pAMC->AppendBuf((unsigned char*)it->second.strChannelName.c_str(), (unsigned int)it->second.strChannelName.length() + 1);
	m_pAMC->AppendBuf(ucData, EN_AODS_AODC_MSG_SIZE + unOffset);
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
}
void CUpdateModule::OnRecvAodsData(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen){
	if (unDataLen < EN_AODS_AODC_MSG_SIZE){return;}
	CUO_MAP_IT it = m_mapCuo.find(pszChannelName); // 未认证消息
	if (m_mapCuo.end() == it){return;}
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)pucData;
	switch(pAAM->unMsgCode){
	case EN_AODS_AODC_UMI: // 上报模块信息
        OnRecvUpdateModuleInfo(it->second.strResId.c_str(), (unsigned char*)pAAM->szMsg, unDataLen - EN_AODS_AODC_MSG_SIZE);
		break;
	case EN_AODS_AODC_UDR: // 更新结果上报
        OnRecvUpdateModuleResult(it->second.strResId.c_str(), (unsigned char*)pAAM->szMsg, unDataLen - EN_AODS_AODC_MSG_SIZE);
		break;
	case EN_AODS_AODC_RESTART:break; // 重启结果
	case EN_AODS_AODC_CONFIG:break; // 配置
	default:break;
	}
}
void CUpdateModule::OnRecvUpdateModuleInfo(const char* pszResId, const unsigned char* pucData, unsigned int unDataLen){
	int nNullNum = 0;
	for (unsigned int n = 0; n < unDataLen; ++n) {if (pucData[n] == '\0'){ ++nNullNum;}}// 检测有效输入
	if (nNullNum < 4) { return;	}
	const char* pszValue = (const char*)pucData;
	cJSON* pObj = cJSON_CreateObject();
	cJSON_AddItemToObject(pObj, "project", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "module", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "version", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "hash", cJSON_CreateString(pszValue));
	cJSON_AddItemToObject(pObj, "rid", cJSON_CreateString(pszResId));
	cJSON* pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "check_update", pObj);
	char* pszJsonBuf = cJSON_PrintUnformatted(pRoot);
	SendOutMsg(EN_MODULE_ID_KAFKA, EN_AO_MSG_SND2KAFKA, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf));
	cJSON_Delete(pRoot);
	cJSON_FreeBuf(pszJsonBuf);
}
void CUpdateModule::OnRecvUpdateModuleInfoResult(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen){	
	m_pAMC->Reset();// 向客户端发送消息
	m_pAMC->AppendBuf((unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
	unsigned char ucMsgHead[EN_AODS_AODC_MSG_SIZE] = {0};
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucMsgHead;
	pAAM->unMsgCode = EN_AODC_AODS_UMI;
	m_pAMC->AppendBuf(ucMsgHead, EN_AODS_AODC_MSG_SIZE);
	m_pAMC->AppendBuf(pucData, unDataLen);
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
}
void CUpdateModule::OnRecvUpdateModuleResult(const char* pszResId, const unsigned char* pucData, unsigned int unDataLen){
	int nNullNum = 0;
	for (unsigned int n = 1; n < unDataLen; ++n) { if (pucData[n] == '\0'){++nNullNum;}}// 检测有效输入
	if (nNullNum < 4) { return; }
	const char* pszValue = (const char*)pucData + 1;
	cJSON* pObj = cJSON_CreateObject();
	cJSON_AddItemToObject(pObj, "project", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "module", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "version", cJSON_CreateString(pszValue));
	pszValue += strlen(pszValue) + 1;
	cJSON_AddItemToObject(pObj, "hash", cJSON_CreateString(pszValue));
	cJSON_AddItemToObject(pObj, "rid", cJSON_CreateString(pszResId));
	cJSON_AddItemToObject(pObj, "result", cJSON_CreateNumber(pucData[0]));
	cJSON* pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "update_result", pObj);
	char* pszJsonBuf = cJSON_PrintUnformatted(pRoot);
	SendOutMsg(EN_MODULE_ID_KAFKA, EN_AO_MSG_SND2KAFKA, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf));
	cJSON_Delete(pRoot);
	cJSON_FreeBuf(pszJsonBuf);
}
void CUpdateModule::OnTimer(unsigned int unTimerID){
    if (EN_TI_KEEPALIVE != unTimerID){return;}
    unsigned char ucData[EN_AODS_AODC_MSG_SIZE] = { 0 };
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucData;
    pAAM->unMsgCode = EN_AODC_AODS_KEEPALIVE; // 保活消息
    for (CUO_MAP_IT it = m_mapCuo.begin(); it != m_mapCuo.end(); ++it){
        m_pAMC->Reset();
        m_pAMC->AppendBuf((unsigned char*)it->first.c_str(), (unsigned int)it->first.length() + 1);
        m_pAMC->AppendBuf(ucData, EN_AODS_AODC_MSG_SIZE);
        SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
    }
}
