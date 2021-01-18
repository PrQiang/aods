/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef MsgTarget_h__
#define MsgTarget_h__
#include "AoDef.h"

// 类型应用声明
class CModule;
class CMsgTarget;
// 定义消息处理映射
typedef void (CMsgTarget::*pMsgFun)(const MT_MSG*);
struct MSG_FUN{
	pMsgFun pFun;
	unsigned int  unMsgCode;
};
struct MSG_FUN_ENTRY{
	MSG_FUN* pMMFs;
	MSG_FUN_ENTRY* pParentMMFE;
};
#define DECLARE_AO_MSG_MAP() \
	static MSG_FUN ms_MsgFun[];\
	virtual MSG_FUN_ENTRY* GetMsgFunEntry();

#define BEGIN_AO_MSG_MAP(Class, BaseClass) \
	MSG_FUN_ENTRY* Class::GetMsgFunEntry()\
{\
	static MSG_FUN_ENTRY sMsgFunEntry = {Class::ms_MsgFun, BaseClass::GetMsgFunEntry()};\
	return &sMsgFunEntry;\
}\
	MSG_FUN Class::ms_MsgFun[] = \
{

#define ON_AO_MSG_FUN(MsgCode, Fun) \
{static_cast<pMsgFun>(Fun), MsgCode},

#define END_AO_MSG_MAP() \
{NULL, NULL}};
class CMsgTarget{
public:
	friend class CModule;
    CMsgTarget(unsigned int unID);
    virtual ~CMsgTarget();
	unsigned int ID();
	bool SendOutMsg(unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData = NULL, unsigned int unLen = 0, CMsgTarget* pRecver = NULL, unsigned int unSMI = 0xFFFFFFFF);
	virtual void OnMessage(const MT_MSG* pMM);
	const char* Name();
protected:
	void Attach(CModule* pOwner);
	void Detach();
	void SetTimer(unsigned int unTimerID, unsigned int unEscape, CMsgTarget* pTimerObj, unsigned int unTimes = 0xFFFFFFFF);
	void KillTimer(CMsgTarget* pTimerObj, unsigned int unTimerID = 0xFFFFFFFF);
	virtual MSG_FUN_ENTRY* GetMsgFunEntry();
	pMsgFun FindMsgFunByMsgCode(unsigned int unMsgCode);
	void OnTimer(const MT_MSG* pMM);
	virtual void OnTimer(unsigned int unTimerID);
protected:
	enum{
		EN_MAX_TEXT_LEN = 1024
	};
	static MSG_FUN_ENTRY ms_MsgFunEntry;
	static MSG_FUN ms_MsgFun[];
	CModule* m_pOwner;
	unsigned int m_unObjId;
	char m_szName[EN_MAX_TEXT_LEN];
};
#endif // MsgTarget_h__