#ifndef AodsAodcMsgDef_h__
#define AodsAodcMsgDef_h__
#include "./util/AoDef.h"
#ifdef WIN32
#pragma warning(disable:4200)
#endif
enum{// ���岿��������ͻ�����Ϣ����ö�ٱ���
	EN_AODS_AODC_ACTV = 1, //���뼤��
	EN_AODC_AODS_ACTV,    // ���뼤��Ӧ��
	EN_AODS_AODC_AUTH, // aods��֤��Ϣ
	EN_AODC_AODS_AUTH, // aodc��֤��Ϣ
	EN_AODS_AODC_UMI,  // �ϱ�����ģ����Ϣ
	EN_AODC_AODS_UMI,  // ����ģ��ȷ����Ϣ
	EN_AODS_AODC_UDR, // ���½���ϱ�
	EN_AODC_AODS_UDR, // ���½��Ӧ��
	EN_AODC_AODS_RESTART, // ����
	EN_AODS_AODC_RESTART, // �������
	EN_AODC_AODS_CONFIG, // ����
	EN_AODS_AODC_CONFIG, // ����
	EN_AODS_AODC_KEEPALIVE, // �����ѯ��Ϣ
	EN_AODC_AODS_KEEPALIVE, // ������ѯӦ����Ϣ
    EN_AODS_AODC_UMI2CHK, // ����ģ�����ϱ�
};
#pragma pack(push)
#pragma pack(1)
struct AODS_AODC_MSG{// ������Ϣͷ�������ݽṹ
	unsigned int unMsgCode; // ��Ϣ����
	char szMsg[0];
};
enum{// ������Ϣͷ�������ݽṹ��Сö�ٱ���
    EN_AODS_AODC_MSG_SIZE = sizeof(AODS_AODC_MSG)
};
/************************************************************************/
/* �������                                                             */
/************************************************************************/
enum{
	EN_ACTIVE_TYPE_BAS = 1, // ������
	EN_ACTIVE_TYPE_MID = 2, // �м���
	EN_ACTIVE_TYPE_HIG = 3, // �߼���
};
enum{
	EN_RES_ID_LEN = 6,
	EN_RD_SERIAL_LEN = 14,
	EN_SEC_SIZE_LEN = 5,
	EN_MAX_IP_ADDR_NUM = 16
};
struct AODS_AODC_ACTIVE{// ���弤����Ϣ���ݽṹ
	unsigned char ucIndex;
	unsigned char ucIpNum;                               // IP��С
	unsigned int unIpAddr[EN_MAX_IP_ADDR_NUM];           // IP��
	unsigned char ucResId[EN_RES_ID_LEN];                // ��Դ��
	unsigned char ucRDSerial[EN_RD_SERIAL_LEN];          // ������к�
};
enum{// ���弤����Ϣ���ݽṹ��Сö�ٱ���
	EN_AODS_AODC_ACTIVATE_SIZE = sizeof(AODS_AODC_ACTIVE)
};
enum{// ���弤��ʧ�ܽ��ö�ٱ���
	EN_ACT_SUCCESS = 0, // ����ɹ�
	EN_ACT_RLT_ACTIVEDBYOTHER,    // ��Դ�ű���������������
	EN_ACT_RLT_REFUSED,          // �ܾ�
	EN_ACT_RLT_DBBUSY,           // ��æ
	EN_ACT_RLT_REMOVED,          // ���Ƴ�
	EN_ACT_RLT_APPLYRESFAIL,     // ������Դ��ʧ��
	EN_ACT_RLT_DATAINV,          // ���ݷǷ�
};
enum{
	EN_ACT_CODE_LEN = EN_RES_ID_LEN + EN_RD_SERIAL_LEN
};
struct AODC_AODS_ACTIVE{// ���弤��Ӧ�����ݽṹ
	unsigned char ucResult; // ������
	unsigned char ucData[0]; // ������
};
enum{// ���弤��Ӧ�����ݽṹ��Сö�ٱ���
    EN_AODC_AODS_ACTIVE_SIZE = sizeof(AODC_AODS_ACTIVE)
};
struct AODS_ACTIVE_SUC{// ���弤��ɹ����ݽṹ
	unsigned char ucResId[EN_RES_ID_LEN]; // ��Դ��
	unsigned char ucActCode[EN_ACT_CODE_LEN]; // ������
};
enum{// ���弤��ɹ����ݽṹ��Сö�ٱ���
    EN_AODS_ACTIVE_SUC_SIZE = sizeof(AODS_ACTIVE_SUC)
};
/************************************************************************/
/* ��֤���                                                             */
/************************************************************************/
struct AODS_AODC_AUTHEN{// ��֤������Ϣ
	unsigned char ucIpNum;                      // IP��С
	unsigned int unIpAddr[EN_MAX_IP_ADDR_NUM];// IP��
	unsigned char ucResId[EN_RES_ID_LEN];       // ��Դ��
	unsigned char ucRDSerial[EN_RD_SERIAL_LEN]; // Ӳ�����к�
	unsigned char ucActCode[EN_ACT_CODE_LEN];
};
enum{
	EN_AODS_AODC_AUTHEN_SIZE = sizeof(AODS_AODC_AUTHEN)
};
enum{// ������֤���ö�ٱ���
	EN_AUTH_RESULT_SUC = 0,                 // ��֤�ɹ�
	EN_AUTH_RESULT_FAI,                     // ��֤ʧ��
    EN_AUTH_RESULT_DIF,                     // ��ͬ��������֤
	EN_AUTH_RESULT_BUS                       // ��æ
};
struct AODC_AODS_AUTHEN{// ������֤�����Ϣ
	unsigned char ucRlt; // ��֤���
};
enum{// ����Aodc��֤��Ϣ���ݽṹ��Сö�ٱ���
	EN_AODC_AODS_AUTHEN_SIZE = sizeof(AODC_AODS_AUTHEN)
};
/************************************************************************/
/* �������                                                             */
/************************************************************************/
struct AODC_AODS_DPFH{// ���崫���ļ�ͷ���ݽṹ
    unsigned int unOffset; // �ļ�ƫ��
    unsigned int unLength; // �ļ�����
    char szBuf[0]; // ��Ŀ����\0ģ������\0�汾��\0�ļ�����\0�ļ�hash
};
enum{ // ���崫���ļ�ͷ���ݽṹ��Сö�ٱ���
    EN_AODC_AODS_DPFH_SIZE = sizeof(AODC_AODS_DPFH)
};
enum{// ���崫���ļ�ͷ���ö�ٱ���
    EN_DPFH_TRANS_RLT_NEWEST = 1, // �Ѿ������°汾
    EN_DPFH_TRANS_RLT_PLEASE, // ����°汾
    EN_DPFH_TRANS_RLT_NOSUPPORT, // ���ز�֧��
};
struct AODS_AODC_DPFH{// ���崫���ļ�ͷ���ݽṹ
    unsigned char ucRlt; // Ӧ����
};
enum{// ���崫���ļ�ͷ���ݽṹ��Сö�ٱ���
    EN_AODS_AODC_DPFH_SIZE = sizeof(AODS_AODC_DPFH)
};
enum{// ���崫���ļ�У����ö�ٱ���
    EN_CHRLT_CORRECT = 1, // �ļ�����У����ȷ
    EN_CHRLT_SAVEFAI, // �����ļ�����ʧ��
    EN_CHRLT_NOEXIST, // У���ļ�������
    EN_CHRLT_WRONG // �ļ�У�����
};
struct AODS_AODC_CHRLT{// ���崫���ļ�У�������ݽṹ
    unsigned char ucRlt;
};
enum{// ���崫���ļ�У�������ݽṹ��Сö�ٱ���
    EN_AODS_AODC_CHRLT_SIZE = sizeof(AODS_AODC_CHRLT)
};
enum{// ������½��ö�ٱ���
    EN_UPDATE_RLT_SUC = 1, // ���³ɹ�
    EN_UPDATE_RLT_NEX, // ָ�����µ�ģ���Ѳ�����
    EN_UPDATE_RLT_CKF, // У�����
    EN_UPDATE_RLT_FRL, // �ļ����滻
    EN_UPDATE_RLT_FFE, // �ļ���ʽ����ȷ
    EN_UPDATE_RLT_FAI, // �����ļ�ʧ��
};
struct AODS_AODC_UPDATE{// �����ļ����½�����ݽṹ
    unsigned char ucRlt;
    char szInfo[0]; // ��Ŀ����\0ģ������\0
};
enum{// �����ļ����½�����ݽṹ��Сö�ٱ���
    EN_AODS_AODC_UPDATE_SIZE = sizeof(AODS_AODC_UPDATE)
};
enum{// ����ģ���������ö�ٱ���
	EN_AODS_AODC_RESTART_RLT_SUC = 1, // �����ɹ�
	EN_AODS_AODC_RESTART_RLT_MNE, // ģ�鲻����
	EN_AODS_AODC_RESTART_RLT_CNE, // �����������
	EN_AODS_AODC_RESTART_RLT_FAI, // ����ʧ��
};
struct AODS_AODC_RESTART{// ��������ģ��ִ�н�����ݽṹ
	unsigned char ucRlt;
	char szInfo[0]; // ��Ŀ����\0ģ������\0
};
enum{// ��������ģ�����ݽṹ��Сö�ٱ���
    EN_AODS_AODC_RESTART_SIZE = sizeof(AODS_AODC_RESTART)
};
enum{// �������ý��ö�ٱ���
	EN_AODS_AODC_CONFIG_RLT_SUC = 1, // �ɹ�
	EN_AODS_AODC_CONFIG_RLT_FAI, // ʧ��
};
struct AODS_AODC_CONFIG{// ���������޸Ľ���ϱ����ݽṹ
	unsigned char ucRlt;
	char szInfo[0]; // ��Ŀ����\0ģ������\0
};
enum{// ���������޸Ľ���ϱ����ݽṹ��Сö�ٱ���
    EN_AODS_AODC_CONFIG_SIZE = sizeof(AODS_AODC_CONFIG)
};
/************************************************************************/
/* ���屣����ز���                                                     */
/************************************************************************/
enum{
    EN_KEEPALIVE_TIME = 60000 // ���ʱʱ��60s
};
#pragma pack(pop)
#endif // AodsAodcMsgDef_h__