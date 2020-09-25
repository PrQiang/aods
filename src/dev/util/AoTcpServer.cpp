/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <vector>
#ifdef WIN32
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#endif
#include "AoDef.h"
#include "CJson.h"
#include "Module.h"
#include "AoSockTcp.h"
#include "AoTcpServer.h"
#include "AoIoSelectGroup.h"
CAoTcpServer::CAoTcpServer(CModule* pOb, const char* pszName):CAoChannel(pOb, pszName){m_bRun = true;}
CAoTcpServer::~CAoTcpServer(){
    for (SC_LST_IT it = m_lstSC.begin(); it != m_lstSC.end(); ++it){SAFE_DELETE(*it);}
    m_lstSC.clear();
}
bool CAoTcpServer::Init(cJSON* pJsonPara){
    if (cJSON_Array != pJsonPara->type){return false; }
    int nNum = 0;
    int nPort = 0;
    std::string strName;
    std::string strAddr;
    const char* pszProtocol = NULL;
    cJSON* pServerProperty = NULL;
    for (cJSON* pServer = pJsonPara->child; NULL != pServer; pServer = pServer->next){
        pServerProperty = cJSON_GetObjectItem(pServer, "Name");
        if (NULL == pServerProperty || cJSON_String != pServerProperty->type){
            return false;
        }
        strName = pServerProperty->valuestring;
        pServerProperty = cJSON_GetObjectItem(pServer, "Address");
        if (NULL == pServerProperty || cJSON_String != pServerProperty->type){return false;}
        strAddr = pServerProperty->valuestring; // 服务器地址
        std::string::size_type tPos = strAddr.find_first_of(":");
        if (std::string::npos == tPos || tPos + 1 == strAddr.length()){return false;}
        sscanf(strAddr.substr(tPos + 1).c_str(), "%d", &nPort); // 监听端口
        strAddr = strAddr.substr(0, tPos);                     // 监听地址       
        pServerProperty = cJSON_GetObjectItem(pJsonPara, "Num");  // 获取监听总数
        nNum = (NULL == pServerProperty || cJSON_Number != pServerProperty->type) ? 10240 : pServerProperty->valueint;
        if (m_lstSC.size() + 1> EN_MAX_SERVER_NUM){return false;}
        pServerProperty = cJSON_GetObjectItem(pJsonPara, "Protocol");
        pszProtocol = (NULL == pServerProperty || cJSON_String != pServerProperty->type) ? "AoCommuProtocol" : pServerProperty->valuestring;
        m_lstSC.push_back(new SERVER_CHANNEL(strName.c_str(), strAddr.c_str(), pszProtocol, nPort, nNum));
    }
    return 0 == Start();
}
int CAoTcpServer::Run(){
    if (NULL == m_pOb|| !BuildServerChannels()){return 0;}
    int nHandle = 0;int nMaxHandle = 0;
    SERVER_CHANNEL* pSC = NULL;
    fd_set* pSet = (fd_set*)MALLOC(sizeof(int) * (EN_MAX_SERVER_NUM + 1));
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms
    char szIndex[64] = { 0 };
    char szCreateChannel[1024 ]= {0};
    const char* pszParaFormat = "{\"%s:%s\":{\"Type\":\"AoSockChannel\", \"Protocol\":\"%s\", \"Para\":{\"Handle\":%d, \"Protocol\":\"Tcp\"}}}";
    while (m_bRun){
        FD_ZERO(pSet);
        nMaxHandle = 0;
        for (SC_LST_IT it = m_lstSC.begin(); it != m_lstSC.end(); ++it){
            nHandle = (*it)->pAST->Socket();
            FD_SET(nHandle, pSet);
            nMaxHandle = (nMaxHandle < nHandle) ? nHandle : nMaxHandle;
        }
        if(select(nMaxHandle, pSet, NULL, NULL, &tv) < 1){continue;}
        for (SC_LST_IT it = m_lstSC.begin(); it != m_lstSC.end(); ++it){
            pSC = (*it);
            if (!FD_ISSET(pSC->pAST->Socket(), pSet)){continue;}
            sprintf(szIndex, "%016x", pSC->ullIndex++);
            sprintf(szCreateChannel, pszParaFormat, pSC->strName.c_str(), szIndex, pSC->strProtocol.c_str(), pSC->pAST->Accept());
            SendOutMsg(EN_MODULE_ID_COMMU, EN_AO_MSG_CHANNEL_NEW, (unsigned char*)szCreateChannel, (unsigned int)strlen(szCreateChannel) + 1);
        }
    }
    SAFE_FREE(pSet);
    return 0;
}
void CAoTcpServer::Stop(){m_bRun = false;CAoThread::Join();}
bool CAoTcpServer::BuildServerChannels(){
    int nRet = EN_NOERROR;
    SERVER_CHANNEL* pSC = NULL;
    for (SC_LST_IT it = m_lstSC.begin(); it != m_lstSC.end(); ++it){
        pSC = (*it);
        pSC->pAST = new CAoSockTcp();
        pSC->pAST->SetPortReused(1);
        nRet = pSC->pAST->Bind(pSC->strAddr.c_str(), pSC->nPort);
        if( EN_NOERROR != nRet){
            m_pOb->LogErr("CAoTcpServer::Run", "Failed to bind(%s:%d), error is %u", pSC->strAddr.c_str(), pSC->nPort, nRet);
            return false;
        }
        nRet = pSC->pAST->Listen(pSC->nNum);
        if( EN_NOERROR != nRet){
            m_pOb->LogErr("CAoTcpServer::Run", "Failed to Listen(%u) at(%s:%d), error is %u", pSC->nNum, pSC->strAddr.c_str(), pSC->nPort, nRet);
            return false;
        }
    }
    return true;
}