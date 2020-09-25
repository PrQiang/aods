/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include "AoDef.h"
#include "AoSock.h"
#include "AoSockChannel.h"
#include "AoCommuProtocol.h"
#include "DBACommuProtocol.h"
CAoSockChannel::CAoSockChannel(CModule* pOb, const char* pszChannelName, const char* pszProtocol, CAoSock* pAS)
	:CAoChannel(pOb, pszChannelName){
	if(0 == strcmp(pszProtocol, "DBACommuProtocol")){m_pACP = new CDBACommuProtocol(10240);}
	else{m_pACP = new CAoCommuProtocol(10240);}
	m_pAS = pAS;	
}
CAoSockChannel::~CAoSockChannel(){	OnFreed();	SAFE_DELETE(m_pAS);	SAFE_DELETE(m_pACP);}
int CAoSockChannel::Socket(){	return (NULL == m_pAS) ? -1 : m_pAS->Socket();}
int CAoSockChannel::ReadData(char* pszBuf, int nLen){
	if (NULL == m_pAS || NULL == m_pACP){return -1;}
	int nReadedLen = m_pAS->Recv(pszBuf, nLen);
	if (nReadedLen < 1){return nReadedLen;}
	if(!m_pACP->AppendDecodeData(pszBuf, nReadedLen)){return 0;}
	int nIsErr = 0;
	const char* pszMsgData = NULL;
	unsigned int unMsgDataLen = 0;
	while (NULL != (pszMsgData = m_pACP->GetContent(unMsgDataLen, nIsErr)))	{
		OnRecvedData((const unsigned char*)pszMsgData, unMsgDataLen); // 接收数据
		m_pACP->MoveNext(); // 移动至协议下一条
	}
	return nReadedLen;
}
void CAoSockChannel::SendMsg(const unsigned char* pucData, unsigned int unDataLen){
	if (NULL == m_pAS || NULL == m_pACP){
		OnSendDataFailed(pucData, unDataLen);
		return;
	}
	unsigned int unMsgLen = 0;
	char* pszBuf = m_pACP->EncodeMsg(pucData, unDataLen, unMsgLen);
	if (NULL == pszBuf){
		OnSendDataFailed(pucData, unDataLen);
		return;
	}
	int nSendedLen = 0;
	for (int nOffset = 0; nOffset < (int)unMsgLen; nOffset += nSendedLen){
		nSendedLen = m_pAS->Send(pszBuf + nOffset, unMsgLen - nOffset);
		if (nSendedLen < 1){
			OnSendDataFailed(pucData, unDataLen);
			return;
		}
	}
}
void CAoSockChannel::OnFree(){if (NULL != m_pAS){m_pAS->Close();}}
void CAoSockChannel::OnBuiled(){
	if (NULL == m_pAS){return;}
	char szAddr[256] = {0};
	m_pAS->GetLocalAddr(szAddr, 255);
	unsigned int unLen = (unsigned int)strlen(szAddr) + 1;
	char* pszPeer = szAddr + unLen;
	m_pAS->GetPeerAddr(pszPeer, 256 - unLen);
	unLen += (unsigned int)strlen(pszPeer) + 1;
	SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_BUILDED, (unsigned char*)szAddr, unLen);
}