/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoMemoryMgr_h__
#define AoMemoryMgr_h__
#include <list>
class CAoLock;
class CAoMemoryMgr{
public:
    CAoMemoryMgr();
    ~CAoMemoryMgr();
	void* Malloc(int nSize);
	void Free(void* pData);
	void* Realloc(void* pData, int nNewSize);
	static CAoMemoryMgr* Instance();
protected:
	enum{
		EN_MEM_INDEX_0512 = 0, // 512Bytes
		EN_MEM_INDEX_1024, // 1KB
		EN_MEM_INDEX_2048, // 2KB
		EN_MEM_INDEX_4096, // 4KB
		EN_MEM_INDEX_8192, // 8KB
		EN_MEM_INDEX_16KB, // 16KB
		EN_MEM_INDEX_32KB, // 32KB
		EN_MEM_INDEX_64KB, // 64KB
		EN_MEM_INDEX_128K, // 128KB
		EN_MEM_INDEX_01MB, // 1MB
		EN_MEM_INDEX_02MB, // 2MB
		EN_MEM_INDEX_08MB, // 8MB
		EN_MEM_INDEX_COUN, // ×ÜÊý
	};
	enum{
		EN_MEM_SIZE_0512 = 512, // 512Bytes
		EN_MEM_SIZE_1024 = 1024, // 1KB
		EN_MEM_SIZE_2048 = 2048, // 2KB
		EN_MEM_SIZE_4096 = 4096, // 4KB
		EN_MEM_SIZE_8192 = 8192, // 8KB
		EN_MEM_SIZE_16KB = 16*1024, // 16KB
		EN_MEM_SIZE_32KB = 32*1024, // 32KB
		EN_MEM_SIZE_64KB = 64*1024, // 64KB
		EN_MEM_SIZE_128K = 128*1024, // 128KB
		EN_MEM_SIZE_01MB = 1024*1024, // 1MB
		EN_MEM_SIZE_02MB = 2*1024*1024, // 2MB
		EN_MEM_SIZE_08MB = 8*1024*1024 // 8MB
	};
	enum{
		EN_MEM_COUNT_0512 = 10240,
		EN_MEM_COUNT_1024 = 10240, // 1KB
		EN_MEM_COUNT_2048 = 10240, // 2KB
		EN_MEM_COUNT_4096 = 10240, // 4KB
		EN_MEM_COUNT_8192 = 10240, // 8KB
		EN_MEM_COUNT_16KB = 1024, // 16KB
		EN_MEM_COUNT_32KB = 1024, // 32KB
		EN_MEM_COUNT_64KB = 1024, // 64KB
		EN_MEM_COUNT_128K = 1024, // 128KB
		EN_MEM_COUNT_01MB = 32, // 1MB
		EN_MEM_COUNT_02MB = 32, // 2MB
		EN_MEM_COUNT_08MB = 16  // 2MB
	};
	typedef std::list<char*> MEM_LST;
	typedef MEM_LST::iterator MEM_LST_IT;
	MEM_LST m_lstMem[EN_MEM_INDEX_COUN];
#ifdef _DEBUG
	MEM_LST m_lstUsedMem[EN_MEM_INDEX_COUN];
#endif
	int m_nSize[EN_MEM_INDEX_COUN];
	CAoLock* m_pLock;
};
#endif // AoMemoryMgr_h__