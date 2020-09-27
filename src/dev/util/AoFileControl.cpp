/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <list>
#include "AoFileControl.h"
#include "AoEncrypt.h"
#include "AoHash.h"
#include "AoFile.h"
#ifdef WIN32
#include <string>
#include <direct.h>
#include <Windows.h>
#else
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <errno.h>
#include "AoProcess.h"
char CAoFileControl::ms_szAppPath[EN_MAX_PATH_LEN] = {0};
char CAoFileControl::ms_szModuleFileName[EN_MAX_PATH_LEN] = {0};
CAoFileControl::CAoFileControl(){}
CAoFileControl::~CAoFileControl(){}
int CAoFileControl::CreateDirs( const char* pszDir ){
	std::string strDir = pszDir;
	const char cWindir = '\\';
	const char cComDir = '/';
	typedef std::list<std::string> DIR_LST;
	typedef DIR_LST::iterator DIR_LST_IT;
	DIR_LST lstDir;
	int nIndex = 0;
	std::string strDirName;
	while(strDir.length() > 0 && nIndex < (int)strDir.length()){
		if(strDir[nIndex] == cWindir || strDir[nIndex] == cComDir){
			strDirName = strDir.substr(0, nIndex);
			if (strDirName == ".."){if(lstDir.size() > 0){lstDir.pop_back();}}// 后退一级目录
			else if (strDirName != "."){lstDir.push_back(strDirName);}
            strDir = strDir.substr(nIndex + 1, strDir.length() - nIndex - 1);        
			nIndex = 0;
		}
		++nIndex;
	}
    if (strDir.length() > 0){lstDir.push_back(strDir);strDir = "";}
	for (DIR_LST_IT it = lstDir.begin(); it != lstDir.end(); ++it){strDir += (*it) + "/";CreateDir(strDir.c_str());}	
	return EN_NOERROR;
}
int CAoFileControl::CreateDir( const char* pszDir, int nMode ){
#ifdef WIN32
	nMode = nMode;
	if (_mkdir(pszDir) ==0){return EN_NOERROR;}
#else
	if (mkdir(pszDir, nMode) == 0){return EN_NOERROR;}
#endif
	return errno;
}
int CAoFileControl::CalcFileCrc32_3(const char* pszFileName, unsigned int& unCrc1, unsigned int& unCrc2, unsigned int & unCrc3, unsigned int& unFileLen){
	CAoFile af;
	if(EN_NOERROR != af.Open(pszFileName, "rb")){
		return EN_ERROR_OPEN_FILE_FAIL;
	}
	af.Seek(0, EN_SEEK_END);
    unFileLen = (unsigned int)af.Tell();
    int nHalfFileLen = (int)(unFileLen / 2);
	af.Seek(0, EN_SEEK_SET);	
	int nReadLen = 0;
	int nReadedLen = 0;
	unCrc3 = unCrc2 = unCrc1 = 0;
	char szData[10240] = {0};
	while(0 < (nReadLen =af.Read(szData, 10240))){
		unCrc3 = CAoHash::CalcCrc32(unCrc3, szData, nReadLen);
		if (nReadedLen + nReadLen < nHalfFileLen){
			unCrc1 = CAoHash::CalcCrc32(unCrc1, szData, nReadLen);
		}else if (nReadedLen >= nHalfFileLen){
			unCrc2 = CAoHash::CalcCrc32(unCrc2, szData, nReadLen);
		}else{
			unCrc1 = CAoHash::CalcCrc32(unCrc1, szData, nHalfFileLen - nReadedLen);
			unCrc2 = CAoHash::CalcCrc32(unCrc2, szData + nHalfFileLen - nReadedLen, (nReadLen - (nHalfFileLen - nReadedLen)));
		}
		nReadedLen += nReadLen;
	}
	return EN_NOERROR;
}
int CAoFileControl::CalcFileCrc32Hash(const char* pszFileName, char* pszHash, int ){
	CAoFile af;
	if(EN_NOERROR != af.Open(pszFileName, "rb")){return EN_ERROR_OPEN_FILE_FAIL;}	
	af.Seek(0, EN_SEEK_END);
	unsigned int unFileLen = af.Tell();
	int nHalfFileLen = (int)unFileLen;
	af.Seek(0, EN_SEEK_SET);
	int nReadLen = 0;
	int nReadedLen = 0;
	unsigned int unCrc1 = 0;
	unsigned int unCrc2 = 0;
	unsigned int unCrc3 = 0;
	char szData[10240] = {0};
	while(0 < (nReadLen =af.Read(szData, 10240))){
		unCrc3 = CAoHash::CalcCrc32(unCrc3, szData, nReadLen);
		if (nReadedLen + nReadLen < nHalfFileLen){
			unCrc1 = CAoHash::CalcCrc32(unCrc1, szData, nReadLen);
		}else if (nReadedLen >= nHalfFileLen){
			unCrc2 = CAoHash::CalcCrc32(unCrc2, szData, nReadLen);
		}else{
			unCrc1 = CAoHash::CalcCrc32(unCrc1, szData, nHalfFileLen - nReadedLen);
			unCrc2 = CAoHash::CalcCrc32(unCrc2, szData + nHalfFileLen - nReadedLen, (nReadLen - (nHalfFileLen - nReadedLen)));
		}
		nReadedLen += nReadLen;
	}
	sprintf(pszHash, "%08x%08x%08x%08x", unFileLen, unCrc1, unCrc2, unCrc3);
	return EN_NOERROR;
}
int CAoFileControl::DelFile( const char* pszFile ){return remove(pszFile);}
int CAoFileControl::RemoveDir( const char* pszDir ){
	char szCmd[EN_MAX_PATH_LEN] = {0};
#ifdef WIN32
	char szTemp[EN_MAX_PATH_LEN] = {0};
	for (int n = 0; *pszDir != 0; ++n, ++pszDir)
	{
		szTemp[n] = (*pszDir == '/') ? '\\' : *pszDir;
	}
	sprintf(szCmd, "cmd /c \"rd /s /q \"%s\"\"", szTemp);
#else
	sprintf(szCmd, "rm -fr \"%s\"", pszDir);
#endif
	CAoProcess ap;
	return ap.RunCmd(szCmd);
}
int CAoFileControl::CopyDir( const char* pszSrc, const char* pszDest ){
	char szCmd[1024] = {0};
#ifdef WIN32
	char szSrc[EN_MAX_PATH_LEN] = {0};
	char szDest[EN_MAX_PATH_LEN] = {0};
	char* pszTemp = szSrc;
	while(*pszSrc !=0){*pszTemp = (*pszSrc == '/') ? '\\' : *pszSrc;++pszSrc;++pszTemp;}
	pszTemp = szDest;
	while(*pszDest !=0){*pszTemp = (*pszDest == '/') ? '\\' : *pszDest;	++pszDest;	++pszTemp;}
	sprintf(szCmd, "xcopy %s\\* %s\\* /E /I /Q /Y", szSrc, szDest);
#else
	sprintf(szCmd, "cp -fr \"%s/\"* \"%s/\"", pszSrc, pszDest);
#endif
	CAoProcess ap;
	return ap.RunCmd(szCmd);
}
int CAoFileControl::CutFile(const char* pszSrc, const char* pszDest){
    char szCmd[1024] = { 0 };
#ifdef WIN32
    char szSrcPath[EN_MAX_PATH_LEN] = { 0 };
    ToStandardlPath(pszSrc, szSrcPath, "\\");
    char szDestPath[EN_MAX_PATH_LEN] = { 0 };
    ToStandardlPath(pszDest, szDestPath, "\\");
    sprintf(szCmd, "move /Y \"%s\" \"%s\"", szSrcPath, szDestPath);
#else
    char szSrcPath[EN_MAX_PATH_LEN] = { 0 };
    ToStandardlPath(pszSrc, szSrcPath, "/");
    char szDestPath[EN_MAX_PATH_LEN] = { 0 };
    ToStandardlPath(pszDest, szDestPath, "/");
    sprintf(szCmd, "mv -f \"%s\" \"%s\"", szSrcPath, szDestPath);
#endif
    CAoProcess ap;
    return ap.RunCmd(szCmd);
}
void CAoFileControl::GetCwd( char* pszBuf, int nLen ){
#ifdef WIN32
	_getcwd(pszBuf, nLen);
#else
	getcwd(pszBuf, nLen);
#endif
}
void CAoFileControl::SetCwd(const char* pszCwd){
#ifdef WIN32
    _chdir(pszCwd);
#else
    chdir(pszCwd);
#endif
}
void CAoFileControl::GetCurAppPath(char* pszBuf, int){	strcpy(pszBuf, ms_szAppPath);}
void CAoFileControl::SetCurAppPath( const char* pszBuf ){	strcpy(ms_szAppPath, pszBuf);}
void CAoFileControl::GetCurModuleFileName(char* pszBuf, int ){strcpy(pszBuf, ms_szModuleFileName);}
void CAoFileControl::SetCurModuleFileName(const char* pszBuf){strcpy(ms_szModuleFileName, pszBuf);}
void CAoFileControl::Split(const char* pszPathFileName, char* pszPathName, int , char* pszFileName, int ){
    int n = (int)strlen(pszPathFileName) - 1;
    for (; pszPathFileName[n] != '/' && pszPathFileName[n] != '\\' && n >= 0; --n);
    if (n < 0){pszPathName[0] = '\0';strcpy(pszFileName, pszPathFileName);return;}    
    memcpy(pszPathName, pszPathFileName, n);
    pszPathName[n] = '\0';
    strcpy(pszFileName, pszPathFileName + n + 1);
}
void CAoFileControl::ToStandardlPath(const char* pszSrc, char* pszDest, const char* pszSplit){
    std::list<std::string> lstDirs;
    const char* pszCur = pszSrc;
    const char* pszStart = pszCur;
    while ('\0' != (*pszCur)){
        if ('\\' == *pszCur || '/' == *pszCur){
            std::string strDir(pszStart, pszCur);
            pszStart = pszCur + 1;
            if (strDir == ".."){if (lstDirs.size() > 0){lstDirs.pop_back();}
            }else if (strDir != "." && (strDir.length() > 0 || lstDirs.size() == 0)){
                lstDirs.push_back(strDir);
            }
        }
        ++pszCur;
    }
    if ('\0' != *pszStart){lstDirs.push_back(pszStart);}
    bool bFirst = true;
    std::string strDest = "";
    for (std::list<std::string>::iterator it = lstDirs.begin(); it != lstDirs.end(); ++it){
        if (bFirst){bFirst = false;strDest = (*it);}
        else{strDest += pszSplit + (*it);}
    }
    sprintf(pszDest, "%s", strDest.c_str());
}
