/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include "AoDef.h"
#include "AoThread.h"
#ifdef WIN32
#include <process.h>
static unsigned __stdcall ThreadRun(void* p){return ((CAoThread*)p)->Run();}
#else
#include <unistd.h>
static void* ThreadRun(void* p){
    int* pnRet = new int;
    *pnRet = ((CAoThread*)p)->Run();
    return (void*)pnRet;
}
#endif
CAoThread::CAoThread(){
#ifdef WIN32
	m_hThread = NULL;
#else
	m_nThread = 0xFFFFFFFFFFFFFFFF;
    pthread_attr_init(&m_attr);
#endif
}
CAoThread::~CAoThread(){
#ifdef WIN32
	if(NULL != m_hThread){	
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
#elif defined LINUX
    pthread_attr_destroy(&m_attr);
#endif
}
int CAoThread::Start(){
#ifdef WIN32
	m_hThread = (HANDLE)_beginthreadex(NULL, 1024*1024, ThreadRun, this, 1, NULL);
	return (NULL == m_hThread) ? -1 : 0;
#elif defined LINUX
    pthread_attr_setstacksize(&m_attr, 1024*1024);
    return pthread_create(&m_nThread, &m_attr, &ThreadRun, (void*)this);
#endif
}
void CAoThread::Join(){
#ifdef WIN32
	if(NULL != m_hThread){
		WaitForSingleObject(m_hThread, INFINITE);
	}	
#else
	if(0xFFFFFFFFFFFFFFFF != m_nThread){
		int* pv = NULL;
		pthread_join(m_nThread, (void**)&pv);
		if(NULL != pv){delete pv;}
	}
#endif
}
int CAoThread::Run(){	return 0;}
