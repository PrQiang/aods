/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoMemCache_h__
#define AoMemCache_h__
class CAoMemCache{
public:
    CAoMemCache(unsigned int unInitSize, unsigned int unAppendStepSize);
    ~CAoMemCache();
	bool AppendBuf(const unsigned char* pucData, unsigned int unDataLen);
	unsigned char* CurData();
	unsigned int CurDataLen();
	unsigned int CacheSize();
	void Reset();
	void Remove(unsigned int unFrom, unsigned int unTo);
protected:
	unsigned int m_unAppedStepSize;
	unsigned int m_unCacheSize;
	unsigned int m_unCacheOff;
	unsigned char* m_pucCache;
};
#endif // AoMemCache_h__