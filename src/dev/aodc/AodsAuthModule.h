/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsAuthModule_h__
#define AodsAuthModule_h__
#include <map>
#include <string>
#include "AodcDef.h"
#include "../util/Module.h"
#include "../util/AoLock.h"
class CAoMemCache;
struct AODS_AODC_AUTHEN;
class CAodsAuthModule : public CModule{
private:    
    CAodsAuthModule();
    virtual ~CAodsAuthModule();
public:
	static CAodsAuthModule* Instance();	// 单例对象
	static void Release();// 释放
protected:
	void OnRecvChannelData(const MT_MSG* pMM);
	void OnSendChannelDataFail(const MT_MSG* pMM);
	void OnChannelFreed(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void OnRecvAodsMsg(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen);
	void CalcActiveCode(char* pszActiveCode);
	void ReportAuthenMsg(const char* pszActiveCode, const char* pszHexResId);
	void OnRecvFromKafka(const MT_MSG* pMM);
protected:
	static CAodsAuthModule* ms_pInstance;
	static CAoLock ms_Lock;
    struct AUTH_INFO{
        unsigned char ucIpNum;
        std::string strChannelName;
        unsigned int unIpAddr[EN_MAX_IP_ADDR_NUM];
    };
    typedef std::map<std::string, AUTH_INFO> RC_MAP;
    typedef RC_MAP::iterator RC_MAP_IT;
    RC_MAP m_mapRC;
	typedef std::map<std::string, std::string> CR_MAP;
    typedef CR_MAP::iterator CR_MAP_IT;
    CR_MAP m_mapCR;
	enum{EN_MAX_DATA_LEN = 1024 * 1024};
	const char* m_pszAodsChannelName;
	unsigned int m_unAodsChannelNameLen;
	CAoMemCache* m_pAMC;
};
#endif // AodsAuthModule_h__