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
enum ENHttpMethod{// 定义Http方法枚举变量
	EN_HTTP_METHOD_GET = 0, // Get
	EN_HTTP_METHOD_POST,    // Post
	EN_HTTP_METHOD_PUT      // Put
};
enum{// 定义编码方式枚举变量
	EN_ENCODE_GB2312 = 0, // 国标 默认
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
	enum{// 定义默认收、发超时时间枚举变量
		EN_DEFAULT_RECV_TIMEOUT = 5000, // 默认接收超时时间
		EN_DEFAULT_SEND_TIMEOUT = 5000  // 默认发送超时时间
	};	
	unsigned int m_unRecvTimeout;// 接收超时时间	
	unsigned int m_unSendTimeout;// 发送超时
	unsigned long long m_unOffset;
	unsigned long long m_unContentLen; // 内容长度
	int m_nHttpErrcode;  // Http应答错误码
	std::string m_strReqHeader; // 请求头
	std::string m_strRequestData; // 请求数据
	unsigned char m_ucEncode;  // 请求数据编码方式
};
#endif