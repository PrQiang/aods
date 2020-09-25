/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AodsDef.h"
#include "UpdateModuleMgr.h"
#include "DownloadThread.h"
#include "../util/CJson.h"
#include "../util/AoTime.h"
#include "../util/AoUnzip.h"
#include "../util/AoProcess.h"
#include "../util/AoFileControl.h"
#ifdef WIN32
#else
#include <string.h>
#include <stdlib.h>
#endif
CUpdateModuleMgr* CUpdateModuleMgr::ms_pInstance = NULL;
CAoLock CUpdateModuleMgr::ms_Lock;
BEGIN_AO_MSG_MAP(CUpdateModuleMgr, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_AUTHEN_SUC, &CUpdateModuleMgr::OnAuthSuc)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_FREED, &CUpdateModuleMgr::OnChannelFreed)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CUpdateModuleMgr::OnRecvChannelData)
    ON_AO_MSG_FUN(EN_AO_MSG_DOWNLOAD_RESULT, &CUpdateModuleMgr::OnDownloadResult)
END_AO_MSG_MAP()
CUpdateModuleMgr::CUpdateModuleMgr()
    : CModule(EN_MODULE_ID_UPDATE, 1024 * 1024, "UpdateModule"){
    m_pszChannelName = "AodcClient";
    for (int n = 0; n < EN_DT_NUM; ++n){
        m_pDT[n] = new CDownloadThread(this);
        m_pDT[n]->Start();
    }
    m_unUpdateTimes = 0;
}
CUpdateModuleMgr::~CUpdateModuleMgr(){for (int n = 0; n < EN_DT_NUM; ++n) {SAFE_DELETE(m_pDT[n]);}}
CUpdateModuleMgr* CUpdateModuleMgr::Instance(){
    if(NULL == ms_pInstance){
         ms_Lock.Lock();
         ms_pInstance = (NULL == ms_pInstance) ? new CUpdateModuleMgr : ms_pInstance;
         ms_Lock.Unlock();
    }
    return ms_pInstance;
}
void CUpdateModuleMgr::Release(){SAFE_DELETE(ms_pInstance);}
void CUpdateModuleMgr::OnAuthSuc(const MT_MSG* ){StartUmiSyn();SetTimer(EN_TIMER_TI, EN_TIMER_TT, 0xFFFFFFFF, this);}
void CUpdateModuleMgr::OnChannelFreed(const MT_MSG* pMM){
    if (strcmp(m_pszChannelName, (const char*)pMM->ucData) != 0){return;}
    KillTimer(EN_TIMER_TI, this);
}
void CUpdateModuleMgr::OnRecvChannelData(const MT_MSG* pMM){
    unsigned int unChannelNameLen = (unsigned int)strlen(m_pszChannelName) + 1;
    if ((0 != strcmp(m_pszChannelName, (const char*)pMM->ucData)) || (pMM->unDataLen < unChannelNameLen + EN_AODS_AODC_MSG_SIZE)){return;}    
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + unChannelNameLen);
    unsigned int unDataLen = pMM->unDataLen - unChannelNameLen - EN_AODS_AODC_MSG_SIZE;
    switch(pAAM->unMsgCode){
    case EN_AODC_AODS_UMI:OnRecvUmi(pAAM->szMsg, unDataLen);break;// 更新模块确认信息
    default:break;
    }
}
void CUpdateModuleMgr::OnDownloadResult(const MT_MSG* pMM){
    DOWNLOAD_RESULT* pDR = (DOWNLOAD_RESULT*)pMM->ucData; // 开始下载
    const char* pszPrj = pDR->szBuf; // 项目名称
    const char* pszModule = pszPrj + strlen(pszPrj) + 1; // 模块名称
    const char* pszFileVer = pszModule + strlen(pszModule) + 1; // 文件版本
    const char* pszHash = pszFileVer + strlen(pszFileVer) + 1; // 文件HASH
    const char* pszUrl = pszHash + strlen(pszHash) + 1; // 下载地址
    const char* pszSaveFile = pszUrl + strlen(pszUrl) + 1; // 保存文件
    if (EN_DR_SUC != pDR->ucRlt){
        Report2Aodc(EN_AODS_AODC_UDR, pMM->ucData, pMM->unDataLen); // 上报下载结果
        LogErr("CUpdateModuleMgr::OnDownloadResult", "更新%s/%s至%s失败， 错误代码: %d", pszPrj, pszModule, pszFileVer, pDR->ucRlt);
        return;
    }
    PM_MAP_IT it = m_mapPM.find(pszPrj);
    if(m_mapPM.end() == it){return;}
    MM_MAP_IT itMm = it->second.find(pszModule); // 模块名称
    if (it->second.end() == itMm){return;}
    {
        CAoUnzip au;
        if (EN_NOERROR != au.OpenZipFile(pszSaveFile)){return;}
        bool bIsDir;
        char szRelativeFile[EN_MAX_PATH_LEN] = { 0 };
        char szFile[EN_MAX_PATH_LEN] = { 0 };
        char szStandFile[EN_MAX_PATH_LEN] = { 0 };
        char szBakFile[EN_MAX_PATH_LEN] = { 0 };
        char szDir[EN_MAX_PATH_LEN] = { 0 };
        CAoFileControl::GetCurAppPath(szFile, EN_MAX_PATH_LEN);
#ifdef WIN32
        if (itMm->second.strWD.length() < 2 || itMm->second.strWD.at(1) != ':') { // 相对路径
#else
        if (itMm->second.strWD.length() < 1 || itMm->second.strWD.at(0) != '/') { // 相对路径
#endif
            sprintf(szDir, "%s/%s", szFile, itMm->second.strWD.c_str());
        }else{ sprintf(szDir, "%s", itMm->second.strWD.c_str());}
        au.GotoFirstFile();
        do {
            if (0 != au.GetCurrentFileInfo(szRelativeFile, EN_MAX_PATH_LEN, bIsDir)) {break;}
            sprintf(szFile, "%s/%s", szDir, szRelativeFile);
#ifdef WIN32
            CAoFileControl::ToStandardlPath(szFile, szStandFile, "\\");
#else      
            CAoFileControl::ToStandardlPath(szFile, szStandFile, "/");
#endif
            if (bIsDir) {CAoFileControl::CreateDirs(szStandFile);continue;}// 目录
            sprintf(szBakFile, "%s/bak/%s/%s/%s", szDir, pszPrj, pszModule, szRelativeFile);
            CreateFileDir(szBakFile);
            CAoFile af;// 若正式文件存在，则将正式文件移动到临时目录下
            if (EN_NOERROR == af.Open(szStandFile, "rb")) {af.Close();
                CAoFileControl::CutFile(szStandFile, szBakFile);
            }
            CreateFileDir(szStandFile);
            if (EN_NOERROR != af.Open(szStandFile, "wb")) { // 写入文件失败
                LogErr("CUpdateModuleMgr::OnDownloadResult", "更新%s/%s至%s时解压文件打开%s失败", pszPrj, pszModule, pszFileVer, szFile);
                pDR->ucRlt = EN_DR_UFA;
                break;
            }
            au.UnzipCurentFile(&af, "");
            af.Close();
#ifdef WIN32
#else
            int m = 0;
            char* pszFileExt = NULL;
            for (m = (int)strlen(szStandFile) - 1; m > 0; --m) {
                if ('.' == szStandFile[m]) { pszFileExt = szStandFile + m; break; }
                if ('/' == szStandFile[m]) { break; }
            }
            // 无扩展名或者扩展名为.so, .lib, .exe则自动添加执行属性
            if (NULL == pszFileExt || 0 == strcmp(pszFileExt, ".so") || 0 == strcmp(pszFileExt, ".lib") || 0 == strcmp(pszFileExt, ".exe") || 0 == strcmp(pszFileExt, ".sh")) {
                sprintf(szFile, "chmod +x %s", szStandFile);
                CAoProcess ap; ap.RunCmd(szFile);
        }
#endif
        } while (au.GotoNextFile());
    }
    int nRet = CAoFileControl::DelFile(pszSaveFile);// 删除发布文件
    if (0 != nRet){LogErr("CUpdateModuleMgr::OnDownloadResult", "文件(%s)删除失败: %d", pszSaveFile, nRet);}
    if (itMm->second.strCMD.length() > 0 && EN_DR_SUC == pDR->ucRlt){// aods项目会涉及自身的重启，会丢失数据，因此需要重启前保存修改和并上报配置变更信息
        if (0 == strcmp(pszPrj, "aods") && strstr(pszModule, "aods-") != NULL){
            itMm->second.strFH = pszHash; // 文件hash
            itMm->second.strFV = pszFileVer; // 文件版本号
            SaveCfg(); // 保存配置
            Report2Aodc(EN_AODS_AODC_UDR, pMM->ucData, pMM->unDataLen); // 上报下载结果
            printf("更新重启项目%s/%s\r\n", pszPrj, pszModule);
            CAoProcess ap;
            pDR->ucRlt = (unsigned char)((EN_NOERROR == ap.RunCmd(itMm->second.strCMD.c_str())) ? EN_DR_SUC : EN_DR_RFA);
            LogInfo("CUpdateModuleMgr::OnDownloadResult", "成功更新%s/%s至%s", pszPrj, pszModule, pszFileVer);
            return;
        }
        printf("更新重启项目%s/%s\r\n", pszPrj, pszModule);
        CAoProcess ap;pDR->ucRlt = (unsigned char)((EN_NOERROR == ap.RunCmd(itMm->second.strCMD.c_str())) ? EN_DR_SUC : EN_DR_RFA);
    }
    if(EN_DR_SUC  == pDR->ucRlt || EN_DR_RFA == pDR->ucRlt){
        itMm->second.strFH = pszHash; // 文件hash
        itMm->second.strFV = pszFileVer; // 文件版本号
        LogInfo("CUpdateModuleMgr::OnDownloadResult", "成功更新%s/%s至%s", pszPrj, pszModule, pszFileVer);
        SaveCfg();
    }
    else{LogErr("CUpdateModuleMgr::OnDownloadResult", "更新%s/%s至%s失败， 错误代码: %d", pszPrj, pszModule, pszFileVer, pDR->ucRlt);}
    Report2Aodc(EN_AODS_AODC_UDR, pMM->ucData, pMM->unDataLen); // 上报下载结果
}
void CUpdateModuleMgr::StartUmiSyn(){
    char szFile[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szFile, EN_MAX_PATH_LEN);
    strcat(szFile, "/data/aodsumi.db");
    char* pszJsonBuf = GetJsonBufFromFile(szFile);
    if (NULL == pszJsonBuf){LogErr("CUpdateModuleMgr::OnAuthSuc", "模块更新数据文件(%s)加载失败", szFile);return;} // 更新配置为空，直接退出
    cJSON* pJson = cJSON_Parse(pszJsonBuf);
    SAFE_FREE(pszJsonBuf);
    if (NULL == pJson){LogErr("CUpdateModuleMgr::OnAuthSuc", "模块更新数据文件(%s)解析失败", szFile);return;}
    cJSON* pProperty = NULL;cJSON* pModule = NULL;cJSON* pPrj = NULL; MODULE_INFO mi;
    m_mapPM.clear();
    for (pPrj = pJson->child; NULL != pPrj; pPrj = pPrj->next){
        for (pModule = pPrj->child; NULL != pModule; pModule = pModule->next){
            pProperty = cJSON_GetObjectItem(pModule, "Hash");// 文件hash
            mi.strFH = (NULL != pProperty && cJSON_String == pProperty->type) ? pProperty->valuestring : "";
            pProperty = cJSON_GetObjectItem(pModule, "Version");// 文件版本号
            mi.strFV = (NULL != pProperty && cJSON_String == pProperty->type) ? pProperty->valuestring : "";
            pProperty = cJSON_GetObjectItem(pModule, "WorkDir");// 工作目录
            mi.strWD = (NULL != pProperty && cJSON_String == pProperty->type) ? pProperty->valuestring : "";
            pProperty = cJSON_GetObjectItem(pModule, "Command");// 重启命令
            mi.strCMD = (NULL != pProperty && cJSON_String == pProperty->type) ? pProperty->valuestring : "";
            m_mapPM[pPrj->string][pModule->string] = mi;
            SendUmi(pPrj->string, pModule->string, mi.strFV.c_str(), mi.strFH.c_str()); // 发送管理项目版本确认消息
        }
    }
    cJSON_Delete(pJson);
}
void CUpdateModuleMgr::SendUmi(const char* pszPrj, const char* pszModule, const char* pszFileVer, const char* pszFileHash){
    unsigned char ucData[1024] = {0}; // 向服务端发送版本确认消息
    memcpy(ucData, pszPrj, strlen(pszPrj) + 1);
    unsigned int unLen = (unsigned int)strlen(pszPrj) + 1;
    memcpy(ucData + unLen, pszModule, strlen(pszModule) + 1);
    unLen += (unsigned int)strlen(pszModule) + 1;
    memcpy(ucData + unLen, pszFileVer, strlen(pszFileVer) + 1);
    unLen += (unsigned int)strlen(pszFileVer) + 1;
    memcpy(ucData + unLen, pszFileHash, strlen(pszFileHash) + 1);
    unLen += (unsigned int)strlen(pszFileHash) + 1;
    Report2Aodc(EN_AODS_AODC_UMI, ucData, unLen);
}
void CUpdateModuleMgr::OnRecvUmi(const char* pszMsg, unsigned int unMsgLen){
    if (unMsgLen < 2 || '\0' != pszMsg[unMsgLen - 2]){return;}
    const char* pszPrjName = pszMsg; // 更新项目名称
    PM_MAP_IT itPrj = m_mapPM.find(pszPrjName);
    if (m_mapPM.end() == itPrj){return;}
    unsigned int unOffset = (unsigned int)strlen(pszPrjName) + 1;
    if (unOffset + 1 >= unMsgLen){return;}
    const char* pszModuleName = pszMsg + unOffset; // 模块名称
    MM_MAP_IT itModule = itPrj->second.find(pszModuleName);
    if (itPrj->second.end() == itModule){return;}
    unOffset += (unsigned int)strlen(pszModuleName) + 1;
    if (unOffset + 1 >= unMsgLen){return;}
    const char* pszFileVersion = pszMsg + unOffset; // 文件版本
    unOffset += (unsigned int)strlen(pszFileVersion) + 1;
    if (unOffset + 1 >= unMsgLen){return;}
    const char* pszFileHash = pszMsg + unOffset; // hash
    unOffset += (unsigned int)strlen(pszFileHash) + 1;
    if (unOffset + 1 >= unMsgLen){return;}
    const char* pszDownloadUrl = pszMsg + unOffset; // 下载地址
    unOffset += (unsigned int)strlen(pszDownloadUrl) + 1;
    if (unOffset + 1 >= unMsgLen){return;}
    const char* pszPassword = pszMsg + unOffset; // 解密密码
    unOffset += (unsigned int)strlen(pszPassword) + 1;
    if (unOffset + 1 > unMsgLen){return;}
    char cForceUpdate = pszMsg[unOffset]; // 强制更新
    if ( 1 == cForceUpdate ){
        char szDir[EN_MAX_PATH_LEN] = {0};
        CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
        char szFileName[EN_MAX_PATH_LEN] = {0};
        sprintf(szFileName, "%s/pub", szDir);
        CAoFileControl::CreateDirs(szFileName); // 创建目录
        sprintf(szFileName, "%s/pub/%s.%s.%s.%u.%u.zip", szDir, pszPrjName, pszModuleName, pszFileVersion, m_unUpdateTimes++, CAoTime::CurrentTick()); // 项目文件
        AppendDownloadTask(pszDownloadUrl, pszPassword, szFileName, pszPrjName, pszModuleName, pszFileVersion, pszFileHash);
        LogInfo("CUpdateModuleMgr::OnRecvUmi", "%s/%s准备更新版本至%s", pszPrjName, pszModuleName, pszFileVersion);
    }
}
void CUpdateModuleMgr::Report2Aodc(unsigned int unMsgCode, const unsigned char* pucData, unsigned int unDataLen){
    unsigned char ucData[1024] = {0};
    static const char* spszChannelName = "AodcClient";
    static unsigned int sunChannelNameLen = (unsigned int)strlen(spszChannelName) + 1;
    memcpy(ucData, spszChannelName, sunChannelNameLen);
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(ucData + sunChannelNameLen);
    pAAM->unMsgCode = unMsgCode;
    memcpy(pAAM->szMsg, pucData, unDataLen);
    SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, ucData, sunChannelNameLen + EN_AODS_AODC_MSG_SIZE + unDataLen);
}
bool CUpdateModuleMgr::SaveCfg(){
    cJSON* pPrj = NULL;
    cJSON* pModule = NULL;
    cJSON* pJsonObj = cJSON_CreateObject();
    for (PM_MAP_IT it1 = m_mapPM.begin(); it1 != m_mapPM.end(); ++it1){
        pPrj = cJSON_CreateObject();
        for (MM_MAP_IT it2 = it1->second.begin(); it2 != it1->second.end(); ++it2){
            MODULE_INFO& mi = it2->second;
            pModule = cJSON_CreateObject();
            cJSON_AddStringToObject(pModule, "Hash", mi.strFH.c_str());
            cJSON_AddStringToObject(pModule, "Version", mi.strFV.c_str());
            cJSON_AddStringToObject(pModule, "WorkDir", mi.strWD.c_str());
            cJSON_AddStringToObject(pModule, "Command", mi.strCMD.c_str());
            cJSON_AddItemToObject(pPrj, it2->first.c_str(), pModule);
        }
        cJSON_AddItemToObject(pJsonObj, it1->first.c_str(), pPrj);
    }
    char* pszJson = cJSON_PrintUnformatted(pJsonObj);
    cJSON_Delete(pJsonObj);
    char szFile[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szFile, EN_MAX_PATH_LEN);
    strcat(szFile, "/data/aodsumi.db");
    CAoFile af;
    if (0 == af.Open(szFile, "wb")) {af.Write(pszJson, (int)strlen(pszJson));}
    cJSON_FreeBuf(pszJson);
    return true;
}
void CUpdateModuleMgr::AppendDownloadTask(const char* pszUrl, const char* pszPwd, const char* pszFile, const char* pszPrj, const char* pszModule, const char* pszFileVer, const char* pszFileHash){
    int nMinLoad = 0xFFFFFF;
    CDownloadThread* pDT = NULL;
    for (int n = 0; n < EN_DT_NUM; ++n){if(nMinLoad > m_pDT[n]->Load()){pDT = m_pDT[n];nMinLoad = m_pDT[n]->Load();}}
    pDT->AppendDownloadTask(pszUrl, pszPwd, pszFile, pszPrj, pszModule, pszFileVer, pszFileHash);
}
void CUpdateModuleMgr::CreateFileDir(const char* pszFilePath){
    char szFileName[EN_MAX_PATH_LEN] = { 0 };
    char szPath[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::Split(pszFilePath, szPath, EN_MAX_PATH_LEN, szFileName, EN_MAX_PATH_LEN);
    CAoFileControl::CreateDirs(szPath);
}
void CUpdateModuleMgr::OnTimer(unsigned int ){StartUmiSyn();}
char* CUpdateModuleMgr::GetJsonBufFromFile(const char* pszFileName) {
    CAoFile af;
    if (0 != af.Open(pszFileName, "rb")) { return NULL;}
    af.Seek(0, EN_SEEK_END);
    ao_size_t astFileLen = af.Tell();
    char* pszBuf = (char*)malloc(astFileLen);
    af.Seek(0, EN_SEEK_SET);
    af.Read(pszBuf, astFileLen); // 小文件可以认为一次读完
    return pszBuf;
}