#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <Objbase.h>
#elif defined LINUX
#include <stdlib.h>
#endif
#include "AoHash.h"
/************************************************************************/
/* Sha256计算宏定义和数据                                               */
/************************************************************************/
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
unsigned int k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
/************************************************************************/
/* CRC32计算数据和宏定义                                                */
/************************************************************************/
const unsigned int crc_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};
const unsigned int * get_crc_table(){ return (const unsigned int *)crc_table;}
#define CRC_DO1(buf) unCrc32 = crc_table[((int)unCrc32 ^ (*buf++)) & 0xff] ^ (unCrc32 >> 8);
#define CRC_DO2(buf)  CRC_DO1(buf); CRC_DO1(buf);
#define CRC_DO4(buf)  CRC_DO2(buf); CRC_DO2(buf);
#define CRC_DO8(buf)  CRC_DO4(buf); CRC_DO4(buf);
CAoHash::CAoHash(){}
CAoHash::~CAoHash(){}
unsigned char CAoHash::CalcCrc8(const unsigned char* pucData, unsigned int unDataLen){
	unsigned char ucCrc = pucData[0];
	for (unsigned int n = 1; n < unDataLen; ++n) {ucCrc ^= pucData[n];}
	return ucCrc;
}
unsigned int CAoHash::CalcCrc32( unsigned int unCrc32, const char* pszBuf, int nLen ){
	unCrc32 = unCrc32 ^ 0xffffffffL;
	while (nLen >= 8){CRC_DO8((unsigned char*)pszBuf); 	nLen -= 8;}
	for (;nLen > 0; --nLen){CRC_DO1((unsigned char*)pszBuf);}
	return (unCrc32 ^ 0xffffffffL);
}
unsigned int CAoHash::Crc32(unsigned int unCrc, char c){return (crc_table[((int)(c)^(unCrc))&0xff]^((c)>>8));}
void CAoHash::CalcSha256( const char* pszData, int nDataLen, char* pszSha256, int  ){
	SHA256_CTX ctx;
	unsigned char ucHash[32];	
	Sha256Init(&ctx);
	Sha256Update(&ctx, (unsigned char*)pszData, nDataLen);
	Sha256Final(&ctx, ucHash);
	char s[3] = {0};
	for (int i = 0; i < 32; i++){
		sprintf(s, "%02x", ucHash[i]);
		strcat(pszSha256, s);
	}	
}
void CAoHash::Sha256Init( SHA256_CTX* pCtx ){
	pCtx->unDataLen = 0;
	pCtx->unBitLen[0] = 0;
	pCtx->unBitLen[1] = 0;
	pCtx->unState[0] = 0x6a09e667;
	pCtx->unState[1] = 0xbb67ae85;
	pCtx->unState[2] = 0x3c6ef372;
	pCtx->unState[3] = 0xa54ff53a;
	pCtx->unState[4] = 0x510e527f;
	pCtx->unState[5] = 0x9b05688c;
	pCtx->unState[6] = 0x1f83d9ab;
	pCtx->unState[7] = 0x5be0cd19;
}
void CAoHash::Sha256Transform( SHA256_CTX* pCtx, unsigned char* pucData ){
	unsigned int unValueA, unValueB, unValueC, unValueD, unValueE, unValueF, unValueG, unValueH, i, j, t1, t2, m[64];
	for (i = 0, j = 0; i < 16; ++i, j += 4){
		m[i] = (pucData[j] << 24) | (pucData[j + 1] << 16) | (pucData[j + 2] << 8) | (pucData[j + 3]);
	}
	for (; i < 64; ++i){m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];}
	unValueA = pCtx->unState[0];
	unValueB = pCtx->unState[1];
	unValueC = pCtx->unState[2];
	unValueD = pCtx->unState[3];
	unValueE = pCtx->unState[4];
	unValueF = pCtx->unState[5];
	unValueG = pCtx->unState[6];
	unValueH = pCtx->unState[7];
	for (i = 0; i < 64; ++i){
		t1 = unValueH + EP1(unValueE) + CH(unValueE, unValueF, unValueG) + k[i] + m[i];
		t2 = EP0(unValueA) + MAJ(unValueA, unValueB, unValueC);
		unValueH = unValueG;
		unValueG = unValueF;
		unValueF = unValueE;
		unValueE = unValueD + t1;
		unValueD = unValueC;
		unValueC = unValueB;
		unValueB = unValueA;
		unValueA = t1 + t2;
	}
	pCtx->unState[0] += unValueA;
	pCtx->unState[1] += unValueB;
	pCtx->unState[2] += unValueC;
	pCtx->unState[3] += unValueD;
	pCtx->unState[4] += unValueE;
	pCtx->unState[5] += unValueF;
	pCtx->unState[6] += unValueG;
	pCtx->unState[7] += unValueH;
}
void CAoHash::Sha256Update( SHA256_CTX* pCtx, const unsigned char* pucData, int nLen ){
	for (int i = 0; i < nLen; ++i){
		pCtx->ucData[pCtx->unDataLen++] = pucData[i];
		if (pCtx->unDataLen == 64){
			Sha256Transform(pCtx, pCtx->ucData);
			DBL_INT_ADD(pCtx->unBitLen[0], pCtx->unBitLen[1], 512);
			pCtx->unDataLen = 0;
		}
	}
}
void CAoHash::Sha256Final( SHA256_CTX* pCtx, unsigned char* pucHash ){
	unsigned int i = pCtx->unDataLen;
	if (pCtx->unDataLen < 56){
		pCtx->ucData[i++] = 0x80;
		while (i < 56){
			pCtx->ucData[i++] = 0x00;
		}
	}
	else{
		pCtx->ucData[i++] = 0x80;
		while (i < 64){
			pCtx->ucData[i++] = 0x00;
		}
		Sha256Transform(pCtx, pCtx->ucData);
		memset(pCtx->ucData, 0, 56);
	}
	DBL_INT_ADD(pCtx->unBitLen[0], pCtx->unBitLen[1], pCtx->unDataLen * 8);
	pCtx->ucData[63] = (unsigned char)pCtx->unBitLen[0];
	pCtx->ucData[62] = (unsigned char)(pCtx->unBitLen[0] >> 8);
	pCtx->ucData[61] = (unsigned char)(pCtx->unBitLen[0] >> 16);
	pCtx->ucData[60] = (unsigned char)(pCtx->unBitLen[0] >> 24);
	pCtx->ucData[59] = (unsigned char)pCtx->unBitLen[1];
	pCtx->ucData[58] = (unsigned char)(pCtx->unBitLen[1] >> 8);
	pCtx->ucData[57] = (unsigned char)(pCtx->unBitLen[1] >> 16);
	pCtx->ucData[56] = (unsigned char)(pCtx->unBitLen[1] >> 24);
	Sha256Transform(pCtx, pCtx->ucData);
	for (i = 0; i < 4; ++i){
		pucHash[i] = (pCtx->unState[0] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 4] = (pCtx->unState[1] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 8] = (pCtx->unState[2] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 12] = (pCtx->unState[3] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 16] = (pCtx->unState[4] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 20] = (pCtx->unState[5] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 24] = (pCtx->unState[6] >> (24 - i * 8)) & 0x000000ff;
		pucHash[i + 28] = (pCtx->unState[7] >> (24 - i * 8)) & 0x000000ff;
	}
}
void CAoHash::GetUuid(char* pszBuf, unsigned short usLen){
#ifdef WIN32    
    GUID guid;
    CoCreateGuid(&guid);
    char szUuid[128] = { 0 };
    sprintf_s(szUuid, 128, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    memcpy(pszBuf, szUuid, usLen < 128 ? usLen : 128);
#else
    int nRand = 0;
    char *p = pszBuf;
    const char *pszSrcCreate = "89ab";
    for (int n = 0; n < 16; ++n)   {
        nRand = rand() % 255;
        switch (n){
        case 6:sprintf(p, "4%x", nRand % 15);break;
        case 8:sprintf(p, "%c%x", pszSrcCreate[rand() % strlen(pszSrcCreate)], nRand % 15);break;
        default:sprintf(p, "%02x", nRand);break;
        }
        p += 2;   
    }
    *p = 0;
#endif
}
