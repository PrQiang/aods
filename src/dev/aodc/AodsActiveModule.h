/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsActiveModule_h__
#define AodsActiveModule_h__
#include <string>
#include <map>
#include "../util/Module.h"
#include "../util/AoLock.h"
#include "../AodsAodcMsgDef.h"
class CAoMemCache;
class CAodsActiveModule : public CModule{
protected:
    CAodsActiveModule();
    virtual ~CAodsActiveModule();
public:
	static CAodsActiveModule* Instance();
	static void Release();
protected:
	void OnChannelBuilded(const MT_MSG* pMM);
	void OnRecvChannelData(const MT_MSG* pMM);
	void OnSendChannelDataFail(const MT_MSG* pMM);
	void OnChannelFreed(const MT_MSG* pMM);
	void OnRecvFromKafka(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void OnRecvAodsChannel(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen);
	void OnActiveFailedResult(const char* pszChannelName, unsigned char ucRlt);
	void OnActiveSucc(const char* pszChannelName, char* pszActiveCode);
	void CalcActiveCode(char* pszActiveCode);
	void ReportActiveSuc(const char* pszActiveCode, const unsigned char* ucResId);
	void RspActiveSuc(const char* pszAodsChannelName, const char* pszActiveCode, unsigned char* pucResId);
protected:
	struct ACTIVE_INFO{
		std::string strChannelName;
		char szActiveCode[EN_ACT_CODE_LEN];
	};
	typedef std::map<std::string, ACTIVE_INFO> UAI_MAP;
	typedef UAI_MAP::iterator UAI_MAP_IT;
	UAI_MAP m_mapUai;
	typedef std::map<std::string, std::string> CU_MAP;
	typedef CU_MAP::iterator CU_MAP_IT;
	CU_MAP m_mapCu;
	typedef std::map<std::string, std::string> CA_MAP;
	typedef CA_MAP::iterator CA_MAP_IT;
	CA_MAP m_mapCA;	
	static CAodsActiveModule* ms_pInstance;
	static CAoLock ms_Lock;	
	const char* m_pszAodsChannelName;
	unsigned int m_unAodsChannelNameLen;
	CAoMemCache* m_pAMC;
};
#endif // AodsActiveModule_h__