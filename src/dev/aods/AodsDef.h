/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AodsDef_h__
#define AodsDef_h__
#include <string>
#include <map>
#include "../AodsAodcMsgDef.h"
enum{// 定义模块枚举变量
	EN_MODULE_ID_AUTH = EN_MODULE_ID_BEGIN, // 认证模块
	EN_MODULE_ID_UPDATE, // 更新模块
};
enum{
	EN_AO_MSG_AUTHEN_SUC = EN_AO_MSG_BEGIN, // 认证成功
	EN_AO_MSG_DOWNLOAD_RESULT // 下载结果
};
enum{
	EN_DR_SUC = 0,
	EN_DR_DFA, // 下载失败
	EN_DR_SFA, // 保存失败
	EN_DR_CFA, // 校验失败
	EN_DR_UFA, // 解压失败
	EN_DR_RFA, // 重启失败
};
struct DOWNLOAD_RESULT{// 定义下载结果数据结构
	unsigned char ucRlt;
	char szBuf[0];
};
enum{// 定义下载结果数据结构大小枚举变量
    EN_DOWNLOAD_RESULT_SIZE = sizeof(DOWNLOAD_RESULT)
};
#endif // AodsDef_h__
