/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodcDef_h__
#define AodcDef_h__
#include "../util/AoDef.h"
enum{// ����ģ��ö�ٱ���
	EN_MODULE_ID_ACTIVE = EN_MODULE_ID_BEGIN, // ����ģ��
	EN_MODULE_ID_AUTH, // ��֤ģ��
	EN_MODULE_ID_UPDATE, // ����ģ��
	EN_MODULE_ID_KAFKA, // kafkaͨѶģ��
};
enum{
	EN_AO_MSG_AUTH_SUC = EN_AO_MSG_BEGIN, // ��֤�ɹ�
	EN_AO_MSG_SND2KAFKA, // ����kafka��Ϣ
	EN_AO_MSG_RCVFKAFKA, // ��������kafka��Ϣ
};
#endif // AodcDef_h__