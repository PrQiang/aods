/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifdef WIN32
#include <WinSock2.h>
#include <iphlpapi.h>
#include <atlconv.h>
#else
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif
#include "../util/AoOs.h"
#include "../util/AoTime.h"
#include "../util/AoDef.h"
#include "../util/AoFile.h"
#include "../util/AoProcess.h"
#include "../util/AoHelper.h"
#include "../util/AoEncrypt.h"
#include "../AodsAodcMsgDef.h"
#include "../util/AoService.h"
#include "../util/AoFileControl.h"
#include "AodsActiveModule.h"
CAodsActiveModule* CAodsActiveModule::ms_pInstance = NULL;
CAoLock CAodsActiveModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAodsActiveModule, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_BUILDED, &CAodsActiveModule::OnChannelBuild)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CAodsActiveModule::OnRecvChannelData)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, &CAodsActiveModule::OnSendChannelDataFail)
    ON_AO_MSG_FUN(EN_AO_MSG_ACTIVE_PARA, &CAodsActiveModule::OnSetActivePara)
END_AO_MSG_MAP()
CAodsActiveModule::CAodsActiveModule()
    :CModule(EN_MODULE_ID_ACTIVE, 1024 * 1024, "AodsActiveModule"){
    m_ucIpNum = 0;
    memset(m_unIpAddr, 0, EN_MAX_IP_ADDR_NUM * 4); // Ip地址
    m_bActived = false;
}
CAodsActiveModule::~CAodsActiveModule(){}
CAodsActiveModule* CAodsActiveModule::Instance(){
    if(NULL == ms_pInstance){
        ms_Lock.Lock();
        ms_pInstance = (NULL == ms_pInstance) ? new CAodsActiveModule : ms_pInstance;
        ms_Lock.Unlock();
    }
    return ms_pInstance;
}
void CAodsActiveModule::Release(){SAFE_DELETE(ms_pInstance);}
void CAodsActiveModule::OnChannelBuild(const MT_MSG* pMM){
    // aodcclient 通道建立连接，发起激活流程
    const char* pszName = (const char*)pMM->ucData;
    if (m_bActived || strcmp(pszName, "AodcClient") != 0){return;}
    unsigned char* pucData = (unsigned char*)MALLOC(10240);
    memcpy(pucData, pszName, strlen(pszName) + 1);
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pucData + strlen(pszName) + 1);
    pAAM->unMsgCode = EN_AODS_AODC_ACTV;
    srand(CAoTime::CurrentTick());
    for (int n = 0; n < EN_RD_SERIAL_LEN; ++n){m_ucRDSerial[n] = (unsigned char)rand();}
    // 获取服务器网卡地址清单
    m_ucIpNum = EN_MAX_IP_ADDR_NUM; // 最大支持IP个数
    GetIpAddrs(m_ucIpNum, m_unIpAddr);
    printf("正在等待激活确认...\r\n");
    AODS_AODC_ACTIVE* pAAA = (AODS_AODC_ACTIVE*)pAAM->szMsg;
    pAAA->ucIpNum = m_ucIpNum;
    pAAA->ucIndex = m_ucIndex;
    memcpy( pAAA->unIpAddr, m_unIpAddr, EN_MAX_IP_ADDR_NUM * sizeof(unsigned int));
    memset( pAAA->ucResId, 0, EN_RES_ID_LEN);
    memcpy(pAAA->ucRDSerial, m_ucRDSerial, EN_RD_SERIAL_LEN);   
    SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, pucData, EN_AODS_AODC_ACTIVATE_SIZE + EN_AODS_AODC_MSG_SIZE + (unsigned int)strlen(pszName) + 1);
    SAFE_FREE(pucData);
}
void CAodsActiveModule::OnRecvChannelData(const MT_MSG* pMM){    
    const char* pszChannelName = (const char*)pMM->ucData;// 通道名称
    if (strcmp(pszChannelName, "AodcClient") != 0) {return;}
    if (pMM->unDataLen <= strlen(pszChannelName) + 1 + EN_AODS_AODC_MSG_SIZE) {return;}
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + strlen(pszChannelName) + 1);
    if (EN_AODC_AODS_ACTV != pAAM->unMsgCode){return;}
    unsigned int unDataLen = (pMM->unDataLen - EN_AODS_AODC_MSG_SIZE - (unsigned int)strlen(pszChannelName) - 1);
    if (unDataLen < EN_AODC_AODS_ACTIVE_SIZE){return;}
    m_bActived = true;
    AODC_AODS_ACTIVE* pAAA = (AODC_AODS_ACTIVE*)pAAM->szMsg;
    switch(pAAA->ucResult){
    case EN_ACT_SUCCESS:ActiveSuccess(pAAA->ucData, unDataLen - EN_AODC_AODS_ACTIVE_SIZE);break; // 激活成功
    case EN_ACT_RLT_ACTIVEDBYOTHER:printf("激活失败\r\n");break; // 激活失败
    case EN_ACT_RLT_DBBUSY:printf("服务器繁忙，请稍后重试\r\n");break; // 繁忙
    case EN_ACT_RLT_REMOVED:printf("服务器被管理员移除\r\n");break; // 被移除
    case EN_ACT_RLT_APPLYRESFAIL:printf("申请资源号失败，请稍后重试\r\n");break; // 申请资源号失败
    case EN_ACT_RLT_DATAINV:printf("数据非法\r\n");break; // 数据非法
    default:break;
    }
#ifdef WIN32 // windows操作系统中增加信息展示时间, linux不需要
    CAoOs::Sleep(10000);
#endif
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
}
void CAodsActiveModule::OnSendChannelDataFail(const MT_MSG* pMM){   
    const char* pszChannelName = (const char*)pMM->ucData; // 通道名称
    if (strcmp(pszChannelName, "AodcClient") != 0){return;}
    if (pMM->unDataLen <= strlen(pszChannelName) + 1){return;}
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + strlen(pszChannelName) + 1);
    if (EN_AODS_AODC_ACTV != pAAM->unMsgCode){return;}
    printf("正在等待激活确认...\r\n");
}
void CAodsActiveModule::OnSetActivePara(const MT_MSG* pMM) {m_ucIndex = pMM->ucData[0];}
bool CAodsActiveModule::GetIpAddrs(unsigned char& ucNum, unsigned int* punIpAddrs){
#ifdef WIN32
    ULONG ulNum = 0;
    if(ERROR_INSUFFICIENT_BUFFER != GetIpAddrTable(NULL, &ulNum, false)){return false;}
    MIB_IPADDRTABLE* p = (MIB_IPADDRTABLE*)MALLOC(ulNum);
    if (NULL == p){return false;}
    if (NO_ERROR != GetIpAddrTable(p, &ulNum, false)){SAFE_FREE(p);return false; }
    unsigned char ucIndex = 0;
    for (unsigned char n = 0; n < p->dwNumEntries && ucIndex < ucNum; ++n){        
        if ( p->table[n].dwAddr == 0x0100007F){continue;}// 排除127.0.0.1的IP地址
        punIpAddrs[ucIndex++] = p->table[n].dwAddr;
    }
    ucNum = ucIndex;
    SAFE_FREE(p);
    return true;
#else
    ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1){return false;}
    unsigned char ucIndex = 0;
    for(ifaddrs* ia = ifaddr; NULL != ia; ia = ia->ifa_next){
        if (NULL == ia->ifa_addr){continue;}
        if (AF_INET != ia->ifa_addr->sa_family){continue;}
        if (ucIndex >= ucNum){break;}        
        punIpAddrs[ucIndex++] = ((sockaddr_in*)ia->ifa_addr)->sin_addr.s_addr;
    }
    ucNum = ucIndex;
    freeifaddrs(ifaddr);
    return true;
#endif
}
void CAodsActiveModule::ActiveSuccess(const unsigned char* pucActData, unsigned int unCodeLen){
    if ( EN_AODS_ACTIVE_SUC_SIZE != unCodeLen ){return;}    
    if(!SaveActive((AODS_ACTIVE_SUC*)pucActData)){return;}// 保存激活消息
    printf("激活成功. \r\n");    
#ifdef WIN32// 先停止服务，保证能够正常覆盖文件
    CAoService::Stop("aods");
#endif
    // 准备安装服务
    char szDir[EN_MAX_PATH_LEN] = {0};
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    char szFile[EN_MAX_PATH_LEN] = {0};
#ifdef WIN32
    // 检测系统类型，x86则执行x86流程，否则执行x64的流程
    /*if (CAoOs::EN_SYS_TYPE_X64 == CAoOs::SysType()){
        // 移除x86执行文件
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/aods-x86.exe", szDir);
        CAoFileControl::DelFile(szFile);
        // 移除x86的配置文件
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/data/aodsumi-x86.db", szDir);
        CAoFileControl::DelFile(szFile);
        // 将x64的配置文件文件重命名为正常启动配置文件
        char szExe[EN_MAX_PATH_LEN] = { 0 };
        sprintf_s(szExe, EN_MAX_PATH_LEN, "%s/data/aodsumi-x64.db", szDir);
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/data/aodsumi.db", szDir);
        CAoFileControl::DelFile(szFile);
        MoveFile(szExe, szFile);
        // 将x64的执行文件重命名为正常启动程序
        sprintf_s(szExe, EN_MAX_PATH_LEN, "%s/aods-x64.exe", szDir);
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/aods.exe", szDir);
        CAoFileControl::DelFile(szFile);
        MoveFile(szExe, szFile);
        CAoFileControl::DelFile(szExe);
    }else{
        // 移除x64执行文件
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/aods-x64.exe", szDir);
        CAoFileControl::DelFile(szFile);
        // 移除x64的配置文件
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/data/aodsumi-x64.db", szDir);
        CAoFileControl::DelFile(szFile);
        // 将x86的配置文件文件重命名为正常启动配置文件
        char szExe[EN_MAX_PATH_LEN] = { 0 };
        sprintf_s(szExe, EN_MAX_PATH_LEN, "%s/data/aodsumi-x86.db", szDir);
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/data/aodsumi.db", szDir);
        CAoFileControl::DelFile(szFile);
        MoveFile(szExe, szFile);
        // 将x86的执行文件重命名为正常启动程序
        sprintf_s(szExe, EN_MAX_PATH_LEN, "%s/aods-x86.exe", szDir);
        sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/aods.exe", szDir);
        CAoFileControl::DelFile(szFile);
        MoveFile(szExe, szFile);
        CAoFileControl::DelFile(szExe);
    }*/
    sprintf_s(szFile, EN_MAX_PATH_LEN, "%s/aods.exe", szDir);
#else
    sprintf(szFile, "%s/aods", szDir);
#endif
    printf("正在安装服务...\r\n");
    if(0 == CAoService::Install("aods", szFile, "Automatic Operation Deployment Service", "自动运维部署服务")){
        CAoOs::Sleep(3000);
        printf("服务安装成功\r\n");
        CAoService::Start("aods");
        return;
    }
    printf("服务安装失败，请重新安装\r\n");
}
bool CAodsActiveModule::SaveActive(const AODS_ACTIVE_SUC* pAAS){
    // 获取保存文件名称，并确保路径存在
    char szDir[EN_MAX_PATH_LEN] = {0};
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    char szFile[EN_MAX_PATH_LEN] = {0};
    sprintf(szFile, "%s/data", szDir);
    CAoFileControl::CreateDir(szFile);
    sprintf(szFile, "%s/data/aoauth.db", szDir);
    // 填充激活信息
    char szSaveData[256] = {0};
    memcpy(szSaveData, pAAS->ucResId, EN_RES_ID_LEN);
    unsigned int unOffset = EN_RES_ID_LEN;
    memcpy(szSaveData + unOffset, m_ucRDSerial, EN_RD_SERIAL_LEN);
    unOffset += EN_RD_SERIAL_LEN;
    szSaveData[unOffset++] = m_ucIpNum;
    memcpy(szSaveData + unOffset, m_unIpAddr, EN_MAX_IP_ADDR_NUM * 4);
    unOffset += EN_MAX_IP_ADDR_NUM * 4;
    memcpy(szSaveData + unOffset, pAAS->ucActCode, EN_ACT_CODE_LEN);
    unOffset += EN_ACT_CODE_LEN;
    // 加密激活码
    unsigned char ucKey[16] = { 0 };
    for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0xBC;}
    CAoEncrypt ae;
    ae.SetKey(ucKey, 16);
    ae.Encode((unsigned char*)szSaveData, (unsigned int)unOffset);
    // 将激活码数据写入文件
    CAoFile af;
    if(EN_NOERROR != af.Open(szFile, "wb")){
        printf("Failed to open file(%s), please check your permission.\r\n", szFile);
        return false;
    }
    af.Write(szSaveData, (ao_size_t)unOffset);
    af.Close();
    return true;
}
