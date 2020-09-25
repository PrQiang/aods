#include "AodsActiveModuleTestFai.h"
#include "../../../../src/dev/util/AoDef.h"
#include "../../../../src/dev/util/AoTime.h"
#include "../../../../src/dev/util/AoFileControl.h"
#include "../../../../src/dev/util/AoFile.h"
#include "../../../../src/dev/util/AoEncrypt.h"
#include "../../../../src/dev/install/AodsActiveModule.h"
BEGIN_AO_MSG_MAP(CAodsActiveModuleTestFai, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA, &CAodsActiveModuleTestFai::OnRecvActive)
END_AO_MSG_MAP()
CAodsActiveModuleTestFai::CAodsActiveModuleTestFai():CModule(EN_MODULE_ID_BEGIN + 0x11, 102400, "AodsActiveModuleTest") {}
CAodsActiveModuleTestFai::~CAodsActiveModuleTestFai() {}
bool CAodsActiveModuleTestFai::Test(){
    char szDir[EN_MAX_PATH_LEN] = { 0 };// 移除激活文件
    CAoFileControl::GetCurAppPath(szDir, EN_MAX_PATH_LEN);
    char szFile[EN_MAX_PATH_LEN] = { 0 };
    sprintf(szFile, "%s/data/aoauth.db", szDir);
    CAoFileControl::DelFile(szFile);
    srand(CAoTime::CurrentTick()); // 开始测试
    m_ucIndex = (unsigned char)rand();
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_ACTIVE_PARA, (const unsigned char*)&m_ucIndex, 1);
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_BUILDED, (const unsigned char*)"AodcClient", strlen("AodcClient") + 1);
    Join();
    if (!m_bResult) { return false; }
    CAoFile af;
    return (0 != af.Open(szFile, "rb"));
}
void CAodsActiveModuleTestFai::OnRecvActive(const MT_MSG * pMM) {
    const char* pszChannelName = (const char*)pMM->ucData;
    if (strcmp(pszChannelName, "AodcClient") != 0) {
        m_bResult = false;
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
        return;
    }
    AODS_AODC_ACTIVE* pAAA = (AODS_AODC_ACTIVE*)(pMM->ucData + strlen(pszChannelName) + 1 + EN_AODS_AODC_MSG_SIZE);
    if (pAAA->ucIndex != m_ucIndex) {
        m_bResult = false;
        SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_EXIT);
        return;
    }
    m_bResult = true;
    unsigned char ucData[64] = { 0 };
    memcpy(ucData, pszChannelName, strlen(pszChannelName) + 1);
    AODS_AODC_MSG* pAAM = (AODS_AODC_MSG*)(ucData + strlen(pszChannelName) + 1);
    pAAM->unMsgCode = EN_AODC_AODS_ACTV;
    AODC_AODS_ACTIVE* pA = (AODC_AODS_ACTIVE*)pAAM->szMsg;
    pA->ucResult = EN_ACT_RLT_ACTIVEDBYOTHER;
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_RECV_DATA, ucData, strlen(pszChannelName) + 1 + EN_AODC_AODS_ACTIVE_SIZE +  EN_AODS_AODC_MSG_SIZE);
};