/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifdef WIN32
#include <atlconv.h>
#include <WinSock2.h>
#else
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#endif
#include <string.h>
#include <stdio.h>
#include "AoDef.h"
#include "AoTime.h"
#include "AoHelper.h"
CAoHelper::CAoHelper(){}
CAoHelper::~CAoHelper(){}
bool CAoHelper::Utf8CvtGb2312(const char* pszSrc, unsigned int unSrcLen, char* pszDest, unsigned int& unLen){   
#ifdef WIN32
    unsigned int unRetLen = (unsigned int)MultiByteToWideChar(CP_UTF8, 0, pszSrc, unSrcLen, NULL, 0);
	WCHAR* pChar = new WCHAR[unRetLen + 1];
	unRetLen = MultiByteToWideChar(CP_UTF8, 0, pszSrc, unSrcLen, pChar,  unRetLen);
	pChar[unRetLen] = 0;
	unRetLen = WideCharToMultiByte(936, 0, pChar, -1, NULL, 0, NULL, NULL);
	if (unLen >= unRetLen)	{
		unLen = WideCharToMultiByte(936, 0, pChar, -1, pszDest, unLen, NULL, NULL);
		delete []pChar;return true;
	}
	delete []pChar;
	return false;
#elif defined LINUX
    unLen = unSrcLen;
    memcpy(pszDest, pszSrc, unSrcLen);
    pszDest[unSrcLen] = '\0';
    return true;
#endif
}
bool CAoHelper::Gb2312CvtUtf8(const char* pszSrc, unsigned int unSrcLen, char* pszDest, unsigned int& unLen){
#ifdef WIN32
	unsigned int unWLen = (unsigned int)MultiByteToWideChar(CP_ACP, 0, pszSrc, unSrcLen, NULL, 0);
	WCHAR* pwszSrc = new WCHAR[unWLen + 6];
	MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pszSrc, unSrcLen, pwszSrc, unWLen);
	unWLen = (unsigned int)WideCharToMultiByte(CP_UTF8, 0, pwszSrc, unWLen, NULL, 0, NULL, NULL);
	if(unLen > unWLen){
		WideCharToMultiByte(CP_UTF8, 0, pwszSrc, unWLen, pszDest, unWLen, NULL, NULL);
		unLen = unWLen;
		delete []pwszSrc;
		return true;
	}
	delete []pwszSrc;
#else
#endif
	return false;
}
int CAoHelper::FindFirstBuf(const char* pszSrc, int nSrcLen, const char* pszFind, int nFindLen){    
    if (NULL == pszFind || 0 == nFindLen){return -2;}// 入参非法
    for (int n = 0; n < nSrcLen - nFindLen + 1; ++n){if (memcmp(pszSrc + n, pszFind, nFindLen) == 0){return n;}}
    return -1;
}
int CAoHelper::FindLastBuf(const char* pszSrc, int nSrcLen, const char* pszFind, int nFindLen){
    for (int n = nSrcLen - nFindLen; n >= 0; --n){if (memcmp(pszSrc + n, pszFind, nFindLen) == 0){return n;}}
    return -1;
}
char CAoHelper::HexChar(unsigned char ucHexData){
	if (ucHexData <= 9){return (char)(ucHexData + '0');}
	else if (ucHexData < 16){return (char)(ucHexData - 10 + 'a');}
	return '0';
}
unsigned char CAoHelper::HexData(char cHexChar){
	if (cHexChar >= '0' && (cHexChar <= '9')){return (cHexChar - '0');}
	else if (cHexChar >= 'a' && cHexChar <= 'f'){return (cHexChar - 'a' + 10);}
	else if (cHexChar >= 'A' && cHexChar <= 'F'){return (cHexChar - 'A' + 10);}
	return 0;
}
void CAoHelper::GetHexString(char* pszHex, const unsigned char* pucData, unsigned short ucLen){
	unsigned short n = 0;
	for (; n < ucLen; ++n){
		pszHex[2 * n] = HexChar((pucData[n] >> 4) & 0xF);
		pszHex[2 * n + 1] = HexChar(pucData[n] & 0xF);
	}
	pszHex[2 * n] = '\0';
}
void CAoHelper::GetHexData(unsigned char* pucData, const char* pszHexString){
	unsigned short nLen = (unsigned short)strlen(pszHexString);
	if ((nLen % 2) != 0){return;}
	for (unsigned short n = 0; n < nLen / 2; ++n){
		pucData[n] = (HexData(pszHexString[2 * n]) << 4) + HexData(pszHexString[2 * n + 1]);
	}
}
void CAoHelper::GetUuid(char* pszUuid, int nUuidLen){
	char szUuid[EN_AO_UUID_LEN + 1] = {0};
#ifdef WIN32
	GUID guid;CoCreateGuid(&guid);	
	sprintf_s(szUuid, EN_AO_UUID_LEN + 1, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#else
	srand(CAoTime::CurrentTick());
	for (int n = 0; n < 16; ++n){sprintf(szUuid + 2 * n, "%02X", (unsigned char)rand());}
#endif
	memcpy(pszUuid, szUuid, nUuidLen < EN_AO_UUID_LEN ? nUuidLen : EN_AO_UUID_LEN);
}
const char* CAoHelper::Ipv4Address(unsigned int unAddr){
	sockaddr_in saddr;saddr.sin_addr.s_addr = unAddr;return ::inet_ntoa(saddr.sin_addr);
}
unsigned int CAoHelper::Ipv4Address(const char* pszAddr){return ::inet_addr(pszAddr);}
char* CAoHelper::Toupper(char* pszText){
	char* p = pszText;
	while(*p != '\0'){
		if (islower(*p)){*p = (char)toupper(*p);}
		++p;
	}
	return pszText;
}
