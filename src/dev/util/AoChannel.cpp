/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include "AoDef.h"
#include "AoChannel.h"
#include "Module.h"
CAoChannel::CAoChannel(CModule* pOb, const char* pszName):m_pOb(pOb){
	m_unFixedOffLen = (int)strlen(pszName) + 1;
	m_unTempBufLen = m_unFixedOffLen < 256 ? 10240 : m_unFixedOffLen + 10240;
	m_pucTempBuf = (unsigned char*)MALLOC(m_unTempBufLen);
	memcpy(m_pucTempBuf, pszName, m_unFixedOffLen);
	m_pucOffsetBuf = m_pucTempBuf + m_unFixedOffLen;
}
CAoChannel::~CAoChannel(){Stop();SAFE_FREE(m_pucTempBuf);}
bool CAoChannel::Init(cJSON* ){	return true;}
int CAoChannel::Run(){	return 0;}
void CAoChannel::SendMsg(const unsigned char* , unsigned int ){}
void CAoChannel::Stop(){}
int CAoChannel::Handle(){return 0;}
void CAoChannel::OnFree(){}
void CAoChannel::OnNew(){
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_NEW, (unsigned char*)this, (unsigned int)sizeof(this));
}
void CAoChannel::OnDel(){
	SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_DEL, (unsigned char*)this, (unsigned int)sizeof(this));
}
void CAoChannel::OnBuiled(){}
void CAoChannel::OnFreed(){SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_FREED);}
void CAoChannel::OnRecvedData(const unsigned char* pucData, unsigned int unDataLen){
	SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_RECV_DATA, pucData, unDataLen);
}
void CAoChannel::OnSendDataFailed(const unsigned char* pucData, unsigned int unDataLen){
	SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, pucData, unDataLen);
}
void CAoChannel::SendOutMsg(unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData /*= NULL*/, unsigned int unLen){
	if (NULL == m_pOb){	return;	}
	if (NULL != pucData && unLen > 0){
		if (m_unTempBufLen < m_unFixedOffLen + unLen){
			m_pucTempBuf = (unsigned char*)REALLOC(m_pucTempBuf, m_unFixedOffLen + unLen + 1024);
		}
		if (NULL == m_pucTempBuf){return;}
		memcpy(m_pucOffsetBuf, pucData, unLen);
	}else{unLen = 0;}
	m_pOb->SendOutMsg(unRMI, unMsgCode, m_pucTempBuf, unLen + m_unFixedOffLen);
}
