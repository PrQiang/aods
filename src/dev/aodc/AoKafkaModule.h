/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef _AoKafkaModule_H__
#define _AoKafkaModule_H__
#include "../util/Module.h"
#include "../util/AoEvent.h"
namespace RdKafka{
	class Producer;
	class Topic;
};
class CConsumerThread;
class CAoKafkaModule : public CModule{
public:
	static CAoKafkaModule* Instance();
	static void Release();
protected:
	CAoKafkaModule();
	virtual ~CAoKafkaModule();
	void OnLoadCfg(const MT_MSG* pMM);
	void OnSnd2Kafka(const MT_MSG* pMM);
	DECLARE_AO_MSG_MAP()
private:
	static CAoKafkaModule* ms_pInstance;
	static CAoLock ms_Lock;
	CAoEvent m_evSigal;
	CConsumerThread* m_pCT;
	RdKafka::Producer* m_pProducer;
	RdKafka::Topic* m_pTopic;
};
#endif // _AoKafkaModule_H__
