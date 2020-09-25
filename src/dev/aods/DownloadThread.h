/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef DownloadThread_h__
#define DownloadThread_h__
#include <list>
#include <string>
#include "../util/AoLock.h"
#include "../util/AoEvent.h"
#include "../util/AoThread.h"
class CModule;
class CDownloadThread : public CAoThread{
public:
    CDownloadThread(CModule* pModule);
    virtual ~CDownloadThread();
	virtual int Run();
	void Stop();
	void AppendDownloadTask(const char* pszUrl, const char* pszPwd, const char* pszFileName, const char* pszPrj, const char* pszModule, const char*pszFileVer, const char* pszHash);
	int Load();
protected:
	void DecryptFile(const char* pszSrcFile, const char* pszDstFileName, const char* pszPwd);
protected:
	bool m_bRun;
	CAoEvent m_evSignal;
	int m_nLoad;
	struct DOWNLOAD_ITEM{
		std::string strUrl;
		std::string strPwd;
		std::string strSaveFileName;
		std::string strFileVer;
		std::string strFileHash;
		std::string strPrj;
		std::string strModule;
	};
	typedef std::list<DOWNLOAD_ITEM> DI_LST;
	typedef DI_LST::iterator DI_LST_IT;
	DI_LST m_lstDi;
	CAoLock m_lock;
	CModule* m_pModule;
};
#endif // DownloadThread_h__
