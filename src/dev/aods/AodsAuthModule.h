/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsAuthModule_h__
#define AodsAuthModule_h__
#include "AodsDef.h"
#include "../util/Module.h"
#include "../util/AoLock.h"
struct AODS_AODC_AUTHEN;
class CAodsAuthModule : public CModule{
private:
    CAodsAuthModule();
    virtual ~CAodsAuthModule();
public:
	static CAodsAuthModule* Instance();
	static void Release();
protected:
	void OnChannelBuild(const MT_MSG* pMM);
	void OnRecvChannelData(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void OnAuthen(const char* pszChannelName);
	bool ReadActiveData(AODS_AODC_AUTHEN* pAAA);
    bool GetIpAddrs(unsigned char& ucNum, unsigned int* punIpAddrs);
protected:
	static CAodsAuthModule* ms_pInstance;
	static CAoLock ms_Lock;
	enum{
		EN_MAX_DATA_LEN = 1024 * 1024
	};
};
#endif // AodsAuthModule_h__