/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoUnzip_h__
#define AoUnzip_h__
#include "AoDef.h"
class CAoStream;
class CAoUnzip{
public:    
    CAoUnzip();// 构造函数    
    ~CAoUnzip();// 析构函数
	int OpenZipFile(const char* pszFileName);
    int GotoFirstFile();
    int GotoNextFile();
    int GetCurrentFileInfo(char* pszFileName, int nFileNameLen, bool& bIsDir);
    int UnzipCurentFile(CAoStream* pAS, const char* pszPwd);
protected:
	void* m_pzf;
};

#endif // AoUnzip_h__