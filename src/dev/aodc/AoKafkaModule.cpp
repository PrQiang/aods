/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <string.h>
#include "../util/CJson.h"
#include "../util/AoHash.h"
#include "AodcDef.h"
#include "AoKafkaModule.h"
#include "ConsumerThread.h"
CAoKafkaModule* CAoKafkaModule::ms_pInstance = NULL;
CAoLock CAoKafkaModule::ms_Lock;
BEGIN_AO_MSG_MAP(CAoKafkaModule, CModule)
    ON_AO_MSG_FUN(EN_AO_MSG_LOAD_CFG, &CAoKafkaModule::OnLoadCfg)
	ON_AO_MSG_FUN(EN_AO_MSG_SND2KAFKA, &CAoKafkaModule::OnSnd2Kafka)
END_AO_MSG_MAP()
CAoKafkaModule::CAoKafkaModule():CModule(EN_MODULE_ID_KAFKA, 1024*1024, "kafka"){m_pProducer = NULL;}
CAoKafkaModule::~CAoKafkaModule(){}
CAoKafkaModule* CAoKafkaModule::Instance(){
	if (NULL == ms_pInstance){
		ms_Lock.Lock();ms_pInstance = (NULL == ms_pInstance) ? new CAoKafkaModule : ms_pInstance;ms_Lock.Unlock();
	}
	return ms_pInstance;
}
void CAoKafkaModule::Release(){SAFE_DELETE(ms_pInstance);}
void CAoKafkaModule::OnLoadCfg(const MT_MSG* pMM){
	cJSON* pObject = cJSON_Parse((const char*)pMM->ucData);
	if (NULL == pObject){LogErr("CAoKafkaModule::LoadConfig", "Faild to load config parameter");return;}
	cJSON* pProperty = cJSON_GetObjectItem(pObject, "brokers");
	if (NULL == pProperty || cJSON_String != pProperty->type ){
		LogErr("CAoKafkaModule::LoadConfig", "Faild to load config parameter: brokers");cJSON_Delete(pObject);return;
	}
	std::string strBroker(pProperty->valuestring);
	pProperty = cJSON_GetObjectItem(pObject, "producer_topic");
	if (NULL == pProperty || cJSON_String != pProperty->type ){
		LogErr("CAoKafkaModule::LoadConfig", "Faild to load config parameter: producer_topic");cJSON_Delete(pObject);return;
	}
	std::string strProducerTopic(pProperty->valuestring);
	pProperty = cJSON_GetObjectItem(pObject, "consumer_topic");
	if (NULL == pProperty || cJSON_String != pProperty->type){
		LogErr("CAoKafkaModule::LoadConfig", "Faild to load config parameter: consumer_topic");
		cJSON_Delete(pObject);return;
	}
	std::string strConsumerTopic(pProperty->valuestring);
	pProperty = cJSON_GetObjectItem(pObject, "consumer_gid");
	if (NULL == pProperty || cJSON_String != pProperty->type) {
		LogErr("CAoKafkaModule::LoadConfig", "Faild to load config parameter: consumer_gid");
		cJSON_Delete(pObject);
		return;
	}
	std::string strConsumerGid(pProperty->valuestring);
	cJSON_Delete(pObject);
	std::string strErr;
	RdKafka::Conf* pGConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	pGConf->set("metadata.broker.list", strBroker, strErr);
	pGConf->set("group.id", strConsumerGid.c_str(), strErr);
	int nRet = 0;
	RdKafka::Conf* pTConf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	m_pCT = CConsumerThread::Create(strConsumerTopic.c_str(), pTConf, pGConf, 0, "", &m_evSigal, &nRet);
	if (NULL == m_pCT){return ;}
	m_pProducer = RdKafka::Producer::create(pGConf, strErr);
	if (NULL == m_pProducer){return;}
	m_pTopic = RdKafka::Topic::create(m_pProducer, strProducerTopic, pTConf, strErr);
}
void CAoKafkaModule::OnSnd2Kafka(const MT_MSG* pMM){
	m_pProducer->produce(m_pTopic, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY, (void*)pMM->ucData, pMM->unDataLen, NULL, NULL);
}