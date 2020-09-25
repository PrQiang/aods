/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoFile_h__
#define AoFile_h__
#include <stdio.h>
#include "AoStream.h"
class CAoFile : public CAoStream
{
public:
    CAoFile();
    ~CAoFile();
	int Open(const char* pszFile, const char* pszMode, int nShFlag = EN_SH_DENYNO);
	virtual int Seek(ao_size_t astOffset, ao_size_t astSeek);
	virtual ao_size_t Tell();
	virtual ao_size_t Read(char* pszBuf, ao_size_t astLen);
	virtual ao_size_t Write(const char* pszBuf, ao_size_t astLen);
	virtual void Close();
	void Flush();
protected:
	FILE* m_pFile;
};
#endif // AoFile_h__