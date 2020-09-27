/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoTime_h__
#define AoTime_h__
class CAoTime{
public:
    CAoTime();
    ~CAoTime();
	static int CurrentTick();
	void GetTime();
public:
	int m_nYear;    // 年
	int m_nMonth;   // 月
	int m_nDay;     // 日
	int m_nHour;    // 时
	int m_nMinute;  // 分
	int m_nSecond;  // 秒
	int m_nMilliSecond; // 毫秒
};
#endif // AoTime_h__