/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include "AodcDef.h"
#include "../util/CJson.h"
#include "../util/AoDef.h"
#include "../util/AoHash.h"
#include "../util/AoHelper.h"
#include "../util/AoEncrypt.h"
#include "../AodsAodcMsgDef.h"
#include "../util/AoMemCache.h"
#include "AodsAuthModule.h"
CAodsAuthModule* CAodsAuthModule::ms_pInstance = NULL;
CAoLock CAodsAuthModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAodsAuthModule, CModule)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CAodsAuthModule::OnRecvChannelData)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, &CAodsAuthModule::OnSendChannelDataFail)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_FREED, &CAodsAuthModule::OnChannelFreed)
	ON_AO_MSG_FUN(EN_AO_MSG_RCVFKAFKA, &CAodsAuthModule::OnRecvFromKafka)
END_AO_MSG_MAP()
CAodsAuthModule::CAodsAuthModule():CModule(EN_MODULE_ID_AUTH, 1024*1024, "AodsAuthModule"){
	m_pszAodsChannelName = "AodsServer";
	m_unAodsChannelNameLen = (unsigned int)strlen(m_pszAodsChannelName) + 1;
	m_pAMC = new CAoMemCache(10240, 10240);
}
CAodsAuthModule::~CAodsAuthModule(){SAFE_DELETE(m_pAMC);}
CAodsAuthModule* CAodsAuthModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CAodsAuthModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CAodsAuthModule::Release(){SAFE_DELETE(ms_pInstance);}
void CAodsAuthModule::OnRecvChannelData(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	unsigned int unChannelNameLen = (unsigned int)strlen(pszChannelName) + 1;
	if (unChannelNameLen >= pMM->unDataLen){ // 错误数据
		return;
	}
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){ // 接收aods客户端消息	
		OnRecvAodsMsg(pszChannelName, pMM->ucData + unChannelNameLen, pMM->unDataLen - unChannelNameLen);
	}
}
void CAodsAuthModule::OnSendChannelDataFail(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
        CR_MAP_IT cit = m_mapCR.find(pszChannelName);
		if (m_mapCR.end() == cit){return;}
		RC_MAP_IT rit = m_mapRC.find(cit->second);
		if (m_mapRC.end() != rit){
            m_mapRC.erase(rit);
        }
		m_mapCR.erase(cit);
	}
}
void CAodsAuthModule::OnChannelFreed(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	if (strstr(pszChannelName, m_pszAodsChannelName) != NULL){
        CR_MAP_IT cit = m_mapCR.find(pszChannelName);
		if (m_mapCR.end() == cit){return;}
		RC_MAP_IT rit = m_mapRC.find(cit->second);
        if (m_mapRC.end() != rit){
            m_mapRC.erase(rit);
        }
        m_mapCR.erase(cit);
	}
}
void CAodsAuthModule::OnRecvAodsMsg(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen){
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)pucData;
	if (EN_AODS_AODC_AUTH != pAAM->unMsgCode){return;}
	if ((EN_AODS_AODC_AUTHEN_SIZE + EN_AODS_AODC_MSG_SIZE) != unDataLen){
		SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
		return;
	}
	AODS_AODC_AUTHEN* pAAA = (AODS_AODC_AUTHEN*)pAAM->szMsg;
	unsigned char ucActCode[EN_ACT_CODE_LEN] = {0}; // 激活码
    memcpy(ucActCode, pAAA->ucResId, EN_ACT_CODE_LEN);
	CalcActiveCode((char*)ucActCode); // 计算激活码
    if (memcmp(ucActCode, pAAA->ucActCode, EN_ACT_CODE_LEN) != 0){// 初步校验失败
		SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
		return;
	}
	char szResId[16] = { 0 }; // 建立资源号与通道映射关系
	CAoHelper::GetHexString(szResId, pAAA->ucResId, EN_RES_ID_LEN);
    AUTH_INFO& ai = m_mapRC[szResId];
    if (ai.strChannelName.length() > 0){
        if (ai.ucIpNum != pAAA->ucIpNum || memcmp(ai.unIpAddr, pAAA->unIpAddr, ai.ucIpNum * 4) != 0){ // 可能是克隆机，直接拒绝
			m_pAMC->Reset();
            m_pAMC->AppendBuf((unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1); // 通道名称
            const unsigned int unMsgLen = EN_AODC_AODS_AUTHEN_SIZE + EN_AODS_AODC_MSG_SIZE;
            unsigned char ucData[unMsgLen] = { 0 };
            AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucData;
            pAAM->unMsgCode = EN_AODC_AODS_AUTH;
            AODC_AODS_AUTHEN* pAAA = (AODC_AODS_AUTHEN*)pAAM->szMsg;
            pAAA->ucRlt = EN_AUTH_RESULT_DIF;
            m_pAMC->AppendBuf(ucData, unMsgLen);
            SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
            SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
        }
        else{ // 可能由于网络原因造成的延迟释放连接，向2个通道发送断开连接，等待客户端重连
			SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)ai.strChannelName.c_str(), (unsigned int)ai.strChannelName.length() + 1);
            SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_FREE, (unsigned char*)pszChannelName, (unsigned int)strlen(pszChannelName) + 1);
        }
       return;
    }
    ai.strChannelName = pszChannelName;
    ai.ucIpNum = pAAA->ucIpNum;
    memcpy(ai.unIpAddr, pAAA->unIpAddr, EN_MAX_IP_ADDR_NUM * 4);
    m_mapCR[pszChannelName] = szResId;
	ReportAuthenMsg((char*)pAAA->ucActCode, szResId);
}
void CAodsAuthModule::CalcActiveCode(char* pszActiveCode){
    pszActiveCode[EN_ACT_CODE_LEN - 1] = CAoHash::CalcCrc8((unsigned char*)pszActiveCode, EN_ACT_CODE_LEN - 1);
	unsigned char ucKey[16] = { 0 };
	for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0x3f; }
	CAoEncrypt ae;ae.SetKey(ucKey, 16);ae.Encode((unsigned char*)pszActiveCode, EN_ACT_CODE_LEN);
}
void CAodsAuthModule::ReportAuthenMsg(const char* pszActiveCode, const char* pszHexResId){
	char szHexAc[128] = { 0 };
	CAoHelper::GetHexString(szHexAc, (const unsigned char*)pszActiveCode, EN_ACT_CODE_LEN);
	cJSON* pObj = cJSON_CreateObject();
	cJSON_AddItemToObject(pObj, "active_code", cJSON_CreateString(szHexAc));
	cJSON_AddItemToObject(pObj, "rid", cJSON_CreateString(pszHexResId));
	cJSON* pRoot = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, "auth", pObj);
	char* pszJsonBuf = cJSON_PrintUnformatted(pRoot);
	SendOutMsg(EN_MODULE_ID_KAFKA, EN_AO_MSG_SND2KAFKA, (unsigned char*)pszJsonBuf, (unsigned int)strlen(pszJsonBuf));
	cJSON_Delete(pRoot);
	cJSON_FreeBuf(pszJsonBuf);
}
void CAodsAuthModule::OnRecvFromKafka(const MT_MSG* pMM) {
	std::string strJson((char*)pMM->ucData, pMM->unDataLen);
	cJSON* pObj = cJSON_Parse(strJson.c_str());
	if (NULL == pObj) { return; }
	cJSON* pAuthObj = cJSON_GetObjectItem(pObj, "auth_ack");
	if (NULL == pAuthObj) {
		cJSON_Delete(pObj);
		return;
	}
	cJSON* pRid = cJSON_GetObjectItem(pAuthObj, "rid");
	RC_MAP_IT it = m_mapRC.find(pRid->valuestring);
	if (m_mapRC.end() == it){
		cJSON_Delete(pObj);
		return;
	}
	AUTH_INFO& ai = it->second;
	cJSON* pAck = cJSON_GetObjectItem(pAuthObj, "ack");
	if (0 == pAck->valueint) { // 向其他模块广播认证成功
		m_pAMC->Reset();
		m_pAMC->AppendBuf((unsigned char*)pRid->valuestring, (unsigned int)strlen(pRid->valuestring) + 1);
		m_pAMC->AppendBuf((unsigned char*)ai.strChannelName.c_str(), (unsigned int)ai.strChannelName.length() + 1);
		SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_AUTH_SUC, m_pAMC->CurData(), m_pAMC->CurDataLen());
	}
	const unsigned int unMsgLen = EN_AODC_AODS_AUTHEN_SIZE + EN_AODS_AODC_MSG_SIZE;
	unsigned char ucData[unMsgLen] = { 0 };
	AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)ucData;
	pAAM->unMsgCode = EN_AODC_AODS_AUTH;
	AODC_AODS_AUTHEN* pAAA = (AODC_AODS_AUTHEN*)pAAM->szMsg;
	pAAA->ucRlt = (unsigned char)pAck->valueint;
	m_pAMC->Reset();
	m_pAMC->AppendBuf((unsigned char*)ai.strChannelName.c_str(), (unsigned int)ai.strChannelName.length() + 1);
	m_pAMC->AppendBuf(ucData, unMsgLen);
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, m_pAMC->CurData(), m_pAMC->CurDataLen());
}