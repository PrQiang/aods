/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "AoDef.h"
#include "AoTime.h"
#include "AoEncrypt.h"
#include "AoMemCache.h"
#include "AoCommuProtocol.h"
CAoCommuProtocol::CAoCommuProtocol(unsigned int unCacheSize){
	m_pDecodeAmc = new CAoMemCache(unCacheSize, EN_STEP_APPEND_SIZE);
	m_pEncodeAmc = new CAoMemCache(unCacheSize, EN_STEP_APPEND_SIZE);
	unsigned char ucKey[16] = { 0 };
	for (unsigned char n = 0; n < 16; ++n) {ucKey[n] = (unsigned char)0xBC + n;}
	m_ae.SetKey(ucKey, 16);
}
CAoCommuProtocol::~CAoCommuProtocol(){
	SAFE_DELETE(m_pDecodeAmc);
	SAFE_DELETE(m_pEncodeAmc);
}
void CAoCommuProtocol::Reset(){
	m_pDecodeAmc->Reset();
	m_pEncodeAmc->Reset();
}
bool CAoCommuProtocol::AppendDecodeData(const char* pszData, unsigned int unDataLen){
	return m_pDecodeAmc->AppendBuf((unsigned char*)pszData, unDataLen);
}
const char* CAoCommuProtocol::GetContent(unsigned int& unDataLen, int& nIsErr){
	nIsErr = 0;
	if ( m_pDecodeAmc->CurDataLen() < EN_AO_MSG_SIZE ){	return NULL;}
	AO_MSG* pAM = (AO_MSG*)m_pDecodeAmc->CurData();
	if (EN_MAX_PROTOCOL_LEN < pAM->nDataLen ||pAM->nDataLen < pAM->nKeyLen + EN_AO_MSG_SIZE){nIsErr = 1;return NULL;} // 数据内容格式有误
	if (pAM->nDataLen > (int)m_pDecodeAmc->CurDataLen()){return NULL;}// 当前缓冲区中的数据长度不足一包
	char* pszData = pAM->szKeyData + pAM->nKeyLen;
	unDataLen = (unsigned int)(pAM->nDataLen - EN_AO_MSG_SIZE - pAM->nKeyLen);
	Decode(pAM->nKeyCode, pAM->nKeyLen, pAM->szKeyData, pszData, (int)unDataLen);
	return pszData;
}
void CAoCommuProtocol::MoveNext(){AO_MSG* pAM = (AO_MSG*)m_pDecodeAmc->CurData();m_pDecodeAmc->Remove(0, pAM->nDataLen);}
char* CAoCommuProtocol::EncodeMsg(const unsigned char* pucData, unsigned int unDataLen, unsigned int& unMsgLen){
	int nKeyLen = (unDataLen > 20) ? 20 : unDataLen;
	unMsgLen = unDataLen + EN_AO_MSG_SIZE + nKeyLen;
	m_pEncodeAmc->Reset();m_pEncodeAmc->AppendBuf(NULL, unMsgLen);
	if (NULL == m_pEncodeAmc->CurData()){return NULL;}
	// 构造协议数据
	AO_MSG* pAM = (AO_MSG*)m_pEncodeAmc->CurData();
	int nRandStart = 0x3f;	int nRandStop = 0xaf;
	srand(CAoTime::CurrentTick());
	pAM->nDataLen = unDataLen + EN_AO_MSG_SIZE + nKeyLen;
	pAM->nKeyLen = nKeyLen;
	pAM->nKeyCode = (rand()%0x20) + nRandStop;
	for (int n = 0; n < nKeyLen; ++n)	{
		pAM->szKeyData[n] = (char)((rand()%(nRandStop - nRandStart)) + nRandStart);
	}
	memcpy(pAM->szKeyData + nKeyLen, pucData, unDataLen);
	Encode(pAM->nKeyCode, nKeyLen, pAM->szKeyData, pAM->szKeyData + nKeyLen, unDataLen);
	return (char*)m_pEncodeAmc->CurData();
}
void CAoCommuProtocol::SetKey(int nKeyCode, int nKeyLen, const char* pszKeyBuf) {
	unsigned char ucKey[16] = { 0 };
	for (unsigned char n = 0; n < (unsigned char)16; ++n) { ucKey[n] = (unsigned char)nKeyCode + (unsigned char)pszKeyBuf[n % nKeyLen]; }
	m_ae.SetKey(ucKey, 16);
}
void CAoCommuProtocol::Decode(int nKeyCode, int nKeyLen, const char* pszKeyBuf, char* pszData, int nDataLen){
	if (nDataLen < 1 || nKeyCode == 0) {return;}// 无解密数据或者未加密数据
	SetKey(nKeyCode, nKeyLen, pszKeyBuf);
	m_ae.Decode((unsigned char*)pszData, nDataLen);
}
void CAoCommuProtocol::Encode(int nKeyCode, int nKeyLen, const char* pszKeyBuf, char* pszData, int nDataLen){
	if (nDataLen < 1 || nKeyCode == 0){return;} // 无解密数据或者未加密数据
	SetKey(nKeyCode, nKeyLen, pszKeyBuf);
	m_ae.Encode((unsigned char*)pszData, nDataLen);
}
