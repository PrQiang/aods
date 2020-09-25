/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoHelper_h__
#define AoHelper_h__
class CAoHelper{
private:
	CAoHelper();
	~CAoHelper();
public:
    static bool Utf8CvtGb2312(const char* pszSrc, unsigned int usSrcLen, char* pszDest, unsigned int& unLen);
	static bool Gb2312CvtUtf8(const char* pszSrc, unsigned int unSrcLen, char* pszDest, unsigned int& unLen);
    static int FindFirstBuf(const char* pszSrc, int nSrcLen, const char* pszFind, int nFindLen);
    static int FindLastBuf(const char* pszSrc, int nSrcLen, const char* pszFind, int nFindLen);
	static char HexChar(unsigned char ucHexData);
	static unsigned char HexData(char cHexChar);
	static void GetHexString(char* pszHex, const unsigned char* pucData, unsigned short ucLen);
	static void GetHexData(unsigned char* pucData, const char* pszHexString);
	static void GetUuid(char* pszUuid, int nUuldLen);
	static const char* Ipv4Address(unsigned int unAddr);
	static unsigned int Ipv4Address(const char* pszAddr);
	static char* Toupper(char* pszTex);
};
#endif // AoHelper_h__