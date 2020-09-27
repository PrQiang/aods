/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoThread_h__
#define AoThread_h__
#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
class CAoThread{
protected:
    CAoThread();
    virtual ~CAoThread();
public:
	int Start();
	void Join();
	virtual int Run();
protected:
#ifdef WIN32
	HANDLE m_hThread;
#else
	pthread_t m_nThread;
    pthread_attr_t m_attr;
#endif
};
#endif // AoThread_h__