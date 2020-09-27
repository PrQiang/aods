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
    unsigned char m_ucIndex; // �����õ����id
    bool m_bTestActSuc; // ���Լ���ɹ�
    bool m_bResult;
};
#endif // AodsActiveModuleTestSuc_H__



