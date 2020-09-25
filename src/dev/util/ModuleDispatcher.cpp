/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoDef.h"
#include "Module.h"
#include "ModuleDispatcher.h"
CModuleDispatcher* CModuleDispatcher::ms_pInstance = NULL;
CAoLock CModuleDispatcher::ms_Lock;
CModuleDispatcher::CModuleDispatcher(){}
CModuleDispatcher::~CModuleDispatcher(){}
CModuleDispatcher* CModuleDispatcher::Instance(){
    if(NULL == ms_pInstance){
         ms_Lock.Lock();
         ms_pInstance = (NULL == ms_pInstance) ? new CModuleDispatcher : ms_pInstance;
         ms_Lock.Unlock();
    }
    return ms_pInstance;
}
void CModuleDispatcher::Release(){SAFE_DELETE(ms_pInstance);}
void CModuleDispatcher::AppendModule( CModule* pModule ){m_mapIM[pModule->ID()] = pModule;}
void CModuleDispatcher::AppendMonitorModule( CModule* pModule ){m_mapMIM[pModule->ID()]  = pModule;}
bool CModuleDispatcher::Dispatch( unsigned int unRMI, unsigned int unMsgCode, const unsigned char* pucData, unsigned int unLen, CMsgTarget* pRecver, CMsgTarget* pSender, unsigned int unSMI ){
	for (IM_MAP_IT it = m_mapMIM.begin(); it != m_mapMIM.end(); ++it){
		it->second->PostMsg(unMsgCode, pucData, unLen, pRecver, pSender, unSMI);
	}	
	if (unRMI == EN_MODULE_ID_BROADCAST){// ¹ã²¥ÏûÏ¢
		for (IM_MAP_IT it = m_mapIM.begin(); it != m_mapIM.end(); ++it){
			it->second->PostMsg(unMsgCode, pucData, unLen, pRecver, pSender, unSMI);
		}
		return true;
	}
	IM_MAP_IT it = m_mapIM.find(unRMI);
	if (m_mapIM.end() == it){return false;}
	return it->second->PostMsg(unMsgCode, pucData, unLen, pRecver, pSender, unSMI);
}