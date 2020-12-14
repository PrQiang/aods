/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include <string.h>
#include "../util/AoDef.h"
#include "../util/CJson.h"
#include "../util/AoHash.h"
#include "../util/AoHelper.h"
#include "../util/AoEncrypt.h"
#include "../util/AoMemCache.h"
#include "../util/CJson.h"
#include "AodcDef.h"
#include "AodsActiveModule.h"
CAodsActiveModule* CAodsActiveModule::ms_pInstance = NULL;
CAoLock CAodsActiveModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAodsActiveModule, CModule)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CAodsActiveModule::OnRecvChannelData)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_BUILDED, &CAodsActiveModule::OnChannelBuilded)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, &CAodsActiveModule::OnSendChannelDataFail)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_FREED, &CAodsActiveModule::OnChannelFreed)
	ON_AO_MSG_FUN(EN_AO_MSG_RCVFKAFKA, &CAodsActiveModule::OnRecvFromKafka)
END_AO_MSG_MAP()
CAodsActiveModule::CAodsActiveModule():CModule(EN_MODULE_ID_ACTIVE, 1024 * 1024, "AodsActiveModule"){
	m_pAMC = new CAoMemCache(10240, 10240);
	m_pszAodsChannelName = "AodsServer";
	m_unAodsChannelNameLen = (unsigned int)strlen(m_pszAodsChannelName) + 1;
}
CAodsActiveModule::~CAodsActiveModule(){
	SAFE_DELETE(m_pAMC);
}
CAodsActiveModule* CAodsActiveModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CAodsActiveModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CAodsActiveModule::Release(){
	SAFE_DELETE(ms_pInstance);
}
void CAodsActiveModule::OnChannelBuilded(const MT_MSG* pMM){
	if ('\0' != pMM->ucData[pMM->unDataLen - 1]){
		return;
	}
	const char* pszChannelName = (char*)pMM->ucData;
	if (NULL == strstr(pszChannelName, m_pszAodsChannelName)){
		return;
	}
	// 设置映射关系
	unsigned int unLen = (unsigned int)strlen(pszChannelName) + 1;
	if (unLen + 1 > pMM->unDataLen){
		return;
	}
	const char* pszLocalAddr = pszChannelName + unLen;
	unLen += (unsigned int)strlen(pszLocalAddr) + 1;
	if (unLen + 1 > pMM->unDataLen){
		return;
	}	
	const char* pszPeerAddr = pszChannelName + unLen;// 通讯通道远端通讯地址
	m_mapCA[pszChannelName] = pszPeerAddr;
}
void CAodsActiveModule::OnRecvChannelData(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;// 通道名称
	unsigned int unChannleNameLen = (unsigned int)strlen(pszChannelName) + 1;	
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){// 处理来自Aods客户端的数据
		OnRecvAodsChannel(pszChannelName, pMM->ucData + unChannleNameLen, pMM->unDataLen - unChannleNameLen);
	}
}
void CAodsActiveModule::OnSendChannelDataFail(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
		SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
	}
}
void CAodsActiveModule::OnChannelFreed(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
		CU_MAP_IT cit = m_mapCu.find(pszChannelName);
		if (m_mapCu.end() == cit){return;}
		UAI_MAP_IT uit = m_mapUai.find(cit->second);
		m_mapCu.erase(cit);
		m_mapUai.erase(uit);		
		CA_MAP_IT itCA = m_mapCA.find(pszChannelName);// 移除通道名称与通讯地址之间的映射关系
		if (m_mapCA.end() != itCA){m_mapCA.erase(itCA);}
	}
}
void CAodsActiveModule::OnRecvAodsChannel(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen){
	if (unDataLen < EN_AODS_AODC_MSG_SIZE){// 发送激活结果消息
		OnActiveFailedResult(pszChannelName, EN_ACT_RLT_DATAINV);return;
	}
	AODS_AODC_MSG* pAAM =(AODS_AODC_MSG*)pucData;
	if (EN_AODS_AODC_ACTV != pAAM->unMsgCode) {return;}// 不是激活消息，忽略	
	AODS_AODC_ACTIVE* pAAA = (AODS_AODC_ACTIVE*)pAAM->szMsg;
	if (EN_AODS_AODC_MSG_SIZE + EN_AODS_AODC_ACTIVATE_SIZE != (int)unDataLen){ // 传入参数消息结构不正确
		OnActiveFailedResult(pszChannelName, EN_ACT_RLT_DATAINV); // 发送激活结果消息
		return;
	}
	CA_MAP_IT it = m_mapCA.find(pszChannelName);
	if (m_mapCA.end() == it){ // 发送激活结果消息
		OnActiveFailedResult(pszChannelName, EN_ACT_RLT_DBBUSY);return;
	}
	char szIpAddr[256] = {0};
	strcpy(szIpAddr, it->second.c_str());
	char* pszPort = strstr(szIpAddr, ":");
	if (NULL == pszPort){ // 发送激活结果消息
		OnActiveFailedResult(pszChannelName, EN_ACT_RLT_DBBUSY);return;
	}
	*pszPort = '\0';
	unsigned int unConIp = CAoHelper::Ipv4Address(szIpAddr);
	if (0 == unConIp){ // 发送激活结果消息
		OnActiveFailedResult(pszChannelName, EN_ACT_RLT_DBBUSY);return;
	}
	cJSON* pIpObj = cJSON_CreateArray();
	for (int n = 0; n < pAAA->ucIpNum; ++n){
		cJSON_AddItemToArray(pIpObj, cJSON_CreateString(CAoHelper::Ipv4Address(pAAA->unIpAddr[n])));
		if (unConIp == pAAA->unIpAddr[n]){unConIp = 0;}
	}
	if (0 != unConIp){cJSON_AddItemToArray(pIpObj, cJSON_CreateString(CAoHelper::Ipv4Address(unConIp)));}
	cJSON* pObj = cJSON_CreateObject();
	cJSON_AddItemToObject(pObj, "ip", pIpObj);
	char szUuid[EN_AO_UUID_LEN + 1] = { 0 };
	CAoHelper::GetUuid(szUuid, EN_AO_UUID_LEN);
	cJSON_AddItemToObject(pObj, "uuid", cJSON_CreateString(szUuid));
	ACTIVE_INFO& ai = m_mapUai[szUuid];
	ai.strChannelName = pszChannelName;
	memcpy(ai.szActiveCode, pAAA->ucResId, EN_ACT_CODE_LEN);
	m_mapCu[pszChannelName] = szUuid;
	char szResId[32] = { 0 };
	CAoHelper::GetHexString(szResId, pAAA->ucResId, EN_RES_ID_LEN);
	cJSON_AddItemToObject(pObj, "rid", cJSON_CreateString(szResId));
	char szRds[128] = { 0 };
	CAoHelper::GetHexString(szRds, pAAA->ucRDSerial, EN_RD_SERIAL_LEN);
	cJSON_AddItemToObject(pObj, "rds", cJSON_CreateString(szRds));
	cJSON_AddItemToObject(pObj, "index", cJSON_CreateNumber(pAAA->ucIndex));
	cJSON* pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "active", pObj);
	char* pszJsonBuf = cJSON_PrintUnformatted(pRoot);
	SendOutMsg(EN_MODULE_ID_KAFKA, EN_AO_MSG_SND2KAFKA, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf));
	cJSON_Delete(pRoot);
	cJSON_FreeBuf(pszJsonBuf);
}
void CAodsActiveModule::OnActiveFailedResult(const char* pszChannelName, unsigned char ucRlt){
	unsigned int unLen = (unsigned int)strlen(pszChannelName) + 1;
	unsigned char ucData[EN_MAX_TEXT_LEN] = {0};
	memcpy(ucData, pszChannelName, unLen);
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(ucData + unLen);
	pAAM->unMsgCode = EN_AODC_AODS_ACTV;
	AODC_AODS_ACTIVE* pAAA =(AODC_AODS_ACTIVE*)(pAAM->szMsg);
	pAAA->ucResult = ucRlt;
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, ucData, unLen + EN_AODS_AODC_MSG_SIZE + EN_AODC_AODS_ACTIVE_SIZE);
}
void CAodsActiveModule::OnActiveSucc(const char* pszAodsChannelName, char* pszActiveCode) {
	unsigned char ucResId[EN_RES_ID_LEN] = { 0 };
	memcpy(ucResId, pszActiveCode, EN_RES_ID_LEN);
	CalcActiveCode(pszActiveCode); // 计算激活码
	ReportActiveSuc(pszActiveCode, ucResId);
	RspActiveSuc(pszAodsChannelName, pszActiveCode, ucResId);
}
void CAodsActiveModule::CalcActiveCode(char* pszActiveCode){
	pszActiveCode[EN_ACT_CODE_LEN - 1] = CAoHash::CalcCrc8((unsigned char*)pszActiveCode, EN_ACT_CODE_LEN - 1);
	unsigned char ucKey[16] = { 0 };
	for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0x3f; }
	CAoEncrypt ae;ae.SetKey(ucKey, 16);ae.Encode((unsigned char*)pszActiveCode, EN_ACT_CODE_LEN);
}
void CAodsActiveModule::OnRecvFromKafka(const MT_MSG* pMM){
	std::string strJson((char*)pMM->ucData, pMM->unDataLen);
	cJSON* pObj = cJSON_Parse(strJson.c_str());
	if (NULL == pObj){return;}
	cJSON* pActiveObj = cJSON_GetObjectItem(pObj, "active_ack");
	if (NULL == pActiveObj) { cJSON_Delete(pObj); return; }
	cJSON* pProperty = cJSON_GetObjectItem(pActiveObj, "uuid");
	UAI_MAP_IT it = m_mapUai.find(pProperty->valuestring);
	if (m_mapUai.end() == it){ cJSON_Delete(pObj);  return;	}// 非激活消息或者已经过时的消息
	CU_MAP_IT it2 = m_mapCu.find(it->second.strChannelName);
	pProperty = cJSON_GetObjectItem(pActiveObj, "ack");
	std::string strAodsChannelName = it2->first;
	if (0 == pProperty->valueint) {
		pProperty = cJSON_GetObjectItem(pActiveObj, "rid");
		CAoHelper::GetHexData((unsigned char*)it->second.szActiveCode, pProperty->valuestring); // 资源号
		OnActiveSucc(strAodsChannelName.c_str(), it->second.szActiveCode);
	}else {OnActiveFailedResult(strAodsChannelName.c_str(), (unsigned char)pProperty->valueint);}
	cJSON_Delete(pObj);
	CA_MAP_IT itCA = m_mapCA.find(strAodsChannelName.c_str()); // 保证不重复认证
	if (m_mapCA.end() != itCA){	m_mapCA.erase(itCA);}
	m_mapCu.erase(it2);
	m_mapUai.erase(it);
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)strAodsChannelName.c_str(), (unsigned int)strAodsChannelName.length() + 1);
}
void CAodsActiveModule::ReportActiveSuc(const char* pszActiveCode, const unsigned char* ucResId) {
	cJSON* pObject = cJSON_CreateObject();
	char szHexActiveCode[128] = { 0 };
	CAoHelper::GetHexString(szHexActiveCode, (unsigned char*)pszActiveCode, EN_ACT_CODE_LEN);
	cJSON_AddItemToObject(pObject, "active_code", cJSON_CreateString(szHexActiveCode));
	char szHexRid[16] = { 0 };
	CAoHelper::GetHexString(szHexRid, ucResId, EN_RES_ID_LEN);
	cJSON_AddItemToObject(pObject, "rid", cJSON_CreateString(szHexRid));
	cJSON* pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "active_suc", pObject);
	char* pszJsonBuf = cJSON_PrintUnformatted(pRoot);
	SendOutMsg(EN_MODULE_ID_KAFKA, EN_AO_MSG_SND2KAFKA, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf));
	cJSON_Delete(pRoot);
	cJSON_FreeBuf(pszJsonBuf);
}
void CAodsActiveModule::RspActiveSuc(const char* pszAodsChannelName, const char* pszActiveCode, unsigned char* pucResId) {
	const unsigned int unDataLen = EN_AODS_AODC_MSG_SIZE + EN_AODC_AODS_ACTIVE_SIZE + EN_AODS_ACTIVE_SUC_SIZE;
	unsigned char ucData[unDataLen] = { 0 };
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucData;
	pAAM->unMsgCode = EN_AODC_AODS_ACTV;
	AODC_AODS_ACTIVE* pAAA = (AODC_AODS_ACTIVE*)(ucData + EN_AODS_AODC_MSG_SIZE);
	pAAA->ucResult = EN_ACT_SUCCESS;
	AODS_ACTIVE_SUC* pAAS = (AODS_ACTIVE_SUC*)pAAA->ucData;
	memcpy(pAAS->ucResId, pucResId, EN_RES_ID_LEN);
	memcpy(pAAS->ucActCode, pszActiveCode, EN_ACT_CODE_LEN);
	m_pAMC->Reset();
	if (!m_pAMC->AppendBuf((unsigned char*)pszAodsChannelName, (unsigned int)strlen(pszAodsChannelName) + 1) || !m_pAMC->AppendBuf(ucData, unDataLen)) {
		return;
	}
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
}