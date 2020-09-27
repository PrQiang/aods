/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include "AoTime.h"
#include "Module.h"
#include "MsgTarget.h"

MSG_FUN_ENTRY CMsgTarget::ms_MsgFunEntry = {NULL, NULL};
MSG_FUN CMsgTarget::ms_MsgFun[] = {
	ON_AO_MSG_FUN(EN_AO_MSG_ONTIMER, &CMsgTarget::OnTimer)
END_AO_MSG_MAP()
CMsgTarget::CMsgTarget(unsigned int unID){m_unObjId = unID;m_pOwner  = NULL;sprintf(m_szName, "%s", "消息处理目标模块");}
CMsgTarget::~CMsgTarget(){}
void CMsgTarget::Attach( CModule* pOwner ){	m_pOwner = pOwner;}
void CMsgTarget::Detach(){	m_pOwner = NULL;}
void CMsgTarget::SetTimer( unsigned int unTimerID, unsigned int unEscape, CMsgTarget* pTimerObj, unsigned int unTimes ){
	if (NULL == m_pOwner){return;}
	m_pOwner->SetTimer(unTimerID, unEscape, unTimes, pTimerObj);
}
void CMsgTarget::KillTimer( CMsgTarget* pTimerObj, unsigned int unTimerID){
	if (NULL == m_pOwner){return;}
	m_pOwner->KillTimer(unTimerID, pTimerObj);
}
unsigned int CMsgTarget::ID(){	return m_unObjId;}
MSG_FUN_ENTRY* CMsgTarget::GetMsgFunEntry(){
	static MSG_FUN_ENTRY mfe = {ms_MsgFun, NULL};
	return &mfe;
}
pMsgFun CMsgTarget::FindMsgFunByMsgCode( unsigned int unMsgCode ){
	MSG_FUN_ENTRY* pMMFE = GetMsgFunEntry();
	MSG_FUN* pMMF = NULL;
	while (NULL != pMMFE){
		pMMF = pMMFE->pMMFs;
		while(NULL != pMMF && (0 != pMMF->unMsgCode) && (NULL != pMMF->pFun)){
			if (pMMF->unMsgCode == unMsgCode){return pMMF->pFun;}
			++pMMF;
		}
		pMMFE = pMMFE->pParentMMFE;
	}
	return NULL;
}
void CMsgTarget::OnMessage( const MT_MSG* pMM ){
	pMsgFun pFun = FindMsgFunByMsgCode(pMM->unMsgCode);
	if (NULL == pFun){return;}
	(this->*pFun)(pMM);
}
bool CMsgTarget::SendOutMsg( unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData /*= NULL*/, unsigned int unLen /*= 0*/, CMsgTarget* pRecver /*= NULL*/, unsigned int unSMI ){
	if (NULL == m_pOwner)	{		return false;	}
	return m_pOwner->SendOutMsg(unRMI, unMsgCode, pucData, unLen, pRecver, this, unSMI);
}
void CMsgTarget::OnTimer( const MT_MSG* pMM ){
	MSG_TIMER* pMT = (MSG_TIMER*)pMM->ucData;
	OnTimer(pMT->unTimerId);
}
void CMsgTarget::OnTimer( unsigned int  ){}
const char* CMsgTarget::Name(){	return m_szName;}