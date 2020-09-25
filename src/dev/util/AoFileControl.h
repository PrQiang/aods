/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoFileControl_h__
#define AoFileControl_h__
#include "AoDef.h"
enum{
	EN_S_IRUSR = 0x400,
	EN_S_IWUSR = 0x200,
	EN_S_IXUSR = 0x100,
	EN_S_IRWXU = (EN_S_IRUSR | EN_S_IWUSR|EN_S_IXUSR)
};
class CAoFileControl{
private:
    CAoFileControl();
    ~CAoFileControl();
public:
	static int CreateDirs(const char* pszDir);
	static int CreateDir(const char* pszDir, int nMode=EN_S_IRWXU);
    static int CalcFileCrc32_3(const char* pszFileName, unsigned int& unCrc1, unsigned int& unCrc2, unsigned int & unCrc3, unsigned int& unFileLen);
	static int CalcFileCrc32Hash(const char* pszFileName, char* pszHash, int nHahsLen);
	static int DelFile(const char* pszFile);
	static int RemoveDir(const char* pszDir);
	static int CopyDir(const char* pszSrc, const char* pszDest);
    static int CutFile(const char* pszSrc, const char* pszDest);
	static void GetCwd(char* pszBuf, int nLen);
    static void SetCwd(const char* pszCwd);
	static void GetCurAppPath(char* pszBuf, int nLen);
	static void SetCurAppPath(const char* pszBuf);
	static void GetCurModuleFileName(char* pszBuf, int nLen);
	static void SetCurModuleFileName(const char* pszBuf);
    static void Split(const char* pszPathFileName, char* pszPathName, int nPathLen, char* pszFileName, int nFileLen);
    static void ToStandardlPath(const char* pszSrc, char* pszDest, const char* pszSplit = "\\");
protected:
	static char ms_szAppPath[EN_MAX_PATH_LEN];
	static char ms_szModuleFileName[EN_MAX_PATH_LEN];
};

#endif // AoFileControl_h__