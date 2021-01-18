#include "../pch.h"
#include <stdlib.h>
#include "AodsAuthModuleTest.h"
#include "../../../../src/dev/util/ModuleDispatcher.h"
#include "../../../../src/dev/util/AoFileControl.h"
#include "../../../../src/dev/util/AoFile.h"
#include "../../../../src/dev/util/AoEncrypt.h"
#include "../../../../src/dev/aods/AodsDef.h"
#include "../../../../src/dev/aods/AodsAuthModule.h"
#include "../../../../src/dev/install/AodsActiveModule.h"
BEGIN_AO_MSG_MAP(CAodsAuthModuleTest, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA, &CAodsAuthModuleTest::OnRecvAuth)
    ON_AO_MSG_FUN(EN_AO_MSG_AUTHEN_SUC, &CAodsAuthModuleTest::OnRecvAuthSuc)
    ON_AO_MSG_FUN(EN_AO_MSG_EXIT, &CAodsAuthModuleTest::OnRecvQuit)
END_AO_MSG_MAP()
CAodsAuthModuleTest::CAodsAuthModuleTest():CModule(EN_MODULE_ID_BEGIN + 0x13, 10240, "AodsAuthModuleTest"){}
CAodsAuthModuleTest::~CAodsAuthModuleTest(){}
bool CAodsAuthModuleTest::constructAuthFile() {
    char szDir[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    char szFile[EN_MAX_PATH_LEN] = { 0 };
    sprintf(szFile, "%s/data", szDir);
    CAoFileControl::CreateDir(szFile);
    sprintf(szFile, "%s/data/aoauth.db", szDir);// 填充激活信息
    const int nLen = EN_AODS_AODC_AUTHEN_SIZE;
    char szSaveData[nLen] = { 0 };
    for (char n = 0; n < nLen; ++n) { szSaveData[n] = n + 1; }
    unsigned char ucKey[16] = { 0 };
    for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0xBC; }
    CAoEncrypt ae;ae.SetKey(ucKey, 16);
    ae.Encode((unsigned char*)szSaveData, (unsigned int)nLen);
    CAoFile af;// 将激活码数据写入文件
    if (EN_NOERROR != af.Open(szFile, "wb")) {
        printf("Failed to open file(%s), please check your permission.\r\n", szFile);
        return false;
    }
    af.Write(szSaveData, (ao_size_t)nLen);
    af.Close();
    return true;
}
bool CAodsAuthModuleTest::TestSuc() {
    constructAuthFile();
    m_bTestSuc = true; m_bResult = false; m_evSignal.Reset();
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_BUILDED, (const unsigned char*)"AodcClient", strlen("AodcClient") + 1);
    return 0 == m_evSignal.Wait(5000) && m_bResult;
}
bool CAodsAuthModuleTest::TestFai() {
    constructAuthFile();
    m_bTestSuc = false; m_bResult = false; m_evSignal.Reset();
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_BUILDED, (const unsigned char*)"AodcClient", strlen("AodcClient") + 1);
    return 0 ==m_evSignal.Wait(5000) && m_bResult;
}
void CAodsAuthModuleTest::OnRecvAuth(const MT_MSG* pMM){
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strcmp(pszChannelName, "AodcClient") != 0) {        
        m_evSignal.Signal();
        return;
    }
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + strlen(pszChannelName) + 1);
    if (EN_AODS_AODC_AUTH != pAAM->unMsgCode) {
        m_evSignal.Signal();
        return ;
    }    
    AODS_AODC_AUTHEN* pAAA = (AODS_AODC_AUTHEN*)pAAM->szMsg;// 校验数据
    for (unsigned char n = 0; n < EN_RES_ID_LEN; ++n) {
        if (pAAA->ucResId[n] != n + 1) {
            m_evSignal.Signal();
            return; 
        }
    }
    for (unsigned char n = 0; n < EN_RD_SERIAL_LEN; ++n) {
        if (pAAA->ucRDSerial[n] != n + 1 + EN_RES_ID_LEN) {
            m_evSignal.Signal();
            return;
        }
    }
    for (unsigned char n = 0; n < EN_ACT_CODE_LEN; ++n) {
        if (pAAA->ucActCode[n] != n + 61 + EN_RES_ID_LEN+ EN_RD_SERIAL_LEN + 5) {
            m_evSignal.Signal();
            return;
        }
    }
    m_bResult = true;
    unsigned char ucData[64] = { 0 };
    memcpy(ucData, pszChannelName, strlen(pszChannelName) + 1);
    pAAM = (AODS_AODC_MSG*)(ucData + strlen(pszChannelName) + 1);
    pAAM->unMsgCode = EN_AODC_AODS_AUTH;
    AODC_AODS_AUTHEN* pA = (AODC_AODS_AUTHEN*)pAAM->szMsg;
    pA->ucRlt = (unsigned char)(m_bTestSuc ? EN_AUTH_RESULT_SUC : EN_AUTH_RESULT_FAI);
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_RECV_DATA, ucData, strlen(pszChannelName) + 1 + EN_AODC_AODS_AUTHEN_SIZE + EN_AODS_AODC_MSG_SIZE);
}
void CAodsAuthModuleTest::OnRecvAuthSuc(const MT_MSG* ) {
    if (m_bTestSuc) {m_evSignal.Signal();}
}
void CAodsAuthModuleTest::OnRecvQuit(const MT_MSG* pMM){
    if(!m_bTestSuc){m_evSignal.Signal();}
    CModule::OnStop(pMM);
}
TEST(AodsAuthModuleTest, ModuleTest) {
    CAodsAuthModule::Instance()->Start();
    CModuleDispatcher::Instance()->AppendModule(CAodsAuthModule::Instance());
    CAodsAuthModuleTest* pTest = new CAodsAuthModuleTest();
    pTest->Start();
    CModuleDispatcher::Instance()->AppendMonitorModule(pTest);
    EXPECT_TRUE(pTest->TestSuc());
    EXPECT_TRUE(pTest->TestFai());
    CAodsActiveModule::Instance()->PostMsg(EN_AO_MSG_EXIT);
    CAodsActiveModule::Instance()->Join();
    pTest->PostMsg(EN_AO_MSG_EXIT);
    pTest->Join();
    delete pTest;
    CAodsAuthModule::Instance()->Release();
    CModuleDispatcher::Instance()->Release();
}
