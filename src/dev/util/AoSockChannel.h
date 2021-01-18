/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoSockChannel_h__
#define AoSockChannel_h__
#include "AoChannel.h"
class CAoSock;
class CAoCommuProtocol;
class CAoSockChannel : public CAoChannel{
public:
    CAoSockChannel(CModule* pOb, const char* pszChannelName, const char* pszProtocol, CAoSock* pAS);
    virtual ~CAoSockChannel();
	int Socket();
	int ReadData(char* pszBuf, int nLen);
	virtual void SendMsg(const unsigned char* pucData, unsigned int unDataLen);
	virtual void OnFree();
	virtual void OnBuiled();
protected:
	CAoCommuProtocol* m_pACP;
	CAoSock* m_pAS;
};
#endif // AoSockChannel_h__