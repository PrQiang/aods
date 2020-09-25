/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef UpdateModule_h__
#define UpdateModule_h__
#include <map>
#include <string>
#include "../util/Module.h"
#include "../util/AoLock.h"
#include "../AodsAodcMsgDef.h"
class CAoMemCache;
class CUpdateModule : public CModule{
private:
    CUpdateModule();
    ~CUpdateModule();
public:
	static CUpdateModule* Instance();
	static void Release();
    virtual bool Start();
protected:
	void OnLoadCfg(const MT_MSG* pMM);
	void OnRecvChannelData(const MT_MSG* pMM);
	void OnSendChannelDataFail(const MT_MSG* pMM);
	void OnChannelFreed(const MT_MSG* pMM);
	void OnAuthSuc(const MT_MSG* pMM);
	void OnRecvFromKafka(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void OnRecvAodsData(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen);
	void OnRecvUpdateModuleInfo(const char* pszResId, const unsigned char* pucData, unsigned int unDataLen);
	void OnRecvUpdateModuleInfoResult(const char* pszChannelName, const unsigned char* pucData, unsigned int unDataLen);
	void OnRecvUpdateModuleResult(const char* pszResId, const unsigned char* pucData, unsigned int unDataLen);
    void OnRecvUpdateModule2Check(const char* pszcResId, const unsigned char* pucData, unsigned int unDataLen);
    virtual void OnTimer(unsigned int unTimerID);
protected:    enum   {
        EN_TI_KEEPALIVE = 1,
        EN_TT_KEEPALIVE = 50000,
    };
	static CUpdateModule* ms_pInstance;
	static CAoLock ms_Lock;
	struct UPDATE_OBJ{
		std::string strChannelName;
		std::string strResId;
	};
	typedef std::map<std::string, UPDATE_OBJ> CUO_MAP;
	typedef CUO_MAP::iterator CUO_MAP_IT;
	CUO_MAP m_mapCuo; // 通讯通道和更新对象之间的映射关系
	CUO_MAP m_mapRuo; // 资源号与更新对象之间的映射关系
	const char* m_pszDBAChannelName;
	unsigned int m_unDBAChannelNameLen;
	const char* m_pszAodsChannelName;
	unsigned int m_unAodsChannelNameLen;
	CAoMemCache* m_pAMC;
};
#endif // UpdateModule_h__