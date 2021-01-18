/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef ModuleData_h__
#define ModuleData_h__
class CModuleData{
public:
    CModuleData();
    ~CModuleData();
	bool Malloc(unsigned int unSize);
	unsigned int  Capacity();
	unsigned int DataLen();
	const unsigned char* Data();
	bool AppendData(const unsigned char* pucData, unsigned int unLen);
	void Reset();
protected:
	unsigned char* m_pucData;
	unsigned int m_unCapacity;
	unsigned int m_unLen;
};
#endif // ModuleData_h__