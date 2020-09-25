/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef UpdateModuleMgr_h__
#define UpdateModuleMgr_h__
#include <list>
#include "../util/AoLock.h"
#include "../util/Module.h"
#include "../util/AoFile.h"

struct cJSON;
class CDownloadThread;
struct AODC_AODS_DPFH;
class CUpdateModuleMgr : public CModule
{
protected:
    CUpdateModuleMgr();
    ~CUpdateModuleMgr();
public:
    static CUpdateModuleMgr* Instance();
    static void Release();
protected:
    void OnAuthSuc(const MT_MSG* pMM);
    void OnChannelFreed(const MT_MSG* pMM);
    void OnRecvChannelData(const MT_MSG* pMM);
    void OnDownloadResult(const MT_MSG* pMM);
    DECLARE_AO_MSG_MAP()
protected:
    void StartUmiSyn();
    void SendUmi(const char* pszPrj, const char* pszModule, const char* pszFileVer, const char* pszFileHash);
    void OnRecvUmi(const char* pszMsg, unsigned int unMsgLen);
    void Report2Aodc(unsigned int unMsgCode, const unsigned char* pucData, unsigned int unDataLen);
    bool SaveCfg();
    void AppendDownloadTask(const char* pszUrl, const char* pszPwd, const char* pszFile, const char* pszPrj, const char* pszModule, const char* pszFileVer, const char* pszFileHash);
    void CreateFileDir(const char* pszFilePath);
    virtual void OnTimer(unsigned int unTimerID);
    char* GetJsonBufFromFile(const char* pszFileName);
protected:
    enum{
        EN_DT_NUM = 1,
        EN_TIMER_TI = 1,
        EN_TIMER_TT = 120000
    };
    struct MODULE_INFO{
        std::string strFV; // 文件版本
        std::string strFH; // 文件hash
        std::string strWD; // 文件工作目录
        std::string strCMD; // 文件命令
    };
    typedef std::map<std::string, MODULE_INFO> MM_MAP;
    typedef MM_MAP::iterator MM_MAP_IT;
    typedef std::map<std::string, MM_MAP> PM_MAP;
    typedef PM_MAP::iterator PM_MAP_IT;
    PM_MAP m_mapPM;
    CDownloadThread* m_pDT[EN_DT_NUM];
    static CUpdateModuleMgr* ms_pInstance;
    static CAoLock ms_Lock;
    const char* m_pszChannelName;
    unsigned int m_unUpdateTimes;
};
#endif // UpdateModuleMgr_h__