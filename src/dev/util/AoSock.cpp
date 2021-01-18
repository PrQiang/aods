/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoSock.h"
#include <stdio.h>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include "AoDef.h"
#include "AoLock.h"
CAoSock::CAoSock(){	m_nSocket = EN_INVALID_SOCK;	m_pLock = new CAoLock();}
CAoSock::~CAoSock(){	Close();	SAFE_DELETE(m_pLock);}
int CAoSock::Socket(){	return m_nSocket;}
void CAoSock::SetSocket( int nSocket ){	m_nSocket = nSocket;}
int CAoSock::Bind( const char* , int  ){	return -1;}
int CAoSock::Listen( int  ){	return -1;}
int CAoSock::Connect( const char* , int  ){	return -1;}
int CAoSock::IsValild(){	return (EN_INVALID_SOCK == m_nSocket) ? 0 : 1;}
int CAoSock::Accept(){	return -1;}
void CAoSock::Close(){
	m_pLock->Lock();
	try{if (EN_INVALID_SOCK != m_nSocket){
#ifdef WIN32
			closesocket(m_nSocket);
#else
			close(m_nSocket);
#endif
			m_nSocket = EN_INVALID_SOCK;
		}
	}
	catch (...)	{	}	
	m_pLock->Unlock();
}
int CAoSock::Recv( char* , int  ){	return -1;}
int CAoSock::Send( const char* , int  ){	return -1;}
void CAoSock::SetRecvTimeout(int nTimeout){	if (IsValild())	{
#ifdef WIN32
		setsockopt( m_nSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeout, sizeof(nTimeout));
#else
        struct timeval timeout = { nTimeout/1000, (nTimeout%1000) * 1000 };
        setsockopt(m_nSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)& timeout, sizeof(timeout));
#endif
	}
}
bool CAoSock::GetLocalAddr(char* pszAddr, int ){
	if (1 != IsValild()){return false;}	
	sockaddr_in saddr;
#ifdef WIN32
	int nSize = sizeof(sockaddr_in);
#else
	socklen_t nSize = sizeof(sockaddr_in);
#endif
	if(0 != getsockname(m_nSocket, (sockaddr*)&saddr, &nSize)){	return false;}
	sprintf(pszAddr, "%s:%d", ::inet_ntoa(saddr.sin_addr), saddr.sin_port);
	return true;
}
bool CAoSock::GetPeerAddr(char* pszAddr, int ){
	if (1 != IsValild()){return false;}
	sockaddr_in saddr;
#ifdef WIN32
	int nSize = sizeof(sockaddr_in);
#else
	socklen_t nSize = sizeof(sockaddr_in);
#endif
	if (0 != getpeername(m_nSocket, (sockaddr*)&saddr, &nSize)){return false;}
	sprintf(pszAddr, "%s:%d", ::inet_ntoa(saddr.sin_addr), saddr.sin_port);
	return true;
}
bool CAoSock::ParseDomain(const char* pszDomain, char* pszAddr){
	struct hostent* p = gethostbyname(pszDomain);
	if (NULL == p){return false;}	
	if(AF_INET != p->h_addrtype){return false;}
	unsigned char* pucAddr = (unsigned char*)p->h_addr_list[0];
	sprintf(pszAddr, "%u.%u.%u.%u", pucAddr[0], pucAddr[1], pucAddr[2], pucAddr[3]);
	return true;
}