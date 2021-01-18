/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsLog_h__
#define AodsLog_h__
#include <string>
#include <map>
#include "AoLock.h"
#include "AoFile.h"
#include "AoLock.h"
#include "Module.h"
class CAoLogModule : public CModule{
private:
    CAoLogModule();
    ~CAoLogModule();
public:
	static CAoLogModule* Instance();
	static void Release();
protected:
	void LogInfo(const MT_MSG* pMM);
	void LogDebug(const MT_MSG* pMM);
	void LogError(const MT_MSG* pMM);
	void LogWarn(const MT_MSG* pMM);
	void LogData(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	void LogString(int nLogLevel, const char* pszModuleName, const char* pszSubModuleName, const char* pszLogInfo);
protected:
	static CAoLogModule* ms_pInstance;
	static CAoLock ms_Lock;
	enum{
		EN_MAX_BUF_LEN = 1024 * 1024,
		EN_LOG_LEVEL_INFO = 1,
		EN_LOG_LEVEL_DATA = 2,
		EN_LOG_LEVEL_DEBUG = 4,
		EN_LOG_LEVEL_WARN = 8,
		EN_LOG_LEVEL_ERROR = 16
	};
	struct STRUCT_LOG_INFO{
		STRUCT_LOG_INFO(){
			m_bOpened = false;
            nLogLevel = EN_LOG_LEVEL_WARN | EN_LOG_LEVEL_ERROR | EN_LOG_LEVEL_INFO;
		}
		bool m_bOpened;
		int nLogLevel;
		CAoFile af;
	};	
	typedef std::map<std::string, STRUCT_LOG_INFO*> SLI_MAP;
	typedef SLI_MAP::iterator SLI_MAP_IT;
	SLI_MAP m_mapSli;
	std::map<int, std::string> m_mapLogLevlText;
};
#endif // AodsLog_h__
