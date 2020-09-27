/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string>
#include <string.h>
#include "AoHelper.h"
#include "AoFindFile.h"
CAoFindFile::CAoFindFile(){
#ifdef WIN32
    m_hFindFile = INVALID_HANDLE_VALUE;
#endif
	memset(m_szPath, 0, EN_MAX_PATH_LEN);
}
CAoFindFile::~CAoFindFile(){Clear();}

bool CAoFindFile::FFindFirtFile(const char* pszFindPath, char* pszFilePathName, unsigned short usLen, bool& bIsDir, FILE_ATTRIBUTE* pFA){
	Clear();
#ifdef WIN32
	WIN32_FIND_DATAA findData;
	m_hFindFile = ::FindFirstFile(pszFindPath, &findData);
	if (INVALID_HANDLE_VALUE == m_hFindFile){return false;}
	bIsDir = (FILE_ATTRIBUTE_DIRECTORY == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	ToFileAttribute(findData, pFA);
	SetPath(pszFindPath);
	if(0 != strcmp(findData.cFileName, "..") && ('.' != findData.cFileName[0])){
		sprintf_s(pszFilePathName, usLen, "%s%s", m_szPath, findData.cFileName);
	}else{pszFilePathName[0] = '\0';}
#endif
	return true;
}
bool CAoFindFile::FFindNextFile(char* pszFilePathName, unsigned short usLen, bool& bIsDir, FILE_ATTRIBUTE* pFA){
#ifdef WIN32
	if (INVALID_HANDLE_VALUE == m_hFindFile){return false;}
	WIN32_FIND_DATAA findData;
	if(FALSE == ::FindNextFile(m_hFindFile, &findData)){return false;}
	bIsDir = (FILE_ATTRIBUTE_DIRECTORY == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	ToFileAttribute(findData, pFA);
	if(0 != strcmp(findData.cFileName, "..") && ('.' != findData.cFileName[0])){
		sprintf_s(pszFilePathName, usLen, "%s%s", m_szPath, findData.cFileName);
	}else{pszFilePathName[0] = '\0';}
#endif
	return true;
}
void CAoFindFile::Clear(){
#ifdef WIN32
	if (INVALID_HANDLE_VALUE != m_hFindFile ){
		FindClose(m_hFindFile);
		m_hFindFile = INVALID_HANDLE_VALUE;
	}
#endif
}
void CAoFindFile::SetPath(const char* pszFindPath){
	int nIndex1 = CAoHelper::FindLastBuf(pszFindPath, (int)strlen(pszFindPath), "\\", 1);
    int nIndex2 = CAoHelper::FindLastBuf(pszFindPath, (int)strlen(pszFindPath), "/", 1);
	nIndex2 = (nIndex2 < nIndex1) ? nIndex1 : nIndex2;
	memset(m_szPath, 0, EN_MAX_PATH_LEN);
	if(nIndex2 > 0){memcpy(m_szPath, pszFindPath, nIndex2 + 1);}}
#ifdef WIN32
void CAoFindFile::ToFileAttribute(const WIN32_FIND_DATAA& wfd, FILE_ATTRIBUTE* pFA){	
	if (NULL == pFA){return;}// ÎÄ¼þÊôÐÔ
	pFA->unFileAttribute = wfd.dwFileAttributes;
	pFA->unSizeHigh = wfd.nFileSizeHigh;
	pFA->unSizeLow = wfd.nFileSizeLow;
}
#endif