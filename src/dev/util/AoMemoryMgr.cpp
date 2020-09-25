/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include "AoLock.h"
#include "AoMemoryMgr.h"
CAoMemoryMgr::CAoMemoryMgr(){
	int nSize[] = {
		EN_MEM_SIZE_0512, 
		EN_MEM_SIZE_1024, 
		EN_MEM_SIZE_2048,
		EN_MEM_SIZE_4096,
		EN_MEM_SIZE_8192,
		EN_MEM_SIZE_16KB,
		EN_MEM_SIZE_32KB,
		EN_MEM_SIZE_64KB,
		EN_MEM_SIZE_128K,
		EN_MEM_SIZE_01MB,
		EN_MEM_SIZE_02MB,
		EN_MEM_SIZE_08MB,
	};
	int nCount[] = {
		EN_MEM_COUNT_0512, 
		EN_MEM_COUNT_1024, 
		EN_MEM_COUNT_2048,
		EN_MEM_COUNT_4096,
		EN_MEM_COUNT_8192,
		EN_MEM_COUNT_16KB,
		EN_MEM_COUNT_32KB,
		EN_MEM_COUNT_64KB,
		EN_MEM_COUNT_128K,
		EN_MEM_COUNT_01MB,
		EN_MEM_COUNT_02MB,
		EN_MEM_COUNT_08MB,
	};
	char* pszMalloc = NULL;
	for (int n = 0; n < EN_MEM_INDEX_COUN; ++n){
		for (int m = 0; m < nCount[n]; ++m){
			pszMalloc = (char*)malloc(nSize[n] + 1);
			memset(pszMalloc, 0, nSize[n] + 1);
			pszMalloc[0] = (char)n;
			m_lstMem[n].push_back(pszMalloc);
		}
		m_nSize[n] = nSize[n];
	}
	m_pLock = new CAoLock();
}
CAoMemoryMgr::~CAoMemoryMgr(){
	for (int n = 0; n < EN_MEM_INDEX_COUN; ++n){
		for (MEM_LST_IT it = m_lstMem[n].begin(); it != m_lstMem[n].end(); ++it){
			free((*it));
		}
		m_lstMem[n].clear();
	}
	delete m_pLock;
	m_pLock = NULL;
}
void* CAoMemoryMgr::Malloc(int nSize){
	for (int n = 0; n < EN_MEM_INDEX_COUN; ++n)	{
		if (nSize > m_nSize[n]){continue;}
		m_pLock->Lock();
		if(m_lstMem[n].size() > 0){
			char* pszBuf = m_lstMem[n].front();
			m_lstMem[n].pop_front();
#ifdef _DEBUG
			m_lstUsedMem[n].push_back(pszBuf);
#endif
			m_pLock->Unlock();			
			return (void*)(pszBuf + 1);
		}
		char* pszBuf = (char*)malloc(m_nSize[n] + 1);
		memset(pszBuf, 0, m_nSize[n]);
		pszBuf[0] = (char)n;
		return (void*)(pszBuf + 1);
	}

	return NULL;
}
void CAoMemoryMgr::Free(void* pData){
	if (NULL == pData){return;}
	char* pszData = ((char*)pData) - 1;
	int nIndex = (int)pszData[0];
	if (nIndex < 0 || nIndex >= EN_MEM_INDEX_COUN){return;}
	m_pLock->Lock();
	m_lstMem[nIndex].push_back(pszData);
#ifdef _DEBUG
	for (MEM_LST_IT it = m_lstUsedMem[nIndex].begin(); it != m_lstUsedMem[nIndex].end(); ++it){
		if (pszData == (*it)){m_lstUsedMem[nIndex].erase(it);break;	}
	}
#endif
	m_pLock->Unlock();
}
void* CAoMemoryMgr::Realloc(void* pData, int nNewSize){
	if (NULL == pData){return Malloc(nNewSize);}
	char* pszData = ((char*)pData) - 1;
	int nIndex = (int)pszData[0];
	if (nIndex < 0 || nIndex >= EN_MEM_INDEX_COUN){return NULL;}	
	if (nNewSize <= m_nSize[nIndex]){return pData;}
	void* pData1 = Malloc(nNewSize);
	memcpy(pData1, pData, m_nSize[nIndex]);
	Free(pData);
	return pData1;
}
CAoMemoryMgr* CAoMemoryMgr::Instance(){static CAoMemoryMgr instance;return &instance;}
