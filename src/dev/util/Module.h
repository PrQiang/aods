#ifndef Module_h__
#define Module_h__
#include <list>
#include <map>
#include "AoLock.h"
#include "AoEvent.h"
#include "AoThread.h"
#include "MsgTarget.h"
class CModule;
class CModuleData;
class CModule : public CMsgTarget, public CAoThread{
public:
	friend class CMsgTarget;
    CModule(unsigned int unMID, unsigned int unInitLen, const char* pszModuleName);
    virtual ~CModule();
	virtual int Run();
	virtual bool PostMsg(unsigned int unMsgCode, const unsigned char* pucData = NULL, unsigned int unLen = 0, CMsgTarget* pRecver = NULL, CMsgTarget* pSender = NULL, unsigned int unSMI = 0xFFFFFFFF);
	virtual bool SendOutMsg(unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData = NULL, unsigned int unLen = 0, CMsgTarget* pRecver = NULL, CMsgTarget* pSender = NULL, unsigned int unSMI = 0xFFFFFFFF);
	bool SetTimer(unsigned int unTimerID, unsigned int unEscape, unsigned int unTimes, CMsgTarget* pTimerObj);
	bool KillTimer(unsigned int unTimerID, CMsgTarget* pTimerObj);
	virtual bool Start();
    virtual void Stop();
	virtual void LogInfo(const char* pszSubModuleName, const char* pszDescFormat, ...);
	virtual void LogDebug(const char* pszSubModuleName, const char* pszDescFormat, ...);
	virtual void LogWarn(const char* pszSubModuleName, const char* pszDescFormat, ...);
	virtual void LogErr(const char* pszSubModuleName, const char* pszDescFormat, ...);
	virtual void LogData(const char* pszSubModuleName, const char* pszData, int nLen);
protected:	
	void Append(CMsgTarget* pMT);
	void Remove(CMsgTarget*  pMT);
	unsigned int ProcTimer();
	void DoMsg(MT_MSG* pMM);
	virtual unsigned char BitStatus();
protected:
	void OnSetTimer(const MT_MSG* pMM);
	void OnKillTimer(const MT_MSG* pMM);
	void OnStop(const MT_MSG* pMM);
	void OnBit(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
protected:
	typedef std::list<MSG_TIMER*> MT_LST;
	typedef MT_LST::iterator MT_LST_IT;
	MT_LST m_lstMT;
	typedef std::map<unsigned int, CMsgTarget*> MT_MAP;
	typedef MT_MAP::iterator MT_MAP_IT;
	MT_MAP m_mapMT;
	CModuleData* m_pCusMD;
	CModuleData* m_pProductMD; // 生产数据
	bool m_bRun;
	CAoLock m_csLock; // 消息同步锁
	CAoEvent m_evWait;  // 消息等待
	enum{
		EN_MAX_TEMP_FORMAT_BUF_LEN = 1024 * 1024
	};
	char* m_pszTempFormatBuf; // 临时buf
};
#endif // Module_h__