#include "AodsActiveModuleTestSuc.h"
#include "../../../../src/dev/util/AoDef.h"
#include "../../../../src/dev/util/AoTime.h"
#include "../../../../src/dev/util/AoFileControl.h"
#include "../../../../src/dev/util/AoFile.h"
#include "../../../../src/dev/util/AoEncrypt.h"
#include "../../../../src/dev/install/AodsActiveModule.h"
BEGIN_AO_MSG_MAP(CAodsActiveModuleTestSuc, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_CHANNEL_SEND_DATA, &CAodsActiveModuleTestSuc::OnRecvActive)
END_AO_MSG_MAP()
CAodsActiveModuleTestSuc::CAodsActiveModuleTestSuc():CModule(EN_MODULE_ID_BEGIN + 0x12, 102400, "AodsActiveModuleTest") {}
CAodsActiveModuleTestSuc::~CAodsActiveModuleTestSuc() {}
bool CAodsActiveModuleTestSuc::Test(){
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
    if (0 != af.Open(szFile, "rb")) { return false; }
    af.Seek(0, EN_SEEK_END);
    ao_size_t astLen = af.Tell();
    af.Seek(0, EN_SEEK_SET);
    char* pszBuf = (char*)malloc(astLen);
    af.Read(pszBuf, astLen);
    unsigned char ucKey[16] = { 0 };
    for (unsigned char n = 0; n < 16; ++n) { ucKey[n] = n + 0xBC; }
    CAoEncrypt ae;
    ae.SetKey(ucKey, 16);
    ae.Decode((unsigned char*)pszBuf, (unsigned int)astLen);
    for (int n = 0; n < EN_RES_ID_LEN; ++n) { if ((unsigned char)pszBuf[n] != m_ucIndex)return false; }
    for (int n = astLen - EN_ACT_CODE_LEN; n < astLen; ++n) { if ((unsigned char)pszBuf[n] != m_ucIndex){return false; }}
    return true;
}
void CAodsActiveModuleTestSuc::OnRecvActive(const MT_MSG * pMM) {
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
    pA->ucResult = EN_ACT_SUCCESS;
    memset(pA->ucData, m_ucIndex, EN_ACT_CODE_LEN + EN_RES_ID_LEN);
    SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_CHANNEL_RECV_DATA, ucData, strlen(pszChannelName) + 1 + EN_AODC_AODS_ACTIVE_SIZE + EN_AODS_ACTIVE_SUC_SIZE + EN_AODS_AODC_MSG_SIZE);
};