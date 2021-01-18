/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoFile.h"
#include <errno.h>
CAoFile::CAoFile(){	m_pFile = NULL;}
CAoFile::~CAoFile(){Close();}
int CAoFile::Open( const char* pszFile, const char* pszMode, int nShFlag){
#ifdef WIN32
	m_pFile = _fsopen( pszFile, pszMode, nShFlag);
#else
	m_pFile = fopen(pszFile, pszMode);	
#endif
	return (NULL == m_pFile)?errno : EN_NOERROR;
}
int CAoFile::Seek( ao_size_t astOffset, ao_size_t astSeek ){
	if (NULL == m_pFile){return EN_ERROR_INVALID_PARA;}
	return 0 == fseek(m_pFile, astOffset, astSeek) ? EN_NOERROR : EN_ERROR_INVALID_PARA;
}
ao_size_t CAoFile::Tell(){
	if (NULL == m_pFile){return (ao_size_t)-1;}
	return ftell(m_pFile);
}
ao_size_t CAoFile::Read( char* pszBuf, ao_size_t astLen ){
	if (NULL == m_pFile){return -1;}
    return (ao_size_t)fread(pszBuf, 1, astLen, m_pFile);
}
ao_size_t CAoFile::Write( const char* pszBuf, ao_size_t astLen ){
	if (NULL == m_pFile){return -1;}
    return (ao_size_t)fwrite(pszBuf, 1, astLen, m_pFile);
}
void CAoFile::Close(){
	if (NULL != m_pFile){fclose(m_pFile);m_pFile = NULL;}
}
void CAoFile::Flush(){
	if (NULL != m_pFile){fflush(m_pFile);}
}
