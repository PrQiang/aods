/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include "AoList.h"
#include "AoDef.h"
CAoList::CAoList(){
	m_nNum = 0;
	m_pEnd = m_pHead = NULL;
}
CAoList::~CAoList(){Clear();}
void CAoList::push_back(void* pData){
	++m_nNum;
	if (NULL == m_pHead){
		m_pEnd = m_pHead = new Node;
		m_pEnd->pNext = NULL;
		m_pEnd->pPre = NULL;
		m_pEnd->pData = pData;
		return;
	}
	Node* pNode = new Node();
	pNode->pPre = m_pEnd;
	m_pEnd->pNext = pNode;
	pNode->pNext = NULL;
	pNode->pData = pData;
	m_pEnd = pNode;
}
void CAoList::push_front(void* pData){
	++m_nNum;
	if (NULL == m_pHead){
		m_pEnd = m_pHead = new Node;
		m_pEnd->pNext = NULL;
		m_pEnd->pPre = NULL;
		m_pEnd->pData = pData;
		return;
	}
	Node* pNode = new Node();
	pNode->pPre = NULL;
	m_pHead->pPre = pNode;
	pNode->pNext = m_pHead;
	pNode->pData = pData;
	m_pHead = pNode;
}
Node* CAoList::GetNext(Node* pNode){return (NULL == pNode) ? m_pHead : pNode->pNext;}
void CAoList::Remove(Node* pNode){
	if(NULL == pNode || m_nNum < 1){return;}
	--m_nNum;
	if (NULL != pNode->pPre){pNode->pPre->pNext = pNode->pNext;}
	if (NULL != pNode->pNext){pNode->pNext->pPre = pNode->pPre;}
	m_pHead = (m_pHead == pNode) ? pNode->pNext : m_pHead;
	m_pEnd  = (m_pEnd == pNode) ? pNode->pPre : m_pEnd;
	SAFE_DELETE(pNode);
}
void CAoList::Clear(){
	Node* pNext = NULL;
	for(Node* pNode = m_pHead; NULL != pNode; pNode = pNext){
		pNext = pNode->pNext;
		SAFE_DELETE(pNode);
	}
	m_pEnd = m_pHead = NULL;
}
int CAoList::Size(){return m_nNum;}
