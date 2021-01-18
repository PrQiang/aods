/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoTime.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#endif
CAoTime::CAoTime(){}
CAoTime::~CAoTime(){}
int CAoTime::CurrentTick(){
#ifdef WIN32
	return GetTickCount();
#else
    struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}
void CAoTime::GetTime(){
#ifdef WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_nYear = st.wYear;
	m_nMonth = st.wMonth;
	m_nDay = st.wDay;
	m_nHour = st.wHour;
	m_nMinute = st.wMinute;
	m_nSecond = st.wSecond;
	m_nMilliSecond = st.wMilliseconds;
#elif defined LINUX
	time_t curr_time;
    time(&curr_time);
	tm *tm_local = localtime(&curr_time); 
	struct timeval tv;
	gettimeofday(&tv,NULL);
	m_nYear = tm_local->tm_year + 1900; // 加上起始年份
	m_nMonth = tm_local->tm_mon + 1;
    m_nDay = tm_local->tm_mday;
	m_nHour = tm_local->tm_hour;
	m_nMinute = tm_local->tm_min;
	m_nSecond = tm_local->tm_sec;
	m_nMilliSecond = ((tv.tv_usec / 1000) % 1000);
#endif
}