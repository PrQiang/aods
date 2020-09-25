/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoLock_h__
#define AoLock_h__
#ifdef WIN32
#include <Windows.h>
#elif defined LINUX
#include<pthread.h>
#endif
class CAoLock{
public:
    CAoLock();
    ~CAoLock();
	void Lock();
	void Unlock();
protected:
#ifdef WIN32
	CRITICAL_SECTION m_csLock;
#elif defined LINUX
    pthread_mutex_t  m_mutex_lock;
#endif
};

#endif // AoLock_h__