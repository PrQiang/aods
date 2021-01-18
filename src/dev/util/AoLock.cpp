/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoLock.h"
CAoLock::CAoLock(){
#ifdef WIN32
	InitializeCriticalSection(&m_csLock);
#elif defined LINUX
    pthread_mutex_init(&m_mutex_lock, NULL);
#endif
}
CAoLock::~CAoLock(){
#ifdef WIN32
	DeleteCriticalSection(&m_csLock);
#elif defined LINUX
    pthread_mutex_destroy(&m_mutex_lock);
#endif
}
void CAoLock::Lock(){
#ifdef WIN32
	EnterCriticalSection(&m_csLock);
#elif defined LINUX
    pthread_mutex_lock(&m_mutex_lock);
#endif
}
void CAoLock::Unlock(){
#ifdef WIN32
	LeaveCriticalSection(&m_csLock);
#elif defined LINUX
    pthread_mutex_unlock(&m_mutex_lock);
#endif
}