/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include "AoDef.h"
#include "AoMemCache.h"

CAoMemCache::CAoMemCache(unsigned int unInitSize, unsigned int unAppendStepSize){
	m_unCacheOff = 0;
	m_unCacheSize = unInitSize;
	m_unAppedStepSize = unAppendStepSize;
	m_pucCache = (m_unCacheSize > 0) ? (unsigned char*)MALLOC(m_unCacheSize) : NULL;	
}
CAoMemCache::~CAoMemCache(){	SAFE_FREE(m_pucCache);}
bool CAoMemCache::AppendBuf(const unsigned char* pucData, unsigned int unDataLen){
	if (unDataLen < 1)	{		return true;	}
	if (NULL == m_pucCache || m_unCacheSize < unDataLen + m_unCacheOff){
		m_unCacheSize = unDataLen + m_unCacheOff + m_unAppedStepSize;
		m_pucCache = (unsigned char*)(NULL == pucData ? MALLOC(m_unCacheSize) : REALLOC(m_pucCache, m_unCacheSize));
	}
	if (NULL == m_pucCache){return false;}
	if(NULL != pucData){memcpy(m_pucCache + m_unCacheOff, pucData, unDataLen);}	
	m_unCacheOff += unDataLen;
	return true;
}
unsigned char* CAoMemCache::CurData(){	return m_pucCache;}
unsigned int CAoMemCache::CurDataLen(){	return m_unCacheOff;}
unsigned int CAoMemCache::CacheSize(){	return m_unCacheSize;}
void CAoMemCache::Reset(){	m_unCacheOff = 0;}
void CAoMemCache::Remove(unsigned int unFrom, unsigned int unTo){
	if (m_unCacheOff > unTo){
		m_unCacheOff -= unTo;
		memcpy(m_pucCache + unFrom, m_pucCache + unTo, m_unCacheOff);
	}else{m_unCacheOff = 0;}
}