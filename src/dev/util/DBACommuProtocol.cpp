/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include "AoDef.h"
#include "AoMemCache.h"
#include "DBACommuProtocol.h"

CDBACommuProtocol::CDBACommuProtocol(unsigned int unCacheSize):CAoCommuProtocol(unCacheSize){}
CDBACommuProtocol::~CDBACommuProtocol(){}
const char* CDBACommuProtocol::GetContent(unsigned int& unDataLen, int& nIsErr){
	nIsErr = 0;
	if ( m_pDecodeAmc->CurDataLen() < EN_MSG_HEAD_SIZE ){		return NULL;	}
	MSG_HEAD* pMH = (MSG_HEAD*)m_pDecodeAmc->CurData();
	if (pMH->unLen + EN_MSG_HEAD_SIZE > m_pDecodeAmc->CurDataLen()){return NULL;}// 数据内容不足一包
	unDataLen = pMH->unLen + EN_MSG_HEAD_SIZE;
	return (char*)m_pDecodeAmc->CurData();
}
void CDBACommuProtocol::MoveNext(){
	MSG_HEAD* pMH = (MSG_HEAD*)m_pDecodeAmc->CurData();
	m_pDecodeAmc->Remove(0, pMH->unLen + EN_MSG_HEAD_SIZE);
}
char* CDBACommuProtocol::EncodeMsg(const unsigned char* pucData, unsigned int unDataLen, unsigned int& unMsgLen){
	m_pEncodeAmc->Reset();
	m_pEncodeAmc->AppendBuf(pucData, unDataLen);
	unMsgLen = unDataLen;
	return (char*)m_pEncodeAmc->CurData();
}