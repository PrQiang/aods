/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef DBACommuProtocol_h__
#define DBACommuProtocol_h__
#include "AoCommuProtocol.h"
class CDBACommuProtocol : public CAoCommuProtocol{
public:
    CDBACommuProtocol(unsigned int unCacheSize);
    virtual ~CDBACommuProtocol();
	virtual const char* GetContent(unsigned int& unDataLen, int& nIsErr);
	virtual void MoveNext();
	virtual char* EncodeMsg(const unsigned char* pucData, unsigned int unDataLen, unsigned int& unMsgLen);
};
#endif // DBACommuProtocol_h__