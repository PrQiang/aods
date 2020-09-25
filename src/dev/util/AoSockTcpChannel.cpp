/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif
#include "CJson.h"
#include "AoDef.h"
#include "AoTime.h"
#include "AoSockTcp.h"
#include "AoSockTcpChannel.h"
CAoSockTcpChannel::CAoSockTcpChannel(CModule* pOb, const char* pszChanelName, const char* pszProtocol)
	:CAoSockChannel(pOb, pszChanelName, pszProtocol, NULL){	m_bRunning = false;}
CAoSockTcpChannel::~CAoSockTcpChannel(){	Stop();	SAFE_DELETE(m_pAS);}
bool CAoSockTcpChannel::Init(cJSON* pJsonPara){
	if (NULL == pJsonPara){return false;}
	m_lstAddr.clear();
	for (cJSON* p = pJsonPara->child; NULL != p; p = p->next){
		if (cJSON_String != p->type){continue;}
		m_lstAddr.push_back(std::string(p->valuestring));
	}
	m_itCurAddr = m_lstAddr.begin();
	return 0 == Start(); // 启动线程
}
int CAoSockTcpChannel::Run(){	
	int nPort = 0;
	m_bRunning = true;
	unsigned int unCurTime = 0;
	unsigned int unPreTime = 0;
	char szAddr[256] = {0};
	char* pszBuf = (char*)MALLOC(EN_BUF_LEN);
	m_pAS = new CAoSockTcp();
	while(m_bRunning){
		unCurTime = (unsigned int)CAoTime::CurrentTick();
		if (unPreTime + 5000 > unCurTime){ // 重连低于5
			m_evTimeout.Wait(5000);
			unPreTime = 0;
			continue;
		}
		unPreTime = CAoTime::CurrentTick();
		if(!GetNextAddr(szAddr, nPort)){strcpy(szAddr, "aodc.qpgame.com");nPort = 7050;}
		m_pAS->Close();		
		if(0 != m_pAS->Connect(szAddr, nPort)){ // 连接失败
			printf("连接服务器(%s:%d)失败\r\n", szAddr, nPort);
			continue;
		}
		OnBuiled();
		m_pAS->SetRecvTimeout(EN_MAX_TIMEOUT);
		while (m_bRunning && ReadData(pszBuf, EN_BUF_LEN) > 0){}
		OnFreed();
	}
	SAFE_FREE(pszBuf);
	return 0;
}
void CAoSockTcpChannel::Stop(){
	m_bRunning = false;
    OnFree();
	m_evTimeout.Signal();
	CAoThread::Join();
}
bool CAoSockTcpChannel::GetNextAddr(char* pszAddr, int& nPort){
	bool bRet = false;
	if (m_lstAddr.end() == m_itCurAddr){
		m_itCurAddr = m_lstAddr.begin();
	}
	if (m_lstAddr.end() != m_itCurAddr){
		bRet = ParseAddr(m_itCurAddr->c_str(), pszAddr, nPort);
		++m_itCurAddr;
	}
	return bRet;
}
bool CAoSockTcpChannel::ParseAddr(const char* pszUri, char* pszAddr, int& nPort){
	char szUri[256] = {0};
	strcpy(szUri, pszUri);
	for (std::size_t t = 0; t < strlen(szUri) - 1; ++t){
		if (':' == szUri[t]){
			szUri[t] = '\0';
			strcpy(pszAddr, szUri);
            sscanf(szUri + t + 1, "%d", &nPort);
			return true;
		}
	}
	return false;
}