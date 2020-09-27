/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoChannel_h__
#define AoChannel_h__
#include <string>
#include "AoThread.h"
struct cJSON;
class CModule;
class CAoChannel : public CAoThread{
public:
	CAoChannel(CModule* pOb, const char* pszName);
    virtual ~CAoChannel();
	virtual bool Init(cJSON* pJsonPara);
	virtual int Run();
	virtual void SendMsg(const unsigned char* pucData, unsigned int unDataLen);
	virtual void Stop();
	virtual int Handle();
	virtual void OnFree();
	virtual void OnNew();
	virtual void OnDel();
	virtual void OnBuiled();
	virtual void OnFreed();
protected:
	void OnRecvedData(const unsigned char* pucData, unsigned int unDataLen);
	void OnSendDataFailed(const unsigned char* pucData, unsigned int unDataLen);
	void SendOutMsg(unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData = NULL, unsigned int unLen = 0);
protected:
	CModule* m_pOb;
	unsigned char* m_pucTempBuf;
	unsigned char* m_pucOffsetBuf;
	unsigned int m_unFixedOffLen;
	unsigned int m_unTempBufLen;
};
#endif // AoChannel_h__
