/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoMemStream_h__
#define AoMemStream_h__
#include "AoStream.h"
class CAoMemStream : public CAoStream{
public:
    CAoMemStream();
    ~CAoMemStream();
	int Open(const char* pszBuf, ao_size_t astLen, bool bDeepCopy);
	virtual int Seek(ao_size_t astOffset, ao_size_t astSeek);
	virtual ao_size_t Tell();
	virtual ao_size_t Read(char* pszBuf, ao_size_t nLen);
	virtual ao_size_t Write(const char* pszBuf, ao_size_t astLen);
	virtual void Close();
protected:
	ao_size_t m_astPos;
	ao_size_t m_astLen;
	char* m_pszBuf;
	bool m_bDeepCopy;
};
#endif // AoMemStream_h__