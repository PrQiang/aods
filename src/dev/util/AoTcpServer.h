/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoTcpServer_h__
#define AoTcpServer_h__
#include <list>
#include <string>
#include "AoChannel.h"
class CAoSockTcp;
class CAoTcpServer : public CAoChannel{
public:
    CAoTcpServer(CModule* pOb, const char* pszName);
    virtual ~CAoTcpServer();
	virtual bool Init(cJSON* pJsonPara);
	virtual int Run();
	virtual void Stop();
protected:
	bool BuildServerChannels();
protected:
	struct SERVER_CHANNEL{
		SERVER_CHANNEL(const char* pszName, const char* pszAddr, const char* pszProtocol, int nPort, int nNum)
			:strName(pszName), strAddr(pszAddr), strProtocol(pszProtocol){
			this->nNum = nNum;this->nPort = nPort;
			ullIndex = 0;pAST = NULL;
		}
		~SERVER_CHANNEL(){if (NULL != pAST){delete pAST;pAST = NULL;}}
		std::string strName;    // ͨ����Ӧ��ҵ������
		std::string strAddr;    // ��IP
		std::string strProtocol; // Э������
		int nPort; // �󶨵�ַ
		int nNum; // ��������
		unsigned long long ullIndex;
		CAoSockTcp* pAST;
	};
	enum{EN_MAX_SERVER_NUM = 256};
	typedef std::list<SERVER_CHANNEL*> SC_LST;
	typedef SC_LST::iterator SC_LST_IT;
	SC_LST m_lstSC;
	bool m_bRun;
};

#endif // AoTcpServer_h__