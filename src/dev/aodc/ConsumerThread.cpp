/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "ConsumerThread.h"
#include "../util/CJson.h"
#include "../util/AoEvent.h"
#include "AodcDef.h"
#include "AoKafkaModule.h"
CConsumerThread::CConsumerThread(RdKafka::Consumer* pConsumer, RdKafka::Topic* pTopic, int nPart, const char* pszUuid, CAoEvent* pev, int* nErr)
	:m_strUuid(pszUuid), m_pev(pev), m_pnErr(nErr){
	m_pConsumer = pConsumer;m_pTopic = pTopic;
	m_nPart = nPart;CAoThread::Start();
}
CConsumerThread::~CConsumerThread(){}
int CConsumerThread::Run(){
    m_bRun = true;
    RdKafka::ErrorCode err = m_pConsumer->start(m_pTopic, m_nPart, RdKafka::Topic::OFFSET_END);
	std::string strErr(RdKafka::err2str(err));
    while (m_bRun){
		m_pConsumer->poll(1000);
		m_pConsumer->consume_callback(m_pTopic, m_nPart, 1000, this, NULL);
	}
	m_pConsumer->stop(m_pTopic, m_nPart);
	m_pConsumer->poll(1000);
	return 0;
}
CConsumerThread* CConsumerThread::Create(const char* pszTopic, RdKafka::Conf *pTConfig, RdKafka::Conf *pConfig, int nPart, const char* pszUuid, CAoEvent* pev, int* nErr){
	std::string strErr;
	RdKafka::Consumer* pConsumer = RdKafka::Consumer::create(pConfig, strErr);
	if (NULL == pConsumer){return NULL;}
	RdKafka::Topic* pTopic = RdKafka::Topic::create(pConsumer, pszTopic, pTConfig, strErr);
	if (NULL == pTopic){return NULL;}
	return new CConsumerThread(pConsumer, pTopic, nPart, pszUuid, pev, nErr);
}
void CConsumerThread::consume_cb(RdKafka::Message &msg, void *){
	switch (msg.err()){
	case RdKafka::ERR__TIMED_OUT:break;
	case RdKafka::ERR_NO_ERROR:
		ProcMsg(static_cast<const char *>(msg.payload()), static_cast<int>(msg.len()));
		m_pConsumer->poll(0);
		break;
	case RdKafka::ERR__PARTITION_EOF:
		ProcMsg(static_cast<const char *>(msg.payload()), static_cast<int>(msg.len()));
		m_pConsumer->poll(0);
		break;
	case RdKafka::ERR__UNKNOWN_TOPIC:
	case RdKafka::ERR__UNKNOWN_PARTITION:  
	default: m_bRun = false;break;
	}
}
void CConsumerThread::ProcMsg(const char* pszBuf, int nLen){
	CAoKafkaModule::Instance()->SendOutMsg(EN_MODULE_ID_BROADCAST, EN_AO_MSG_RCVFKAFKA, (unsigned char*)pszBuf, nLen);
}
void CConsumerThread::Stop(){m_bRun = false;Join();}
