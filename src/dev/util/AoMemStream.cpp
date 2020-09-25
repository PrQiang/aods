/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "AoMemStream.h"
CAoMemStream::CAoMemStream(){	m_astPos = 0;	m_astLen = 0;	m_pszBuf = NULL;	m_bDeepCopy = false;}
CAoMemStream::~CAoMemStream(){
	if (m_bDeepCopy && NULL != m_pszBuf){
		free(m_pszBuf);
		m_pszBuf = NULL;
	}
}
int CAoMemStream::Open( const char* pszBuf, ao_size_t astLen, bool bDeepCopy ){
	m_astPos = 0;
	m_astLen = astLen;
	m_bDeepCopy = bDeepCopy;
	if (bDeepCopy){
		m_pszBuf = (char*)MALLOC(astLen);
		if (NULL == m_pszBuf){return 1;	}
		memcpy(m_pszBuf, pszBuf, astLen);
	}else{	m_pszBuf = const_cast<char*>(pszBuf);}
	return 0;
}
int CAoMemStream::Seek( ao_size_t astOffset, ao_size_t astSeek ){
	ao_size_t tPos = m_astPos;
	switch(astSeek){
		case EN_SEEK_SET:tPos = astOffset;break;
		case EN_SEEK_CUR:tPos += astOffset;break;
		case EN_SEEK_END:tPos = m_astLen + astOffset;break;
		default:return EN_ERROR_INVALID_PARA;
	}
	if (tPos >= 0 && tPos < m_astLen){	m_astLen = tPos;return EN_NOERROR;}
	return EN_ERROR_INVALID_PARA;
}
ao_size_t CAoMemStream::Tell(){	return m_astPos;}
ao_size_t CAoMemStream::Read( char* pszBuf, ao_size_t astLen ){
	ao_size_t astReadLen = m_astLen - m_astPos;
	astReadLen = (astReadLen < astLen) ? astReadLen : astLen;
	if (astReadLen > 0){
		memcpy(pszBuf, m_pszBuf + m_astPos, astReadLen);
	}
	return astReadLen;
}
ao_size_t CAoMemStream::Write( const char* pszBuf, ao_size_t astLen ){
	ao_size_t astWriteLen = m_astLen - m_astPos;
	astWriteLen = (astWriteLen < astLen) ? astWriteLen : astLen;
	if (astWriteLen > 0){memcpy(m_pszBuf + m_astPos, pszBuf, astWriteLen);}
	return astWriteLen;
}
void CAoMemStream::Close(){}