#ifndef AodsActiveModuleTestSuc_H__
#define AodsActiveModuleTestSuc_H__
#include "../../../../src/dev/util/Module.h"
struct MT_MSG;
class CAodsActiveModuleTestSuc : public CModule {
public:
    CAodsActiveModuleTestSuc();
    ~CAodsActiveModuleTestSuc();
    bool Test();

protected:
    void OnRecvActive(const MT_MSG* pMM);

    DECLARE_AO_MSG_MAP()

private:
    unsigned char m_ucIndex; // 测试用的随机id
    bool m_bTestActSuc; // 测试激活成功
    bool m_bResult;
};
#endif // AodsActiveModuleTestSuc_H__



