/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/

#ifndef AodsActiveModule_h__
#define AodsActiveModule_h__
#include "InstallDef.h"
#include "../util/Module.h"
#include "../util/AoLock.h"
struct AODS_ACTIVE_SUC;
struct AODS_AODC_ACTIVE;
class CAodsActiveModule : public CModule{
private:
    CAodsActiveModule();
    virtual ~CAodsActiveModule();
public:
	static CAodsActiveModule* Instance();
	static void Release();
protected:
	void OnChannelBuild(const MT_MSG* pMM);
	void OnRecvChannelData(const MT_MSG* pMM);
	void OnSendChannelDataFail(const MT_MSG* pMM);
	void OnSetActivePara(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void GetActiveData(AODS_AODC_ACTIVE* pAAA);
    bool GetIpAddrs(unsigned char& ucNum, unsigned int* punIpAddrs);
	void ActiveSuccess(const unsigned char* pucActData, unsigned int unCodeLen);
	bool SaveActive(const AODS_ACTIVE_SUC* pAAS);
protected:
    bool m_bActived;
	unsigned char m_ucIndex; // 
    unsigned char m_ucRDSerial[EN_RD_SERIAL_LEN];          // 硬盘序列号
    unsigned char m_ucIpNum;                               // Ip总数
    unsigned int  m_unIpAddr[EN_MAX_IP_ADDR_NUM];          // Ip地址
	static CAodsActiveModule* ms_pInstance;
	static CAoLock ms_Lock;
};
#endif // AodsActiveModule_h__