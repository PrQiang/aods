/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoEvent_h__
#define AoEvent_h__
class CAoEvent{
public:
    CAoEvent();
    ~CAoEvent();
	void Reset();
	void Signal();
	int Wait(int nMilSec);
protected:
	void* m_pHandle;
};
#endif // AoEvent_h__