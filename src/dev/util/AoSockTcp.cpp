/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoSockTcp.h"
#ifdef WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#endif
CAoSockTcp::CAoSockTcp(){}
CAoSockTcp::~CAoSockTcp(){}
int CAoSockTcp::Bind( const char* pszAddr, int nPort ){
	if (IsValild() == 0){m_nSocket = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);}
	sockaddr_in sockAddr={0};
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((unsigned short)nPort);
	sockAddr.sin_addr.s_addr = ::inet_addr(pszAddr);
	return ::bind(m_nSocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
}
int CAoSockTcp::Listen( int nNum ){return listen(m_nSocket, nNum);}
int CAoSockTcp::Accept(){
	sockaddr_in sockAddr = {0};
#ifdef WIN32
	int len = sizeof(sockaddr_in);
#else
	socklen_t len = sizeof(sockaddr_in);
#endif
	return (int)(::accept(m_nSocket, (sockaddr*)&sockAddr, &len));
}
int CAoSockTcp::Connect( const char* pszAddr, int nPort ){
	if (IsValild() == 0){m_nSocket = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);}
	sockaddr_in sockAddr={0};
	sockAddr.sin_addr.s_addr = ::inet_addr(pszAddr);
	if(INADDR_NONE == sockAddr.sin_addr.s_addr){
		char szAddr[256] = {0};
		if(!ParseDomain(pszAddr, szAddr)){return -1;}
		sockAddr.sin_addr.s_addr = ::inet_addr(szAddr);
		if (INADDR_NONE == sockAddr.sin_addr.s_addr){return -1;}	
	}
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((unsigned short)nPort);
	return ::connect(m_nSocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
}
int CAoSockTcp::Recv( char* pszBuf, int nLen ){return IsValild() ? recv(m_nSocket, pszBuf, nLen, 0) : -1;}
int CAoSockTcp::Send( const char* pszBuf, int nLen ){
	if (IsValild()){return send(m_nSocket, pszBuf, nLen, 0);}
	return -1;
}
int CAoSockTcp::SetPortReused(int nUsed){
	return IsValild()?setsockopt(m_nSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nUsed, sizeof(nUsed)):-1;
}
