/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoSockTcp_h__
#define AoSockTcp_h__
#include "AoSock.h"
class CAoSockTcp : public CAoSock{
public:
    CAoSockTcp();
    virtual ~CAoSockTcp();
	virtual int Bind(const char* pszAddr, int nPort);
	virtual int Listen(int nNum);
	virtual int Accept();
	virtual int Connect(const char* pszAddr, int nPort);
	virtual int Recv(char* pszBuf, int nLen);
	virtual int Send(const char* pszBuf, int nLen);
	virtual int SetPortReused(int nUsed);
};
#endif // AoSockTcp_h__