/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoHttp_h__
#define AoHttp_h__
#include <string>
#ifdef WIN32
#pragma warning(disable:4251)
#endif
enum ENHttpMethod{// ����Http����ö�ٱ���
	EN_HTTP_METHOD_GET = 0, // Get
	EN_HTTP_METHOD_POST,    // Post
	EN_HTTP_METHOD_PUT      // Put
};
enum{// ������뷽ʽö�ٱ���
	EN_ENCODE_GB2312 = 0, // ���� Ĭ��
	EN_ENCODE_UNICODE,    // Unicode
	EN_ENCDOE_UTF8,       // Utf-8
};
class CAoSock;
class CAoStream;
class CAoHttp{
public:
	CAoHttp();
	~CAoHttp();
	void SetOffset(unsigned long long unOffset);
	unsigned int HttpRun(const char* pszUrl, ENHttpMethod enMt, CAoStream* pAS);
	unsigned long long QueryContentLength();
	int QuersyErrorCode();
	unsigned long long QueryOffset();
	void SetReqHeader(const char* pszReqHeader);
	void SetRequestData(const char* pszRequestData);
	void SetEncode(unsigned char ucEncode);
	void Clear();
	void SetRecvSendTimeout(unsigned int unRecvTimeout, unsigned int unSendTimeout);
protected:
	bool CrashUrl(const char* pszUrl, std::string& strHostName, std::string& strObjName, unsigned short& usPort);
	bool CrashHostName(std::string& strUrl, std::string& strHostName);
	bool CheckHostName(const char* pszSrcAddr);
	bool CrashPort(const std::string& strObj, unsigned short& usPort);
	std::string FormatRequest(ENHttpMethod enMth, const char* pszObj, const char* pszHost);	
	std::string MethodString(ENHttpMethod enMth) const;
	std::string FormatRange() const;
	bool SendRequest(CAoSock* pAS, ENHttpMethod enMt, const char* pszObj, const char* pszHost);
	bool ReadData(CAoSock* pAS, CAoStream* pAoStream);
	bool ParseResponse( char* pszRep, int &nRepLen);
	void Encode(const unsigned char* pucSrc, unsigned short usLen, std::string& strResult);
protected:	
	enum{// ����Ĭ���ա�����ʱʱ��ö�ٱ���
		EN_DEFAULT_RECV_TIMEOUT = 5000, // Ĭ�Ͻ��ճ�ʱʱ��
		EN_DEFAULT_SEND_TIMEOUT = 5000  // Ĭ�Ϸ��ͳ�ʱʱ��
	};	
	unsigned int m_unRecvTimeout;// ���ճ�ʱʱ��	
	unsigned int m_unSendTimeout;// ���ͳ�ʱ
	unsigned long long m_unOffset;
	unsigned long long m_unContentLen; // ���ݳ���
	int m_nHttpErrcode;  // HttpӦ�������
	std::string m_strReqHeader; // ����ͷ
	std::string m_strRequestData; // ��������
	unsigned char m_ucEncode;  // �������ݱ��뷽ʽ
};
#endif