/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifdef WIN32
#else
#include <string.h>
#endif
#include "AoDef.h"
#include "CJson.h"
#include "AoSockTcp.h"
#include "CommuModule.h"
#include "AoTcpServer.h"
#include "AoIoSelectGroup.h"
#include "AoSockTcpChannel.h"
// 静态成员变量声明
CCommuModule* CCommuModule::ms_pInstance = NULL;
CAoLock CCommuModule::ms_Lock;
BEGIN_AO_MSG_MAP(CCommuModule, CModule)
	ON_AO_MSG_FUN(EN_AO_MSG_LOAD_CFG, &CCommuModule::LoadConfig)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA, &CCommuModule::SendData2Channel)
	ON_AO_MSG_FUN(EN_AO_MSG_EXIT, &CCommuModule::OnStop)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_NEW, &CCommuModule::OnNewChannel)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_DEL, &CCommuModule::OnDelChannel)
	ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_FREE, &CCommuModule::OnFreeChannel)
END_AO_MSG_MAP()
CCommuModule::CCommuModule(): CModule(EN_MODULE_ID_COMMU, 2 * 1024 * 1024, "CommuModule"){
#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);
#endif
	for (int n = 0; n < EN_MAX_IO_SELECT_GROUP_COUNT; ++n){m_pAISG[n] = NULL;}
}
CCommuModule::~CCommuModule(){
#ifdef WIN32
	WSACleanup();
#endif
	for (AC_MAP_IT it = m_mapAC.begin(); it != m_mapAC.end(); ++it)	{SAFE_DELETE(it->second);}
	m_mapAC.clear();
	for (int n = 0; n < EN_MAX_IO_SELECT_GROUP_COUNT; ++n){SAFE_DELETE(m_pAISG[n]);}
}
CCommuModule* CCommuModule::Instance(){
	if(NULL == ms_pInstance){
		ms_Lock.Lock();
		ms_pInstance = (NULL == ms_pInstance) ? new CCommuModule : ms_pInstance;
		ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CCommuModule::Release(){SAFE_DELETE(ms_pInstance);}
void CCommuModule::LoadConfig(const MT_MSG* pMM){
	cJSON* pObject = cJSON_Parse((const char*)pMM->ucData);
	if (NULL == pObject){
		LogErr("CCommuModule::LoadConfig", "Faild to load config parameter");
		return ;
	}
	for (cJSON* p = pObject->child; NULL != p; p = p->next){
		if(!CreateChannleByPara(p)){
			LogWarn("CCommuModule::LoadConfig", "The channel(%s) Created faied", p->string);
		}
	}
	cJSON_Delete(pObject);
}
void CCommuModule::SendData2Channel(const MT_MSG* pMM){
	const char* pszChannelName = (const char*)pMM->ucData;
	unsigned int unNameLen = (unsigned int)strlen(pszChannelName) + 1;
	if (pMM->unDataLen < unNameLen){
		SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, pMM->ucData, pMM->unDataLen);
		return;
	}
	AC_MAP_IT it = m_mapAC.find(std::string(pszChannelName));
	if (m_mapAC.end() == it || NULL == it->second){
		SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, pMM->ucData, pMM->unDataLen);
		return;
	}
	it->second->SendMsg(pMM->ucData + unNameLen, pMM->unDataLen - unNameLen);
}
void CCommuModule::OnStop(const MT_MSG* pMM){
	CModule::OnStop(pMM);
	for (AC_MAP_IT it = m_mapAC.begin(); it != m_mapAC.end(); ++it){it->second->Stop();}
}
void CCommuModule::OnNewChannel(const MT_MSG* pMM){
	if (pMM->unDataLen < 1 || '\0' != pMM->ucData[pMM->unDataLen - 1] ){return;}
	const char* pszChannelName = (const char*)(pMM->ucData);
	AC_MAP_IT it = m_mapAC.find(pszChannelName);
	if (m_mapAC.end() == it){return;}
	unsigned int unChannleNameLen = (unsigned int)strlen(pszChannelName) + 1;
	if (unChannleNameLen + 1 > pMM->unDataLen ){return;}
	const char* pszCreatePara = pszChannelName + unChannleNameLen;
	cJSON* pPara = cJSON_Parse(pszCreatePara);
	if (NULL == pPara){return;}
	for (cJSON* pObj = pPara->child; NULL != pObj; pObj = pObj->next){
		if(!CreateChannleByPara(pObj)){
			LogWarn("CCommuModule::OnNewChannel", "The channel(%s) Created faied", pObj->string);
		}
	}
	cJSON_Delete(pPara);
}
void CCommuModule::OnDelChannel(const MT_MSG* pMM){
	if ('\0' != pMM->ucData[pMM->unDataLen - 1] ){return;}
	const char* pszChannelName = (const char*)(pMM->ucData);
	AC_MAP_IT it = m_mapAC.find(pszChannelName);
	if (m_mapAC.end() != it){SAFE_DELETE(it->second);m_mapAC.erase(it);}
}
void CCommuModule::OnFreeChannel(const MT_MSG* pMM){
	if ('\0' != pMM->ucData[pMM->unDataLen - 1]){return;}
	const char* pszChannelName = (const char*)(pMM->ucData);
	AC_MAP_IT it = m_mapAC.find(pszChannelName);
	if (m_mapAC.end() != it){it->second->OnFree();}
}
bool CCommuModule::CreateChannleByPara(cJSON* pPara){
	if (NULL == pPara){return false;}
	cJSON* pType = cJSON_GetObjectItem(pPara, "Type");
	if (NULL == pType || cJSON_String != pType->type){return false;}
	const char* pszCommuProtocol = "AoCommuProtocol";
	cJSON* pCommuProtocol = cJSON_GetObjectItem(pPara, "Protocol");
	if (NULL != pCommuProtocol  && cJSON_String == pCommuProtocol->type){
		pszCommuProtocol = pCommuProtocol->valuestring;
	}
	if (0 == strcmp(pType->valuestring, "AoSockChannel")){
		return CreateAoSockChannel(pPara->string, pszCommuProtocol, cJSON_GetObjectItem(pPara, "Para"));	
	}
	if (0 == strcmp(pType->valuestring, "TcpClientChanel")){
		return CreateAoSockTcpChannel(pPara->string, pszCommuProtocol, cJSON_GetObjectItem(pPara, "Para"));
	}
	if (0 == strcmp(pType->valuestring, "TcpServer")){
		return CreateAoTcpServer(pPara->string, cJSON_GetObjectItem(pPara, "Para"));
	}
	return false;
}
bool CCommuModule::CreateAoSockChannel(const char* pszChannelName, const char* pszProtocol, cJSON* pPara){
	if (NULL == pPara || cJSON_Object != pPara->type){return false;}
	cJSON* pProtocol = cJSON_GetObjectItem(pPara, "Protocol");
	if (NULL == pProtocol || cJSON_String != pProtocol->type){return false;}
	cJSON* pHandle = cJSON_GetObjectItem(pPara, "Handle");
	if (NULL == pHandle || cJSON_Number != pHandle->type){return false;}
	CAoSock* pAS = NULL;
	if (strcmp(pProtocol->valuestring, "Tcp") == 0){pAS = new CAoSockTcp();}
	if (NULL == pAS){return false;}
	pAS->SetSocket(pHandle->valueint);
    pAS->SetRecvTimeout(60000);
	CAoSockChannel* pASC = new CAoSockChannel(this, pszChannelName, pszProtocol, pAS);
	m_mapAC[pszChannelName] = pASC;
	pASC->OnBuiled();
 	if (!AppendSockChannle2IoSelectGroup(pASC)){
 		m_mapAC.erase(m_mapAC.find(pszChannelName));
 		SAFE_DELETE(pASC);
 		return false;
 	}
	return true;
}
bool CCommuModule::AppendSockChannle2IoSelectGroup(CAoSockChannel* pASC){
	for (int n = 0; n < EN_MAX_IO_SELECT_GROUP_COUNT; ++n){
		if (NULL == m_pAISG[n]){m_pAISG[n] = new CAoIoSelectGroup();}
		if (m_pAISG[n]->Append(pASC)){return true;}
	}
	return false;
}
bool CCommuModule::CreateAoSockTcpChannel(const char* pszChannelName, const char* pszProtocol, cJSON* pPara){
	CAoSockTcpChannel* pASTC = new CAoSockTcpChannel(this, pszChannelName, pszProtocol);
	if (NULL == pASTC){return false;}
	if(!pASTC->Init(pPara)){SAFE_DELETE(pASTC);return false;}
	m_mapAC[pszChannelName] = pASTC;
	return true;
}
bool CCommuModule::CreateAoTcpServer(const char* pszChannelName, cJSON* pPara){
	CAoTcpServer* pATS = new CAoTcpServer(this, pszChannelName);
	if (NULL == pATS){return false;}
	if (!pATS->Init(pPara)){SAFE_DELETE(pATS);return false;}
	m_mapAC[pszChannelName] = pATS;
	return true;
}