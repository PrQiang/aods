/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoHash_h__
#define AoHash_h__
struct SHA256_CTX{
	unsigned char ucData[64];
	unsigned int unDataLen;
	unsigned int unBitLen[2];
	unsigned int unState[8];
} ;
class CAoHash{
private:
    CAoHash();
    ~CAoHash();
public:
	static unsigned char CalcCrc8(const unsigned char* pucData, unsigned int unDataLen);
	static unsigned int CalcCrc32(unsigned int unCrc32, const char* pszBuf, int nLen);
	static unsigned int Crc32(unsigned int unCrc, char c);
	static void CalcSha256(const char* pszData, int nDataLen, char* pszSha256, int nLen);
	static void Sha256Init(SHA256_CTX* pCtx);
	static void Sha256Transform(SHA256_CTX* pCtx, unsigned char* pucData);
	static void Sha256Update(SHA256_CTX* pCtx, const unsigned char* pucData, int nLen);
	static void Sha256Final(SHA256_CTX* pCtx, unsigned char* pucHash);
    static void GetUuid(char* pszBuf, unsigned short usLen);
};
#endif // AoHash_h__