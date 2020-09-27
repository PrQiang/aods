#ifndef AodsAodcMsgDef_h__
#define AodsAodcMsgDef_h__
#include "./util/AoDef.h"
#ifdef WIN32
#pragma warning(disable:4200)
#endif
enum{// 定义部署服务端与客户端消息代码枚举变量
	EN_AODS_AODC_ACTV = 1, //申请激活
	EN_AODC_AODS_ACTV,    // 申请激活应答
	EN_AODS_AODC_AUTH, // aods认证消息
	EN_AODC_AODS_AUTH, // aodc认证消息
	EN_AODS_AODC_UMI,  // 上报更新模块信息
	EN_AODC_AODS_UMI,  // 更新模块确认信息
	EN_AODS_AODC_UDR, // 更新结果上报
	EN_AODC_AODS_UDR, // 更新结果应答
	EN_AODC_AODS_RESTART, // 重启
	EN_AODS_AODC_RESTART, // 重启结果
	EN_AODC_AODS_CONFIG, // 配置
	EN_AODS_AODC_CONFIG, // 配置
	EN_AODS_AODC_KEEPALIVE, // 保活查询消息
	EN_AODC_AODS_KEEPALIVE, // 包括查询应答消息
    EN_AODS_AODC_UMI2CHK, // 更新模块检测上报
};
#pragma pack(push)
#pragma pack(1)
struct AODS_AODC_MSG{// 定义消息头定义数据结构
	unsigned int unMsgCode; // 消息代码
	char szMsg[0];
};
enum{// 定义消息头定义数据结构大小枚举变量
    EN_AODS_AODC_MSG_SIZE = sizeof(AODS_AODC_MSG)
};
/************************************************************************/
/* 激活相关                                                             */
/************************************************************************/
enum{
	EN_ACTIVE_TYPE_BAS = 1, // 基础版
	EN_ACTIVE_TYPE_MID = 2, // 中级版
	EN_ACTIVE_TYPE_HIG = 3, // 高级版
};
enum{
	EN_RES_ID_LEN = 6,
	EN_RD_SERIAL_LEN = 14,
	EN_SEC_SIZE_LEN = 5,
	EN_MAX_IP_ADDR_NUM = 16
};
struct AODS_AODC_ACTIVE{// 定义激活消息数据结构
	unsigned char ucIndex;
	unsigned char ucIpNum;                               // IP大小
	unsigned int unIpAddr[EN_MAX_IP_ADDR_NUM];           // IP数
	unsigned char ucResId[EN_RES_ID_LEN];                // 资源号
	unsigned char ucRDSerial[EN_RD_SERIAL_LEN];          // 随机序列号
};
enum{// 定义激活消息数据结构大小枚举变量
	EN_AODS_AODC_ACTIVATE_SIZE = sizeof(AODS_AODC_ACTIVE)
};
enum{// 定义激活失败结果枚举变量
	EN_ACT_SUCCESS = 0, // 激活成功
	EN_ACT_RLT_ACTIVEDBYOTHER,    // 资源号被其他服务器激活
	EN_ACT_RLT_REFUSED,          // 拒绝
	EN_ACT_RLT_DBBUSY,           // 繁忙
	EN_ACT_RLT_REMOVED,          // 被移除
	EN_ACT_RLT_APPLYRESFAIL,     // 申请资源号失败
	EN_ACT_RLT_DATAINV,          // 数据非法
};
enum{
	EN_ACT_CODE_LEN = EN_RES_ID_LEN + EN_RD_SERIAL_LEN
};
struct AODC_AODS_ACTIVE{// 定义激活应答数据结构
	unsigned char ucResult; // 激活结果
	unsigned char ucData[0]; // 激活码
};
enum{// 定义激活应答数据结构大小枚举变量
    EN_AODC_AODS_ACTIVE_SIZE = sizeof(AODC_AODS_ACTIVE)
};
struct AODS_ACTIVE_SUC{// 定义激活成功数据结构
	unsigned char ucResId[EN_RES_ID_LEN]; // 资源号
	unsigned char ucActCode[EN_ACT_CODE_LEN]; // 激活码
};
enum{// 定义激活成功数据结构大小枚举变量
    EN_AODS_ACTIVE_SUC_SIZE = sizeof(AODS_ACTIVE_SUC)
};
/************************************************************************/
/* 认证相关                                                             */
/************************************************************************/
struct AODS_AODC_AUTHEN{// 认证请求消息
	unsigned char ucIpNum;                      // IP大小
	unsigned int unIpAddr[EN_MAX_IP_ADDR_NUM];// IP数
	unsigned char ucResId[EN_RES_ID_LEN];       // 资源号
	unsigned char ucRDSerial[EN_RD_SERIAL_LEN]; // 硬盘序列号
	unsigned char ucActCode[EN_ACT_CODE_LEN];
};
enum{
	EN_AODS_AODC_AUTHEN_SIZE = sizeof(AODS_AODC_AUTHEN)
};
enum{// 定义认证结果枚举变量
	EN_AUTH_RESULT_SUC = 0,                 // 认证成功
	EN_AUTH_RESULT_FAI,                     // 认证失败
    EN_AUTH_RESULT_DIF,                     // 不同服务器认证
	EN_AUTH_RESULT_BUS                       // 繁忙
};
struct AODC_AODS_AUTHEN{// 定义认证结果消息
	unsigned char ucRlt; // 认证结果
};
enum{// 定义Aodc认证消息数据结构大小枚举变量
	EN_AODC_AODS_AUTHEN_SIZE = sizeof(AODC_AODS_AUTHEN)
};
/************************************************************************/
/* 部署相关                                                             */
/************************************************************************/
struct AODC_AODS_DPFH{// 定义传输文件头数据结构
    unsigned int unOffset; // 文件偏移
    unsigned int unLength; // 文件长度
    char szBuf[0]; // 项目名称\0模块名称\0版本号\0文件名称\0文件hash
};
enum{ // 定义传输文件头数据结构大小枚举变量
    EN_AODC_AODS_DPFH_SIZE = sizeof(AODC_AODS_DPFH)
};
enum{// 定义传输文件头结果枚举变量
    EN_DPFH_TRANS_RLT_NEWEST = 1, // 已经是最新版本
    EN_DPFH_TRANS_RLT_PLEASE, // 请更新版本
    EN_DPFH_TRANS_RLT_NOSUPPORT, // 本地不支持
};
struct AODS_AODC_DPFH{// 定义传输文件头数据结构
    unsigned char ucRlt; // 应答结果
};
enum{// 定义传输文件头数据结构大小枚举变量
    EN_AODS_AODC_DPFH_SIZE = sizeof(AODS_AODC_DPFH)
};
enum{// 定义传输文件校验结果枚举变量
    EN_CHRLT_CORRECT = 1, // 文件传输校验正确
    EN_CHRLT_SAVEFAI, // 配置文件保存失败
    EN_CHRLT_NOEXIST, // 校验文件不存在
    EN_CHRLT_WRONG // 文件校验错误
};
struct AODS_AODC_CHRLT{// 定义传输文件校验结果数据结构
    unsigned char ucRlt;
};
enum{// 定义传输文件校验结果数据结构大小枚举变量
    EN_AODS_AODC_CHRLT_SIZE = sizeof(AODS_AODC_CHRLT)
};
enum{// 定义更新结果枚举变量
    EN_UPDATE_RLT_SUC = 1, // 更新成功
    EN_UPDATE_RLT_NEX, // 指定更新的模块已不存在
    EN_UPDATE_RLT_CKF, // 校验错误
    EN_UPDATE_RLT_FRL, // 文件被替换
    EN_UPDATE_RLT_FFE, // 文件格式不正确
    EN_UPDATE_RLT_FAI, // 更新文件失败
};
struct AODS_AODC_UPDATE{// 定义文件更新结果数据结构
    unsigned char ucRlt;
    char szInfo[0]; // 项目名称\0模块名称\0
};
enum{// 定义文件更新结果数据结构大小枚举变量
    EN_AODS_AODC_UPDATE_SIZE = sizeof(AODS_AODC_UPDATE)
};
enum{// 定义模块重启结果枚举变量
	EN_AODS_AODC_RESTART_RLT_SUC = 1, // 重启成功
	EN_AODS_AODC_RESTART_RLT_MNE, // 模块不存在
	EN_AODS_AODC_RESTART_RLT_CNE, // 重启命令不存在
	EN_AODS_AODC_RESTART_RLT_FAI, // 重启失败
};
struct AODS_AODC_RESTART{// 定义重启模块执行结果数据结构
	unsigned char ucRlt;
	char szInfo[0]; // 项目名称\0模块名称\0
};
enum{// 定义重启模块数据结构大小枚举变量
    EN_AODS_AODC_RESTART_SIZE = sizeof(AODS_AODC_RESTART)
};
enum{// 定义配置结果枚举变量
	EN_AODS_AODC_CONFIG_RLT_SUC = 1, // 成功
	EN_AODS_AODC_CONFIG_RLT_FAI, // 失败
};
struct AODS_AODC_CONFIG{// 定义配置修改结果上报数据结构
	unsigned char ucRlt;
	char szInfo[0]; // 项目名称\0模块名称\0
};
enum{// 定义配置修改结果上报数据结构大小枚举变量
    EN_AODS_AODC_CONFIG_SIZE = sizeof(AODS_AODC_CONFIG)
};
/************************************************************************/
/* 定义保活相关参数                                                     */
/************************************************************************/
enum{
    EN_KEEPALIVE_TIME = 60000 // 保活超时时间60s
};
#pragma pack(pop)
#endif // AodsAodcMsgDef_h__