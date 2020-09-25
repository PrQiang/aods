/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoList_h__
#define AoList_h__
struct Node{
	void* pData;
	Node* pPre;
	Node* pNext;
};
class CAoList{
public:
    CAoList();
    ~CAoList();
	void push_back(void* pData);
	void push_front(void* pData);
	Node* GetNext(Node* pNode);
	void Remove(Node* pNode);
	void Clear();
	int Size();
protected:
	Node* m_pHead;
	Node* m_pEnd;
	int m_nNum;
};

#endif // AoList_h__