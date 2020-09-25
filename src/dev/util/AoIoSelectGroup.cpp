/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoDef.h"
#include "AoLock.h"
#include "AoEvent.h"
#include "AoSock.h"
#include "AoSockChannel.h"
#include "AoIoSelectGroup.h"
#ifdef WIN32
#define AO_FD_SET(fd, set) do { \
    if (((fd_set FAR *)(set))->fd_count < EN_MAX_IO_COUNT) \
        ((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd);\
} while(0)
#else
#define AO_FD_SET(fd, set) FD_SET(fd, set)
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#endif
CAoIoSelectGroup::CAoIoSelectGroup(){m_bRun = true;m_pLock = new CAoLock();m_pEvTimeout = new CAoEvent();Start();}
CAoIoSelectGroup::~CAoIoSelectGroup(){
	m_lstASC.clear();
	SAFE_DELETE(m_pLock);
	SAFE_DELETE(m_pEvTimeout);
}
int CAoIoSelectGroup::Run(){
	timeval tv;tv.tv_sec = 0;tv.tv_usec  = 500;
	int nSock = 0;int nMaxSock = 0;int nBufLen = 10240;int nRet = 0;	
	char* pszBuf = (char*)MALLOC(nBufLen);	
	CAoSock as;	ASC_LSTP_IT it;	CAoSockChannel* pASC = NULL;
	fd_set* pSet = (fd_set*)MALLOC(sizeof(u_int) + sizeof(int) * EN_MAX_IO_COUNT);
	while(m_bRun){
		if(0 == GetIoCount()){m_pEvTimeout->Wait(1000);continue;}
		FD_ZERO(pSet);nMaxSock = 0;
		m_pLock->Lock();
		for (it = m_lstASC.begin(); it != m_lstASC.end();){
			nSock = (*it)->Socket();as.SetSocket(nSock);
			if (1 == as.IsValild()){AO_FD_SET(nSock, pSet);nMaxSock = ( nMaxSock < nSock ) ? nSock : nMaxSock;++it;
			}else{(*it)->OnDel();	it = m_lstASC.erase(it);}}
		m_pLock->Unlock();
		if (0 == nMaxSock){continue;}
		if(1 > (nRet = select(nMaxSock + 1, pSet, NULL, NULL, &tv))){continue;}
		m_pLock->Lock();
		for (it = m_lstASC.begin(); it != m_lstASC.end(); ){
			pASC = (*it);as.SetSocket(pASC->Socket());
			if (!as.IsValild() || (FD_ISSET(pASC->Socket(), pSet) && pASC->ReadData(pszBuf, nBufLen) < 1)){
				pASC->OnDel();it = m_lstASC.erase(it);}
			else{ ++it; }			
		}
		m_pLock->Unlock();
	}
	as.SetSocket(-1);
	SAFE_FREE(pSet);
	SAFE_FREE(pszBuf);
	return 0;
}
bool CAoIoSelectGroup::Append(CAoSockChannel* pASC){
	if (GetIoCount() >= EN_MAX_IO_COUNT){return false;}
	m_pLock->Lock();m_lstASC.push_back(pASC);m_pLock->Unlock();
	m_pEvTimeout->Signal();
	return true;
}
void CAoIoSelectGroup::Stop(){m_bRun = false;m_pEvTimeout->Signal();Join();}
int CAoIoSelectGroup::GetIoCount(){m_pLock->Lock();int nSize = (int)m_lstASC.size();m_pLock->Unlock();return nSize;}