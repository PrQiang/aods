/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef ModuleDispatcher_h__
#define ModuleDispatcher_h__
#include <map>
#include "AoLock.h"
// 类型引用声明
class CModule;
struct MT_MSG_HEAD;
class CModuleDispatcher{
protected:
    CModuleDispatcher();
    ~CModuleDispatcher();
public:
    static CModuleDispatcher* Instance();
    static void Release();
	void AppendModule(CModule* pModule);
	void AppendMonitorModule(CModule* pModule);
	bool Dispatch(unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData, unsigned int unLen, CMsgTarget* pRecver, CMsgTarget* pSender, unsigned int unSMI);
protected:
    static CModuleDispatcher* ms_pInstance;
    static CAoLock ms_Lock;
	typedef std::map<unsigned int, CModule*> IM_MAP;
	typedef IM_MAP::iterator IM_MAP_IT;
	IM_MAP m_mapIM;      // 业务模块
	IM_MAP m_mapMIM;  // 监视模块
};
#endif // ModuleDispatcher_h__