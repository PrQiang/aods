/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsClient_h__
#define AodsClient_h__
#include <list>
#include <string>
#include "AoEvent.h"
#include "AoSockChannel.h"
class CAoSockTcpChannel : public CAoSockChannel{
public:
    CAoSockTcpChannel(CModule* pOb, const char* pszChanelName, const char* pszProtocol);
    virtual ~CAoSockTcpChannel();
public:
	bool Init(cJSON* pJsonPara);
	virtual int Run();
	virtual void Stop();
protected:
	bool GetNextAddr(char* pszAddr, int& nPort);
	bool ParseAddr(const char* pszUri, char* pszAddr, int& nPort);
protected:
	enum{
		EN_MAX_TIMEOUT = 60000,
		EN_BUF_LEN = 10240,
		EN_CACHE_BUF_LEN = 2 * 1024 * 1024
	};
	typedef std::list<std::string> ADDR_LST;
	typedef ADDR_LST::iterator ADDR_LST_IT;
	ADDR_LST m_lstAddr;
	ADDR_LST_IT m_itCurAddr;
	CAoEvent m_evTimeout;
	bool m_bRunning;
};
#endif // AodsClient_h__
