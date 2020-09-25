/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "AoDef.h"
#include "AoTime.h"
#include "Module.h"
#include "ModuleData.h"
#include "ModuleDispatcher.h"
BEGIN_AO_MSG_MAP(CModule, CMsgTarget)
	ON_AO_MSG_FUN(EN_AO_MSG_SETTIMER, &CModule::OnSetTimer)
	ON_AO_MSG_FUN(EN_AO_MSG_KILLTIMER, &CModule::OnKillTimer)
	ON_AO_MSG_FUN(EN_AO_MSG_BIT, &CModule::OnBit)
	ON_AO_MSG_FUN(EN_AO_MSG_EXIT, &CModule::OnStop)
END_AO_MSG_MAP()
CModule::CModule( unsigned int unMID, unsigned int unInitLen, const char* pszModuleName )
	:CMsgTarget(unMID){
	m_evWait.Reset();
	m_pCusMD = new CModuleData();
	m_pCusMD->Malloc(unInitLen);
	m_pProductMD = new CModuleData();
	m_pProductMD->Malloc(unInitLen);	
	strcpy(m_szName, pszModuleName);
	m_bRun = true;
	m_pszTempFormatBuf = (char*)MALLOC(EN_MAX_TEMP_FORMAT_BUF_LEN);
	Append(this);
}
CModule::~CModule(){
	SAFE_DELETE(m_pCusMD);
	SAFE_DELETE(m_pProductMD);
	SAFE_FREE(m_pszTempFormatBuf);
	for (MT_LST_IT it = m_lstMT.begin(); it != m_lstMT.end(); ++it){delete (*it);}
	m_lstMT.clear();
}
int CModule::Run(){
	unsigned int unLen = 0;
	unsigned int unOffset = 0;
	unsigned int unMin = 0xFFFFFFFF;
	MT_MSG* pMM = NULL;	
	CModuleData* pMD = NULL;
	const unsigned char* pucData =  NULL;
	while(m_bRun)	{
		m_evWait.Wait(unMin);
		m_csLock.Lock();
		pMD = m_pCusMD;
		m_pCusMD = m_pProductMD;
		m_pProductMD = pMD;
		m_pProductMD->Reset();
		m_csLock.Unlock();
		pucData = m_pCusMD->Data();
		unLen = m_pCusMD->DataLen();
		for (unOffset = 0; unOffset < unLen && m_bRun; unOffset += pMM->unDataLen + EN_MT_MSG_HEAD_SIZE)		{
			pMM = (MT_MSG*)(pucData + unOffset);
			DoMsg(pMM);
		}
		unMin = ProcTimer();	
	}
	return 0;
}
bool CModule::PostMsg( unsigned int unMsgCode, const unsigned char* pucData, unsigned int unLen, CMsgTarget* pRecver, CMsgTarget* pSender, unsigned int unSMI ){
	MT_MSG_HEAD mmh;
	mmh.unSMI = unSMI;
	mmh.pRecver = (NULL == pRecver) ? this : pRecver;
	mmh.pSender = pSender;
	mmh.unDataLen = unLen;
	mmh.unMsgCode = unMsgCode;
	if (!m_bRun){return false;}
	m_csLock.Lock();
	bool bRet =  m_pProductMD->AppendData((unsigned char*)&mmh, EN_MT_MSG_HEAD_SIZE);
	if(bRet && NULL != pucData && mmh.unDataLen > 0){
		bRet = m_pProductMD->AppendData(pucData, mmh.unDataLen);
	}
	m_csLock.Unlock();
	m_evWait.Signal();
	return bRet;
}
bool CModule::SendOutMsg( unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData, unsigned int unLen, CMsgTarget* pRecver, CMsgTarget* pSender, unsigned int unSMI){
	return CModuleDispatcher::Instance()->Dispatch(unRMI, unMsgCode, pucData, unLen, pRecver, pSender, unSMI);
}
bool CModule::Start(){
	m_csLock.Lock();
	m_pCusMD->Reset();
	m_pProductMD->Reset();
	m_csLock.Unlock();
	return CAoThread::Start() == 0;
}
void CModule::Stop(){
	PostMsg(EN_AO_MSG_STOP);
	CAoThread::Join();
}
bool CModule::SetTimer( unsigned int unTimerID, unsigned int unEscape, unsigned int unTimes, CMsgTarget* pTimerObj ){
	if (unEscape < 10){return false;} // 定时器计数器不能低于10ms,否则不执行
	MSG_TIMER mt;
	mt.unTimerId = unTimerID;
	mt.unEscape = unEscape;
	mt.unTime = CAoTime::CurrentTick();
	mt.unTimes = unTimes;
	mt.pMT = pTimerObj;
	return PostMsg(EN_AO_MSG_SETTIMER, (unsigned char*)&mt, EN_MSG_TIMER_SIZE, this, pTimerObj);
}
bool CModule::KillTimer( unsigned int unTimerID, CMsgTarget* pTimerObj ){
	MSG_TIMER mt;
	mt.unTimerId = unTimerID;
	mt.unEscape = 0;
	mt.unTime = CAoTime::CurrentTick();
	mt.unTimes = 0;
	mt.pMT = pTimerObj;
	return PostMsg(EN_AO_MSG_KILLTIMER, (unsigned char*)&mt, EN_MSG_TIMER_SIZE, this, pTimerObj);
}
void CModule::Append( CMsgTarget* pMT ){
	MT_MAP_IT it = m_mapMT.find(pMT->ID());
	if (m_mapMT.end() == it){m_mapMT[pMT->ID()] = pMT;	return;	}
}
void CModule::Remove( CMsgTarget* pMT ){
	MT_MAP_IT it = m_mapMT.find(pMT->ID());
	if (m_mapMT.end() == it){return;}
	m_mapMT.erase(it);
}
unsigned int CModule::ProcTimer(){
	MSG_TIMER* pMT = NULL;	
	unsigned int unCurTime = CAoTime::CurrentTick();
    unsigned int unMinSleepTime = 100000; // 误差为15~16ms
	int nDifTime = 0; // 节点超时时间与当前时间只差
	for (MT_LST_IT it = m_lstMT.begin(); it != m_lstMT.end(); ){
		pMT = *it;
		if (0 == pMT->unTimes){
			SAFE_DELETE(pMT);
			it = m_lstMT.erase(it);
			continue;
		}
		// 记录时间与周期时间之和小于当前时间，则响应对象定时处理
		nDifTime = (int)(pMT->unTime + pMT->unEscape - unCurTime);
		if (nDifTime <= 0){
			// 执行定时器消息
			PostMsg(EN_AO_MSG_ONTIMER, (unsigned char*)pMT, EN_MSG_TIMER_SIZE, pMT->pMT, pMT->pMT);
			pMT->unTime = unCurTime;
			nDifTime = (int)pMT->unEscape;
			pMT->unTimes = (0xFFFFFFFF != pMT->unTimes) ? pMT->unTimes - 1 : pMT->unTime;
		}
		unMinSleepTime = (unMinSleepTime < (unsigned int)nDifTime) ? unMinSleepTime : (unsigned int)nDifTime;
		++it;
	}
	return unMinSleepTime;
}
void CModule::DoMsg( MT_MSG* pMM ){
	MT_MAP_IT it = m_mapMT.find(pMM->pRecver->ID());
	if (m_mapMT.end() == it || it->second != pMM->pRecver){return;}
	pMM->pRecver->OnMessage(pMM);
}
void CModule::OnSetTimer( const MT_MSG* pMM ){
	if (EN_MSG_TIMER_SIZE != pMM->unDataLen){return;}
	// 检测定时器是否存在，若不存在则更新数据即可
	MSG_TIMER* pOM = NULL;
	MSG_TIMER* pMT = (MSG_TIMER*)pMM->ucData;
	for (MT_LST_IT it = m_lstMT.begin(); it != m_lstMT.end(); ++it){
		pOM = *it;
		if ((pOM->pMT == pMT->pMT) && (pMT->unTimerId == pOM->unTimerId)){
			pOM->unEscape = pMT->unEscape;
			pOM->unTime = pMT->unTime;
			pOM->unTimes = pMT->unTimes;
			return;
		}
	}
	pMT = new MSG_TIMER;
	memcpy(pMT, pMM->ucData, pMM->unDataLen);
	m_lstMT.push_back(pMT);
}
void CModule::OnKillTimer( const MT_MSG* pMM ){
	MSG_TIMER* pOM = NULL;
	MSG_TIMER* pMT = (MSG_TIMER*)pMM->ucData;
	for (MT_LST_IT it = m_lstMT.begin(); it != m_lstMT.end();){
		pOM = *it;
		if ((pOM->pMT != pMT->pMT) || ((0xFFFFFFFF != pMT->unTimerId) && (pMT->unTimerId != pOM->unTimerId))){
			++it;
			continue;
		}
		SAFE_DELETE(pOM);
		it = m_lstMT.erase(it);
	}
}
void CModule::OnStop( const MT_MSG*  ){
	for (MT_MAP_IT it = m_mapMT.begin(); it != m_mapMT.end(); ++it){it->second->Detach();}
	m_mapMT.clear();
	for (MT_LST_IT it = m_lstMT.begin(); it != m_lstMT.end(); ++it){SAFE_DELETE(*it);}
	m_lstMT.clear();
	m_bRun = false;
}
void CModule::OnBit( const MT_MSG* pMM ){
	AO_MSG_BIT_RESULT ambr;
	ambr.ucModuleID = (unsigned char)ID();
	ambr.ucResult = BitStatus();
	SendOutMsg(pMM->unSMI, EN_AO_MSG_BIT_RLT, (unsigned char*)&ambr, EN_AO_MSG_BIT_RESULT_SIZE, pMM->pSender, this);
}
unsigned char CModule::BitStatus(){	return 0;}
void CModule::LogInfo(const char* pszSubModuleName, const char* pszDescFormat, ...){
	va_list arglist;
	va_start(arglist, pszDescFormat);
	strcpy(m_pszTempFormatBuf, m_szName); // 模块名称
	unsigned int unLen = (unsigned int)strlen(m_szName) + 1;
	strcpy(m_pszTempFormatBuf + unLen, pszSubModuleName); // 子模块名称
	unLen += (unsigned int)strlen(pszSubModuleName) + 1;
	vsnprintf(m_pszTempFormatBuf + unLen, EN_MAX_TEMP_FORMAT_BUF_LEN - unLen, pszDescFormat, arglist);
	va_end(arglist);
	unLen += (unsigned int)strlen(m_pszTempFormatBuf + unLen) + 1;
	// 将数据转换为问本内容
	SendOutMsg(EN_BASE_MODULE_ID_LOG, EN_LOG_MODULE_LOG_INFO, (unsigned char*)m_pszTempFormatBuf, unLen);
}
void CModule::LogDebug(const char* pszSubModuleName, const char* pszDescFormat, ...){
	va_list arglist;
	va_start(arglist, pszDescFormat);
	strcpy(m_pszTempFormatBuf, m_szName); // 模块名称
	unsigned int unLen = (unsigned int)strlen(m_szName) + 1;
	strcpy(m_pszTempFormatBuf + unLen, pszSubModuleName); // 子模块名称
	unLen += (unsigned int)strlen(pszSubModuleName) + 1;
	vsnprintf(m_pszTempFormatBuf + unLen, EN_MAX_TEMP_FORMAT_BUF_LEN - unLen, pszDescFormat, arglist);
	va_end(arglist);
	unLen += (unsigned int)strlen(m_pszTempFormatBuf + unLen) + 1;
	// 将数据转换为问本内容
	SendOutMsg(EN_BASE_MODULE_ID_LOG, EN_LOG_MODULE_LOG_DEBUG, (unsigned char*)m_pszTempFormatBuf, unLen);
}
void CModule::LogWarn(const char* pszSubModuleName, const char* pszDescFormat, ...){
	va_list arglist;
	va_start(arglist, pszDescFormat);
	strcpy(m_pszTempFormatBuf, m_szName); // 模块名称
	unsigned int unLen = (unsigned int)strlen(m_szName) + 1;
	strcpy(m_pszTempFormatBuf + unLen, pszSubModuleName); // 子模块名称
	unLen += (unsigned int)strlen(pszSubModuleName) + 1;
	vsnprintf(m_pszTempFormatBuf + unLen, EN_MAX_TEMP_FORMAT_BUF_LEN - unLen, pszDescFormat, arglist);
	va_end(arglist);
	unLen += (unsigned int)strlen(m_pszTempFormatBuf + unLen) + 1;
	// 将数据转换为问本内容
	SendOutMsg(EN_BASE_MODULE_ID_LOG, EN_LOG_MODULE_LOG_WARN, (unsigned char*)m_pszTempFormatBuf, unLen);
}
void CModule::LogErr(const char* pszSubModuleName, const char* pszDescFormat, ...){
	va_list arglist;
	va_start(arglist, pszDescFormat);
	strcpy(m_pszTempFormatBuf, m_szName); // 模块名称
	unsigned int unLen = (unsigned int)strlen(m_szName) + 1;
	strcpy(m_pszTempFormatBuf + unLen, pszSubModuleName); // 子模块名称
	unLen += (unsigned int)strlen(pszSubModuleName) + 1;
	vsnprintf(m_pszTempFormatBuf + unLen, EN_MAX_TEMP_FORMAT_BUF_LEN - unLen, pszDescFormat, arglist);
	va_end(arglist);
	unLen += (unsigned int)strlen(m_pszTempFormatBuf + unLen) + 1;
	SendOutMsg(EN_BASE_MODULE_ID_LOG, EN_LOG_MODULE_LOG_ERR, (unsigned char*)m_pszTempFormatBuf, unLen);
}
void CModule::LogData(const char* pszSubModuleName, const char* pszData, int nLen){
	strcpy(m_pszTempFormatBuf, m_szName); // 模块名称
	unsigned int unLen = (unsigned int)strlen(m_szName) + 1;
	strcpy(m_pszTempFormatBuf + unLen, pszSubModuleName); // 子模块名称
	unLen += (unsigned int)strlen(pszSubModuleName) + 1;
	memcpy(m_pszTempFormatBuf + unLen, pszData, nLen);
	unLen += (unsigned int)nLen;
	SendOutMsg(EN_BASE_MODULE_ID_LOG, EN_LOG_MODULE_LOG_DATA, (unsigned char*)m_pszTempFormatBuf, unLen);
}