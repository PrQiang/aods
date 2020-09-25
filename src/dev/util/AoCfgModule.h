/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoCfg_h__
#define AoCfg_h__
#include "Module.h"
class CAoLock;
class CAoCfgModule : public CModule{
private:   
    CAoCfgModule(); // 构造函数
    ~CAoCfgModule();// 析构函数
public:	
	static CAoCfgModule* Instance();// 单例对象	
	static void Release();// 释放
protected:	
	void StartLoad(const MT_MSG* pMM);// 加载	
	void SaveCfg(const MT_MSG* pMM);// 保存
	DECLARE_AO_MSG_MAP()
private:
	static void GetConfigFile(char* pszFile, int nLen);
	static void SaveConfigBuf(char* pszBuf, int nLen);
	char* GetConfigBuf();
protected:	
	static CAoCfgModule* ms_pInstance;// 单例对象指针
	static CAoLock ms_Lock;
};
#endif // AoCfg_h__