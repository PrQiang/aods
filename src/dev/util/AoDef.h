/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoDef_h__
#define AoDef_h__

#include "AoMemoryMgr.h"
#ifdef WIN32
#pragma warning(disable:4200)
#endif
class CMsgTarget;
enum{// 定义偏移枚举变量
	EN_SEEK_SET = 0,
	EN_SEEK_CUR,
	EN_SEEK_END,
};
enum{// 定义文件名称最大长度枚举变量
	EN_MAX_PATH_LEN = 256
};
enum{
	EN_SOMAX_CONN = 0x7fffffff
};
enum{
	EN_AO_UUID_LEN = 32
};
typedef int ao_size_t;// 定义数据类型
enum{// 定义错误码
	EN_NOERROR = 0, // 无错误
	EN_ERROR_INVALID_PARA = 1,
    // 内存相关错误码
	EN_ERROR_MEMORY_MALLOC_FAIL = 0x2000, // 内存分配失败	
	// 文件相关
	EN_ERROR_OPEN_FILE_FAIL = 0x3000, // 文件打开失败
	EN_ERROR_INVALID_ZIP_FILE, // 非法的压缩文件
	EN_ERROR_ERROR_ZIP_FILE_PASSWORD, // 错误的解密密码
	EN_ERROR_ZIP_STREAM_FAIL, // 解压至流失败
	EN_ERROR_HTTP_CRASH_URL_FAIL, // url解析失败
	EN_ERROR_HTTP_CONNECT_FAIL, // 连接失败
	EN_ERROR_HTTP_DLOAD_FILE_FAIL, // 下载文件失败
	EN_ERROR_HTTP_SEND_REQUEST_FAIL, // 发送请求失败
	EN_ERROR_HTTP_BEGIN = 0x500000,
};
enum{// 文件共享方式
	EN_SH_DENYRW = 0x10,
	EN_SH_DENYWR = 0x20,
	EN_SH_DENYRD = 0x30,
	EN_SH_DENYNO = 0x40
};
enum{// 定义文件属性枚举变量
	EN_FILE_ATTRIBUTE_RDO = 0x1,  // 只读
	EN_FILE_ATTRIBUTE_HID = 0x2,   // 隐藏
	EN_FILE_ATTRIBTUE_SYS = 0x4,  // 系统
	EN_FILE_ATTRIBUTE_DIR = 0x10, // 目录
	EN_FILE_ATTRIBUTE_ARC = 0x20, // 存档	
};
enum{// 通讯相关
	EN_INVALID_SOCKET = -1,
    EN_INVALID_HANDLE = -1
};
enum{// 定义基础模块枚举变量
	EN_MODULE_ID_BROADCAST = 0,
	EN_BASE_MODULE_ID_LOG, // 日志模块
	EN_MODULE_ID_CONFIG, // 配置模块
	EN_MODULE_ID_COMMU, // 通讯模块
	EN_MODULE_ID_BEGIN = 0x100,
};
enum{// 定义基础模块消息定义
	EN_LOG_MODULE_LOG_INFO = 1, // 打印信息日志	
	EN_LOG_MODULE_LOG_DEBUG, // 打印调试日志
	EN_LOG_MODULE_LOG_DATA,  // 打印数据日志
	EN_LOG_MODULE_LOG_WARN,  // 打印告警日志
	EN_LOG_MODULE_LOG_ERR,   // 打印错误日志
	// 配置相关消息
	EN_AO_MSG_CFG_START,    // 启动配置
	EN_AO_MSG_LOAD_CFG,     // 配置加载
	EN_AO_MSG_LOAD_CCF,     // 加载通用配置
	EN_AO_MSG_SAVE_CFG,     // 修改配置
	// 系统基础功能相关
	EN_AO_MSG_SETTIMER,     // 设置定时器
	EN_AO_MSG_KILLTIMER,    // 卸载定时器
	EN_AO_MSG_ONTIMER,      // 定时器事件
	EN_AO_MSG_STOP,        // 停止
	EN_AO_MSG_BIT,         // 自检消息
	EN_AO_MSG_BIT_RLT,     // 自检结果
	EN_AO_MSG_EXIT,        // 退出
	// 通讯通道相关
	EN_AO_MSG_CHANNEL_NEW, // 新增通道
	EN_AO_MSG_CHANNEL_DEL, // 删除通道
	EN_AO_MSG_CHANNEL_BUILD,   // 创建通道
	EN_AO_MSG_CHANNEL_BUILDED, // 已建立通道	
	EN_AO_MSG_CHANNEL_FREE,    // 释放通道
	EN_AO_MSG_CHANNEL_FREED,   // 已释放通道
	EN_AO_MSG_CHANNEL_RECV_DATA,    // 接收数据
	EN_AO_MSG_CHANNEL_SEND_DATA,    // 发送数据
	EN_AO_MSG_CHANNEL_SEND_DATA_FAIL, // 发送数据失败
	EN_AO_MSG_BEGIN = 0x200
};
#ifdef MEM_CACHE 
#define MALLOC(size) CAoMemoryMgr::Instance()->Malloc(size)
#define FREE(p) CAoMemoryMgr::Instance()->Free(p)
#define REALLOC(p, newSize) CAoMemoryMgr::Instance()->Realloc(p, newSize)
#else
#define MALLOC(size) malloc(size)
#define FREE(p) free((p))
#define REALLOC(p, newSize) realloc((p), (newSize))
#endif
#define SAFE_FREE(p) if(NULL != (p)){FREE((p)); (p) = NULL;};
#define SAFE_DELETE(p) if(NULL != (p)){delete (p); (p) = NULL;};
struct MT_MSG_HEAD{// 定义消息头数据结构数据结构
	unsigned int unSMI;              // 发送消息模块标识
	unsigned int unMsgCode;     // 消息代码
	unsigned int unDataLen;      // 仅包括数据部分长度
	CMsgTarget* pRecver;  // 接收者
	CMsgTarget* pSender;  // 发送者
};
enum{// 定义定义消息头数据结构数据结构长度枚举变量
	EN_MT_MSG_HEAD_SIZE = sizeof(MT_MSG_HEAD)
};
struct MT_MSG : public MT_MSG_HEAD{// 定义模块消息数据结构
	unsigned char ucData[0];        // 数据内容
};
enum{// 定义定义消息长度枚举变量枚举变量
	EN_MT_MSG_SIZE = sizeof(MT_MSG)
};
struct MSG_TIMER{// 定义定时器消息数据结构
	unsigned int unTimerId; // 定时器ID
	unsigned int unEscape;    // 周期
	unsigned int unTime;      // 时间
	unsigned int unTimes;    // 次数
	CMsgTarget* pMT; // 消息处理对象
};
enum{// 定义定时器消息数据结构长度枚举变量
	EN_MSG_TIMER_SIZE = sizeof(MSG_TIMER),
};
struct AO_MSG_BIT_RESULT{// 定义自检结果数据结构
	unsigned char ucModuleID; // 模块ID
	unsigned char ucResult;       // 自检结果
};
enum{// 定义自检结果数据结构长度枚举变量
	EN_AO_MSG_BIT_RESULT_SIZE = sizeof(AO_MSG_BIT_RESULT)
};
struct AO_MSG_SAVE_CFG{// 定义修改配置数据结构
	unsigned int unSrcModuleId;
	char szCfgJson[0];
};
enum{// 定义修改配置数据结构大小枚举变量
    EN_AO_MSG_SAVE_CFG_SIZE = sizeof(AO_MSG_SAVE_CFG)
};
// 定义通讯消息结构
#pragma pack(push)
#pragma pack(1)
struct AO_MSG{
	int nDataLen; // 数据长度，包含自身
	int nKeyCode; // 密钥代码
	int nKeyLen; //  秘钥长度
	char szKeyData[0];
};
enum{
	EN_AO_MSG_SIZE = sizeof(AO_MSG)
};
struct MSG_HEAD{// 定义消息头数据结构
	unsigned int unMsgCode;           // 消息代码
	unsigned int unLen;               // 消息长度 不包括消息本身长度
	unsigned char ucEncryptKey;         // 密钥号
	unsigned int unCliId;             // 客户端ID
	unsigned char ucData[0];            // 消息
};
enum{
	EN_MSG_HEAD_SIZE = sizeof(MSG_HEAD)
};
#pragma pack(pop)
#endif // AoDef_h__