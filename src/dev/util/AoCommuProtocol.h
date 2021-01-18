/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoCommuProtocol_h__
#define AoCommuProtocol_h__
#include "AoEncrypt.h"
class CAoMemCache;
class CAoCommuProtocol{
public:
    CAoCommuProtocol(unsigned int unCacheSize);
    virtual ~CAoCommuProtocol();
	virtual void Reset();
	virtual bool AppendDecodeData(const char* pszData, unsigned int unDataLen);
	virtual const char* GetContent(unsigned int& unDataLen, int& nIsErr);
	virtual void MoveNext();
	virtual char* EncodeMsg(const unsigned char* pucData, unsigned int unDataLen, unsigned int& unMsgLen);
protected:
	void SetKey(int nKeyCode, int nKeyLen, const char* pszKeyBuf);
	void Decode(int nKeyCode, int nKeyLen, const char* pszKeyBuf, char* pszData, int nDataLen);
	void Encode(int nKeyCode, int nKeyLen, const char* pszKeyBuf, char* pszData, int nDataLen);
protected:
	enum{
		EN_STEP_APPEND_SIZE = 10240,
		EN_MAX_PROTOCOL_LEN = 1024 * 1024
	};
	CAoMemCache* m_pDecodeAmc; // 解析数据缓冲
	CAoMemCache* m_pEncodeAmc; // 加密数据缓冲
	CAoEncrypt m_ae;
};
#endif // AoCommuProtocol_h__