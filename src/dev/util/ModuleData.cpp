/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include <stdlib.h>
#include <memory>
#include "AoDef.h"
#include "ModuleData.h"
CModuleData::CModuleData(){
	m_pucData = NULL;
	m_unCapacity = 0;
	m_unLen = 0;
}
CModuleData::~CModuleData(){
	SAFE_FREE(m_pucData);
}
bool CModuleData::Malloc( unsigned int unSize ){
	if (unSize <= m_unCapacity){
		memset(m_pucData, 0, m_unCapacity);
		return true;
	}
	SAFE_FREE(m_pucData);
	m_pucData = (unsigned char*)MALLOC(unSize);
	if (NULL == m_pucData){return false;}
	m_unCapacity = unSize;
	memset(m_pucData, 0, m_unCapacity);
	return true;
}
unsigned int CModuleData::Capacity(){	return m_unCapacity;}
unsigned int CModuleData::DataLen(){	return m_unLen;}
const unsigned char* CModuleData::Data(){	return m_pucData;}
bool CModuleData::AppendData( const unsigned char* pucData, unsigned int unLen ){
	if (unLen + m_unLen > m_unCapacity){return false;}
	memcpy(m_pucData + m_unLen, pucData, unLen);
	m_unLen += unLen;
	return true;
}
void CModuleData::Reset(){	m_unLen = 0;}