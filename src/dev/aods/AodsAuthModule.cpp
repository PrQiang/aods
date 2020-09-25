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
#endif
#include "AodsDef.h"
#include "../util/AoFile.h"
#include "../util/AoEncrypt.h"
#include "../util/AoService.h"
#include "../AodsAodcMsgDef.h"
#include "../util/AoFileControl.h"
#include "AodsAuthModule.h"
CAodsAuthModule* CAodsAuthModule::ms_pInstance = NULL;
CAoLock CAodsAuthModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAodsAuthModule, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_BUILDED, &CAodsAuthModule::OnChannelBuild)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_RECV_DATA, &CAodsAuthModule::OnRecvChannelData)
END_AO_MSG_MAP()
CAodsAuthModule::CAodsAuthModule():CModule(EN_MODULE_ID_AUTH, 1024*1024, "AodsAuthModule"){}
CAodsAuthModule::~CAodsAuthModule(){}
CAodsAuthModule* CAodsAuthModule::Instance(){
    if(NULL == ms_pInstance){
        ms_Lock.Lock();
        ms_pInstance = (NULL == ms_pInstance) ? new CAodsAuthModule : ms_pInstance;
        ms_Lock.Unlock();
    }
    return ms_pInstance;
}
void CAodsAuthModule::Release(){SAFE_DELETE(ms_pInstance);}
void CAodsAuthModule::OnChannelBuild(const MT_MSG* pMM){
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strcmp(pszChannelName, "AodcClient") != 0){return;}
    OnAuthen(pszChannelName);
}
void CAodsAuthModule::OnRecvChannelData(const MT_MSG* pMM){
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strcmp(pszChannelName, "AodcClient") != 0){return;}
    if (pMM->unDataLen < strlen(pszChannelName) + 1 + EN_AODS_AODC_MSG_SIZE){return;}
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + strlen(pszChannelName) + 1);
    unsigned int unDataLen = (pMM->unDataLen - EN_AODS_AODC_MSG_SIZE - (unsigned int)strlen(pszChannelName) - 1);
    if (EN_AODC_AODS_KEEPALIVE == pAAM->unMsgCode){
        unsigned char ucData[64] = { 0 };
        memcpy(ucData, "AodcClient", 11);
        pAAM = (AODS_AODC_MSG*)(ucData + 11);
        pAAM->unMsgCode = EN_AODS_AODC_KEEPALIVE;
        SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, ucData, 11 + EN_AODS_AODC_MSG_SIZE);
        return;
    }
    if (EN_AODC_AODS_AUTH != pAAM->unMsgCode || unDataLen < EN_AODC_AODS_AUTHEN_SIZE) {  return;  }
    AODC_AODS_AUTHEN* pAAA = (AODC_AODS_AUTHEN*)pAAM->szMsg;
    switch(pAAA->ucRlt){
    case EN_AUTH_RESULT_SUC: // 认证成功
        LogInfo("CAodsAuthModule::OnRecvChannelData", "认证成功");
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_AUTHEN_SUC);
        break;
    case EN_AUTH_RESULT_FAI: // 认证失败        
        LogErr("CAodsAuthModule::OnRecvChannelData", "认证失败");
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
        CAoService::Stop("aods");
        break;
    case EN_AUTH_RESULT_DIF: // 多服务器认证        
        LogErr("CAodsAuthModule::OnRecvChannelData", "多服务器重复认证,请重新激活");
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
        CAoService::Stop("aods");
        break;
    case EN_AUTH_RESULT_BUS:OnAuthen(pszChannelName);break; // 繁忙, 重新发起认证
    default:break;
    }
}
void CAodsAuthModule::OnAuthen(const char* pszChannelName){
    unsigned char* pucData = (unsigned char*)malloc(1024);
    if (NULL == pucData) {LogErr("CAodsAuthModule::OnChannelBuild", "Failed to malloc data(size 1024)");return; }
    memcpy(pucData, pszChannelName, strlen(pszChannelName) + 1);
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pucData + strlen(pszChannelName) + 1);
    pAAM->unMsgCode = EN_AODS_AODC_AUTH;
    AODS_AODC_AUTHEN* pAAA = (AODS_AODC_AUTHEN*)pAAM->szMsg;
    if (!ReadActiveData(pAAA) || !GetIpAddrs(pAAA->ucIpNum, pAAA->unIpAddr)){
        LogErr("CAodsAuthModule::OnChannelBuild", "Failed to get active data");
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
        return;
    }
    SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_SEND_DATA, pucData, (unsigned int)strlen(pszChannelName) + 1 + EN_AODS_AODC_MSG_SIZE + EN_AODS_AODC_AUTHEN_SIZE);
}
bool CAodsAuthModule::ReadActiveData(AODS_AODC_AUTHEN* pAAA){
    char szDir[EN_MAX_PATH_LEN] = {0};// 填充文件名称
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    char szFile[EN_MAX_PATH_LEN] = {0};
    sprintf(szFile, "%s/data/aoauth.db", szDir);
    CAoFile af;
    if(af.Open(szFile, "rb") != EN_NOERROR){return false;}
    af.Seek(0, EN_SEEK_END);
    ao_size_t ast = af.Tell();
    if (ast < EN_AODS_AODC_AUTHEN_SIZE){return false;}
    char* pszBuf = (char*)malloc(ast);
    af.Seek(0, EN_SEEK_SET);
    af.Read(pszBuf, ast);
    af.Close();
    unsigned char ucKey[16] = { 0 };
    for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0xBC; }
    CAoEncrypt ae;
    ae.SetKey(ucKey, 16);
    ae.Decode((unsigned char*)pszBuf, (unsigned int)ast);
    memcpy(pAAA->ucResId, pszBuf, EN_RES_ID_LEN); // 资源号
    unsigned int unOffset = EN_RES_ID_LEN;
    memcpy(pAAA->ucRDSerial, pszBuf + unOffset, EN_RD_SERIAL_LEN); // 随机代码
    unOffset += EN_RD_SERIAL_LEN;
    pAAA->ucIpNum = pszBuf[unOffset++];// IP地址
    memcpy(pAAA->unIpAddr, pszBuf + unOffset, EN_MAX_IP_ADDR_NUM * 4);
    unOffset += EN_MAX_IP_ADDR_NUM * 4;
    memcpy(pAAA->ucActCode, pszBuf + unOffset, EN_ACT_CODE_LEN);// 激活码
    SAFE_FREE(pszBuf);
    return true;
}
bool CAodsAuthModule::GetIpAddrs(unsigned char& ucNum, unsigned int* punIpAddrs){
#ifdef WIN32
    ULONG ulNum = 0;
    if (ERROR_INSUFFICIENT_BUFFER != GetIpAddrTable(NULL, &ulNum, false)){return false;}
    MIB_IPADDRTABLE* p = (MIB_IPADDRTABLE*)malloc(ulNum);
    if (NULL == p){return false;}
    if (NO_ERROR != GetIpAddrTable(p, &ulNum, false)){
        free(p);
        return false;
    }
    unsigned char ucIndex = 0;
    for (unsigned char n = 0; n < p->dwNumEntries && ucIndex < ucNum; ++n){        
        if (p->table[n].dwAddr == 0x0100007F){continue;} // 排除127.0.0.1的IP地址
        punIpAddrs[ucIndex++] = p->table[n].dwAddr;
    }
    ucNum = ucIndex;
    free(p);
    return true;
#else
    ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1){
        return false;
    }
    unsigned char ucIndex = 0;
    for (ifaddrs* ia = ifaddr; NULL != ia; ia = ia->ifa_next){
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
