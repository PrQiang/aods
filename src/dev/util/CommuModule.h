/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef CommuModule_h__
#define CommuModule_h__
#include "../util/AoLock.h"
#include "../util/Module.h"
#include <string>
#include <map>
struct cJSON;
class CAoChannel;
class CAoSockChannel;
class CAoIoSelectGroup;
class CCommuModule : public CModule{
protected:
    CCommuModule();
    ~CCommuModule();
public:
    static CCommuModule* Instance();
    static void Release();
protected:
	void LoadConfig(const MT_MSG* pMM);
	void SendData2Channel(const MT_MSG* pMM);
	void OnStop(const MT_MSG* pMM);
	void OnNewChannel(const MT_MSG* pMM);
	void OnDelChannel(const MT_MSG* pMM);
	void OnFreeChannel(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	bool CreateChannleByPara(cJSON* pPara);
	bool CreateAoSockChannel(const char* pszChannelName, const char* pszProtocol, cJSON* pPara);
	bool AppendSockChannle2IoSelectGroup(CAoSockChannel* pASC);
	bool CreateAoSockTcpChannel(const char* pszChannelName, const char* pszProtocol, cJSON* pPara);
	bool CreateAoTcpServer(const char* pszChannelName, cJSON* pPara);
protected:
    static CCommuModule* ms_pInstance;
    static CAoLock ms_Lock;
	typedef std::map<std::string, CAoChannel*> AC_MAP;
	typedef AC_MAP::iterator AC_MAP_IT;
	AC_MAP m_mapAC;
	enum{
		EN_MAX_IO_SELECT_GROUP_COUNT = 20
	};
	CAoIoSelectGroup* m_pAISG[EN_MAX_IO_SELECT_GROUP_COUNT];
};
#endif // CommuModule_h__