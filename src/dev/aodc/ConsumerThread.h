/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef ComsumerThread_h__
#define ComsumerThread_h__
#include "../util/AoThread.h"
#ifdef WIN32
#include "../librdkafkacpp/rdkafkacpp.h"
#else
#include "rdkafkacpp.h"
#endif
class CAoEvent;
class CConsumerThread : public CAoThread, public RdKafka::ConsumeCb{
protected:
    CConsumerThread(RdKafka::Consumer* pConsumer, RdKafka::Topic* pTopic, int nPart, const char* pszUuid, CAoEvent* pev, int* nErr);
    ~CConsumerThread();
public:
	virtual int Run();
	static CConsumerThread* Create(const char* pszTopic, RdKafka::Conf *pTConfig, RdKafka::Conf *pConfig, int nPart, const char* pszUuid, CAoEvent* pev, int* nErr);
	void consume_cb(RdKafka::Message &msg, void *opaque);
    void Stop();
protected:
	void ProcMsg(const char* pszBuf, int nLen);
protected:
	RdKafka::Consumer* m_pConsumer;
	RdKafka::Topic* m_pTopic;
	int m_nPart;
	std::string m_strUuid;
	CAoEvent* m_pev;
	int* m_pnErr;
    bool m_bRun;
};
#endif // ComsumerThread_h__
