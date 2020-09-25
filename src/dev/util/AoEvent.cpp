/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoEvent.h"
#ifdef WIN32
#include <stdio.h>
#include <Windows.h>
#else
#include <time.h>
#include <semaphore.h>
#endif
CAoEvent::CAoEvent(){
#ifdef WIN32
	m_pHandle = ::CreateEvent(NULL, FALSE, TRUE, NULL);
#else
	m_pHandle = new sem_t;
	sem_init((sem_t*)m_pHandle, 0, 2);
#endif
	Reset();
}
CAoEvent::~CAoEvent(){
	if(NULL != m_pHandle){
#ifdef WIN32
		CloseHandle(m_pHandle);
#else
		sem_destroy((sem_t*)m_pHandle);
		delete (sem_t*)m_pHandle;
#endif
		m_pHandle = NULL;
	}
}
void CAoEvent::Reset(){
	if(NULL != m_pHandle){
#ifdef WIN32
	::ResetEvent(m_pHandle);
#else
#endif
	}
}
void CAoEvent::Signal(){
	if(NULL != m_pHandle){
#ifdef WIN32
		::SetEvent(m_pHandle);
#else
		sem_post((sem_t*)m_pHandle);
#endif
	}
}

int CAoEvent::Wait( int nMilSec ){
	if(NULL != m_pHandle){
#ifdef WIN32
		return WaitForSingleObject(m_pHandle, nMilSec);
#else
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME, &ts );
		ts.tv_sec += nMilSec / 1000;
		ts.tv_nsec += (nMilSec % 1000) * 1000000;
		return sem_timedwait((sem_t*)m_pHandle, &ts);
#endif
	}
	return 0;
}