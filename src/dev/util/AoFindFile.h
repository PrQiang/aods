/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoFindFile_h__
#define AoFindFile_h__
#ifdef WIN32
#include <Windows.h>
#endif
#include "AoDef.h"
#include "AoTime.h"
struct FILE_ATTRIBUTE {// 定义文件属性信息数据结构
    unsigned int unFileAttribute; // 文件属性
	CAoTime stCreate;     // 创建时间
    CAoTime stLastAccess; // 最近访问时间
    CAoTime stLastWrite;  // 最近更改时间
	unsigned int unSizeHigh; // 文件大小 高4字节
    unsigned int unSizeLow;  // 文件大小 低4字节
};
class CAoFindFile
{
public:
    CAoFindFile();
    ~CAoFindFile();
    bool FFindFirtFile(const char* pszFindPath, char* pszFilePathName, unsigned short usLen, bool& bIsDir, FILE_ATTRIBUTE* pFA = NULL);
    bool FFindNextFile(char* pszFilePathName, unsigned short usLen, bool& bIsDir, FILE_ATTRIBUTE* pFA = NULL);
protected:
	void SetPath(const char* pszFindPath);
#ifdef WIN32
    void ToFileAttribute(const WIN32_FIND_DATAA& wfd, FILE_ATTRIBUTE* pFA);
#endif
protected:
	void Clear();
protected:
	char m_szPath[EN_MAX_PATH_LEN];
#ifdef WIN32
	HANDLE m_hFindFile;
#endif
};
#endif // AoFindFile_h__