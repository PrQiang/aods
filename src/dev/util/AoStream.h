/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoStream_h__
#define AoStream_h__
#include "AoDef.h"
class CAoStream{
public:
    CAoStream();
    virtual ~CAoStream();
	virtual int Seek(ao_size_t astOffset, ao_size_t astSeek);
	virtual ao_size_t Tell();
	virtual ao_size_t Read(char* pszBuf, ao_size_t astLen);
	virtual ao_size_t Write(const char* pszBuf, ao_size_t astLen);
	virtual void Close();
};
#endif // AoStream_h__
