#include "../pch.h"
#include "../../../../src/dev/aods/AodsDef.h"
#include "../../../../src/dev/aods/UpdateModuleMgr.h"
#include "../../../../src/dev/util/AoFileControl.h"
#include "../../../../src/dev/util/ModuleDispatcher.h"
#include "UpdateModuleMgrTest.h"
#define TEST_FAIL(id) return (0==m_evFai##id.Wait(5000) && m_bFai##id);
BEGIN_AO_MSG_MAP(CUpdateModuleMgrTest, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA, &CUpdateModuleMgrTest::OnRecvData)
END_AO_MSG_MAP()
CUpdateModuleMgrTest::CUpdateModuleMgrTest():CModule(EN_MODULE_ID_BEGIN+0x14, 10240, "UpdateModuleMgrTest"){
    m_bSuc0 = false; m_bFai1 = false; m_bFai2 = false; m_bFai3 = false; m_bFai4 = false; m_bFai5 = false;
    m_evFai1.Reset(); m_evFai2.Reset(); m_evFai3.Reset(); m_evFai4.Reset(); m_evFai5.Reset();
}
CUpdateModuleMgrTest::~CUpdateModuleMgrTest(){}
bool CUpdateModuleMgrTest::TestSuc(){return 0 == m_evSuc0.Wait(5000) && m_bSuc0;}
bool CUpdateModuleMgrTest::TestFail1(){TEST_FAIL(1)}
bool CUpdateModuleMgrTest::TestFail2() {TEST_FAIL(2)}
bool CUpdateModuleMgrTest::TestFail3() {TEST_FAIL(3)}
bool CUpdateModuleMgrTest::TestFail4() {TEST_FAIL(4)}
bool CUpdateModuleMgrTest::TestFail5() {TEST_FAIL(5);}
void CUpdateModuleMgrTest::OnRecvData(const MT_MSG* pMM) {
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strcmp(pszChannelName, "AodcClient") != 0) {return;}
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(pMM->ucData + strlen(pszChannelName) + 1);
    if (EN_AODS_AODC_UMI == pAAM->unMsgCode) {
        unsigned char ucData[1024] = { 0 };
        memcpy(ucData, pMM->ucData, pMM->unDataLen);
        pAAM = (AODS_AODC_MSG*)(ucData + strlen(pszChannelName) + 1);
        pAAM->unMsgCode = EN_AODC_AODS_UMI;
        char szUrl[1024] = {0};
        const char* pszPrj = (const char*)pAAM->szMsg;
        sprintf(szUrl, "http://update.ao.com:8210/%s", pszPrj);
        memcpy(ucData + pMM->unDataLen, szUrl, strlen(szUrl) + 1);
        unsigned int unLen = pMM->unDataLen + (unsigned int)strlen(szUrl) + 1;
        for (int n = 0; n < 10; ++n, ++unLen) {ucData[unLen] = (unsigned char)rand();}
        ucData[unLen++] = '\0';
        ucData[unLen++] = 1;
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_RECV_DATA, ucData, unLen);

    } else if(EN_AODS_AODC_UDR == pAAM->unMsgCode){
        DOWNLOAD_RESULT* pDR = (DOWNLOAD_RESULT*)pAAM->szMsg;
        const char* pszPrj = (const char*)pDR->szBuf;
        if (0 == strcmp(pszPrj, "aom-suc0")) {m_bSuc0 = (EN_DR_SUC == pDR->ucRlt); m_evSuc0.Signal();
        }else if (0 == strcmp(pszPrj, "aom-fai1")) {m_bFai1 = (EN_DR_DFA == pDR->ucRlt); m_evFai1.Signal();
        }else if (0 == strcmp(pszPrj, "aom-fai2")) {m_bFai2 = (EN_DR_SFA == pDR->ucRlt); m_evFai2.Signal();
        }else if (0 == strcmp(pszPrj, "aom-fai3")) {m_bFai3 = (EN_DR_CFA == pDR->ucRlt); m_evFai3.Signal();
        }else if (0 == strcmp(pszPrj, "aom-fai4")) {m_bFai4 = (EN_DR_UFA == pDR->ucRlt); m_evFai4.Signal();
        }else if (0 == strcmp(pszPrj, "aom-fai5")) {m_bFai5 = (EN_DR_RFA == pDR->ucRlt); m_evFai5.Signal();
        }
    }
}
TEST(UpdateModuleMgrTest, ModuleTest) {
    CUpdateModuleMgrTest* pUMT = new CUpdateModuleMgrTest();
    pUMT->Start();CUpdateModuleMgr::Instance()->Start();
    CModuleDispatcher::Instance()->AppendMonitorModule(pUMT);
    CModuleDispatcher::Instance()->AppendModule(CUpdateModuleMgr::Instance());
    const char* pszJsonCfg = "{\"aom-suc0\": { 		\"0\": { 			\"WorkDir\": \"./0\", 			\"Command\": \"\" 		} 	}, 	\"aom-fai1\": { 		\"1\": { 			\"WorkDir\": \"./1\", 			\"Command\": \"\" 		} 	}, 	\"aom-fai2\": { 		\"2\": { 			\"WorkDir\": \"./2\", 			\"Command\": \"\" 		} 	}, 	\"aom-fai3\": { 		\"3\": { 			\"WorkDir\": \"./3\", 			\"Command\": \"\" 		} 	}, 	\"aom-fai4\": { 		\"4\": { 			\"WorkDir\": \"./4\", 			\"Command\": \"\" 		} 	}, 	\"aom-fai5\": { 		\"5\": { 			\"WorkDir\": \"./5\", 			\"Command\": \"pingmkkdkdkdkkdkd\" 		} 	} }";

    char szFile[EN_MAX_PATH_LEN] = { 0 };
    CAoFileControl::GetCurAppPath(szFile, EN_MAX_PATH_LEN);
    strcat(szFile, "/data/aodsumi.db");
    CAoFile af;
    if (0 != af.Open(szFile, "wb")) { EXPECT_TRUE(false); return; }
    af.Write(pszJsonCfg, strlen(pszJsonCfg));
    af.Close();
    CUpdateModuleMgr::Instance()->PostMsg(EN_AO_MSG_AUTHEN_SUC);
    EXPECT_TRUE(pUMT->TestSuc());
    EXPECT_TRUE(pUMT->TestFail1());
    EXPECT_TRUE(pUMT->TestFail2());
    EXPECT_TRUE(pUMT->TestFail3());
    EXPECT_TRUE(pUMT->TestFail4());
    EXPECT_TRUE(pUMT->TestFail5());
    CUpdateModuleMgr::Instance()->PostMsg(EN_AO_MSG_EXIT);
    pUMT->PostMsg(EN_AO_MSG_EXIT);
    pUMT->Join();
    CUpdateModuleMgr::Instance()->Join();
    delete pUMT;
    CUpdateModuleMgr::Instance()->Release();
    CModuleDispatcher::Instance()->Release();
}