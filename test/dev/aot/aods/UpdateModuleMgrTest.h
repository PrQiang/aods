#ifndef UpdateModuleMgrTest_H__
#define UpdateModuleMgrTest_H__
#include "../../../../src/dev/util/Module.h"
class CUpdateModuleMgrTest:public CModule {
public:
    CUpdateModuleMgrTest();
    ~CUpdateModuleMgrTest();
    bool TestSuc();
    bool TestFail1();
    bool TestFail2();
    bool TestFail3();
    bool TestFail4();
    bool TestFail5();
protected:
    void OnRecvData(const MT_MSG* pMM);
    DECLARE_AO_MSG_MAP()
protected:
    bool m_bSuc0;
    CAoEvent m_evSuc0;
    bool m_bFai1;
    CAoEvent m_evFai1;
    bool m_bFai2;
    CAoEvent m_evFai2;
    bool m_bFai3;
    CAoEvent m_evFai3;
    bool m_bFai4;
    CAoEvent m_evFai4;
    bool m_bFai5;
    CAoEvent m_evFai5;
};
#endif