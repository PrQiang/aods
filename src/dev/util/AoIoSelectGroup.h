/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoIoSelectGroup_h__
#define AoIoSelectGroup_h__
#include <list>
#include "AoThread.h"
class CAoLock;
class CAoEvent;
class CAoSockChannel;
class CAoIoSelectGroup : public CAoThread{
public:
    CAoIoSelectGroup();
    virtual ~CAoIoSelectGroup();
	virtual int Run();
	bool Append(CAoSockChannel* pASC);
	void Stop();
protected:
	int GetIoCount();
	enum{
		EN_MAX_IO_COUNT = 1024,
		EN_INIT_CAHCE_SIZE = 102400
	};
	typedef std::list<CAoSockChannel*> ASC_LST;
	typedef ASC_LST::iterator ASC_LSTP_IT;
	ASC_LST m_lstASC;
	bool m_bRun;
	CAoLock* m_pLock;
	CAoEvent* m_pEvTimeout;
};
#endif // AoIoSelectGroup_h__
