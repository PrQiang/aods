/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodcDef_h__
#define AodcDef_h__
#include "../util/AoDef.h"
enum{// 定义模块枚举变量
	EN_MODULE_ID_ACTIVE = EN_MODULE_ID_BEGIN, // 激活模块
	EN_MODULE_ID_AUTH, // 认证模块
	EN_MODULE_ID_UPDATE, // 更新模块
	EN_MODULE_ID_KAFKA, // kafka通讯模块
};
enum{
	EN_AO_MSG_AUTH_SUC = EN_AO_MSG_BEGIN, // 认证成功
	EN_AO_MSG_SND2KAFKA, // 发送kafka消息
	EN_AO_MSG_RCVFKAFKA, // 接收来自kafka消息
};
#endif // AodcDef_h__