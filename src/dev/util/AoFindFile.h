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
struct FILE_ATTRIBUTE {// �����ļ�������Ϣ���ݽṹ
    unsigned int unFileAttribute; // �ļ�����
	CAoTime stCreate;     // ����ʱ��
    CAoTime stLastAccess; // �������ʱ��
    CAoTime stLastWrite;  // �������ʱ��
	unsigned int unSizeHigh; // �ļ���С ��4�ֽ�
    unsigned int unSizeLow;  // �ļ���С ��4�ֽ�
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