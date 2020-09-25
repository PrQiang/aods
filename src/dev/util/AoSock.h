/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoSock_h__
#define AoSock_h__
class CAoLock;
class CAoSock{
public:
    CAoSock();
    virtual ~CAoSock();
	int Socket();
	void SetSocket(int nSocket);
	virtual int Bind(const char* pszAddr, int nPort);
	virtual int Listen(int nNum);
	virtual int Connect(const char* pszAddr, int nPort);
	virtual int IsValild();
	virtual int Accept();
	virtual void Close();
	virtual int Recv(char* pszBuf, int nLen);
	virtual int Send(const char* pszBuf, int nLen);
	void SetRecvTimeout(int nTimeout);
	bool GetLocalAddr(char* pszAddr, int nAddrLen);
	bool GetPeerAddr(char* pszAddr, int nAddrLen);
protected:
	bool ParseDomain(const char* pszDomain, char* pszAddr);
protected:
	enum{EN_INVALID_SOCK = -1};
	int m_nSocket;
	CAoLock* m_pLock;
};
#endif // AoSock_h__