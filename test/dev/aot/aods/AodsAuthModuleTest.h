#ifndef AodsAuthModuleTest_H__
#define AodsAuthModuleTest_H__
#include "../../../../src/dev/util/Module.h"
#include "../../../../src/dev/util/AoEvent.h"
class CAodsAuthModuleTest :public CModule {
public:
    CAodsAuthModuleTest();
    ~CAodsAuthModuleTest();
    bool TestSuc();
    bool TestFai();
protected:
    bool constructAuthFile();
protected:    
    void OnRecvAuth(const MT_MSG* pMM);
    void OnRecvAuthSuc(const MT_MSG* pMM);
    void OnRecvQuit(const MT_MSG* pMM);
    DECLARE_AO_MSG_MAP()
protected:
    bool m_bTestSuc;
    bool m_bResult;
    CAoEvent m_evSignal;
};
#endif // AodsAuthModuleTest_H__