/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AoHttp.h"
#include "AoDef.h"
#include "AoSockTcp.h"
#include "AoStream.h"
#include "AoHelper.h"
CAoHttp::CAoHttp(){
	Clear();
	m_unRecvTimeout = EN_DEFAULT_RECV_TIMEOUT;
	m_unSendTimeout = EN_DEFAULT_SEND_TIMEOUT;
}
CAoHttp::~CAoHttp(){}
void CAoHttp::SetOffset( unsigned long long unOffset ){	m_unOffset = unOffset;}
unsigned int CAoHttp::HttpRun( const char* pszUrl, ENHttpMethod enMt, CAoStream* pAS ){
	if (NULL == pszUrl){return EN_ERROR_INVALID_PARA;}
	std::string strHostName, strObjName;
	unsigned short usPort;
	if (!CrashUrl(pszUrl, strHostName, strObjName, usPort)){return EN_ERROR_HTTP_CRASH_URL_FAIL;}
	CAoSockTcp ast;
	if(0 != ast.Connect(strHostName.c_str(), usPort)){return EN_ERROR_HTTP_CONNECT_FAIL;}
	ast.SetRecvTimeout(m_unRecvTimeout);
	if (!SendRequest(&ast, enMt, strObjName.c_str(), strHostName.c_str())){return EN_ERROR_HTTP_SEND_REQUEST_FAIL;}
	return ReadData(&ast, pAS) ? EN_NOERROR : EN_ERROR_HTTP_DLOAD_FILE_FAIL;	
}
unsigned long long CAoHttp::QueryContentLength(){return m_unContentLen;}
int CAoHttp::QuersyErrorCode(){	return m_nHttpErrcode;}
unsigned long long CAoHttp::QueryOffset(){	return m_unOffset;}
void CAoHttp::SetReqHeader( const char* pszReqHeader){	m_strReqHeader = pszReqHeader;}
void CAoHttp::SetRequestData( const char* pszRequestData){
	Encode((unsigned char*)pszRequestData, (unsigned short)strlen(pszRequestData), m_strRequestData);
}
void CAoHttp::SetEncode( unsigned char ucEncode ){	m_ucEncode = ucEncode;}
void CAoHttp::Clear(){
	m_unOffset = 0;	m_unContentLen = 0;
	m_nHttpErrcode = EN_NOERROR;m_strReqHeader = "";
	m_strRequestData = "";m_ucEncode = EN_ENCDOE_UTF8;
}
void CAoHttp::SetRecvSendTimeout( unsigned int unRecvTimeout, unsigned int unSendTimeout ){
	m_unRecvTimeout = unRecvTimeout;m_unSendTimeout = unSendTimeout;}
bool CAoHttp::CrashUrl( const char* pszUrl, std::string& strHostName, std::string& strObjName, unsigned short& usPort ){
	std::string strUrl(pszUrl);
	std::string strTempHostName;
	unsigned short usTempPort;
	if (!CrashHostName(strUrl, strTempHostName)){return false;}
	int nIndex = (int)(strUrl.find_first_of("//") + 2 + strTempHostName.length());
	strUrl = strUrl.substr(nIndex);
	if (!CrashPort(strUrl, usTempPort)){return false;}
	nIndex = (int)strUrl.find_first_of('/');
	if (nIndex >= 0){strObjName = strUrl.substr(nIndex);}
	strHostName = strTempHostName;
	usPort = usTempPort;	
	return true;
}
bool CAoHttp::CrashHostName(std::string& strUrl, std::string& strHostName){
	int nIndex = (int)strUrl.find("//") + 2;
	if (nIndex < 2){return false;}
	std::string strProHead =  strUrl.substr(0, nIndex);
	std::string strHttp("http://");
	std::string strHttps("https://");
	if((strProHead != strHttp) && (strProHead != strHttps)){return false;}
	std::string::size_type unEndPos = strUrl.find_first_of(':', nIndex);
	std::string::size_type unStartObjPos = strUrl.find_first_of('/', nIndex);
	unEndPos = (unEndPos < unStartObjPos) ? unEndPos : unStartObjPos;
	strHostName = strUrl.substr(nIndex, unEndPos - nIndex);
	return CheckHostName(strHostName.c_str());
}
bool CAoHttp::CheckHostName(const char* pszSrcAddr){
	int nIndex = 0;
	while (pszSrcAddr[nIndex] != '\0'){
		if (!::isalnum(pszSrcAddr[nIndex])&& !::isalpha(pszSrcAddr[nIndex]) && pszSrcAddr[nIndex] != '-' && (pszSrcAddr[nIndex] != '.')) {
			return false;
		}
		++nIndex;
	}
	return true;
}
bool CAoHttp::CrashPort(const std::string& strObj, unsigned short& usPort){
	usPort = 80;
	if ((0 == strObj.length()) || strObj.at(0) != ':' ){return true;}
	int nIndex = (int)strObj.find_first_of('/');
	if (nIndex < 0){nIndex = (int)strObj.length();}
	if (1 == nIndex){return true;}
	std::string strPort = strObj.substr(1, nIndex - 1);
	for (int n = 0; n < (int)strPort.length(); ++n){if(!::isalnum(strPort.at(n))){return false;}}
	int nNum = 0;
	sscanf(strPort.c_str(), "%d", &nNum);
	if (nNum < 0 || nNum > 65535){return false;}
	usPort = (unsigned short)nNum;
	return true;
}
std::string CAoHttp::FormatRequest(ENHttpMethod enMth, const char* pszObj, const char* pszHost){
	std::string strObj;
	Encode((unsigned char*)pszObj, (unsigned short)strlen(pszObj), strObj);
	std::string strRequest = MethodString(enMth) + strObj + std::string(" HTTP/1.1\r\n");
	char szText[255] = {0};
	sprintf(szText, "Host: %s\r\n", pszHost);
	strRequest += std::string(szText);
	strRequest += std::string("Connection: Keep-Alive\r\nPragma:no-cache\r\n");
	strRequest += FormatRange();
	if (!m_strReqHeader.empty()){strRequest += m_strReqHeader + std::string("\r\n");}
	if (!m_strRequestData.empty()){	
		char szContent[50] = {0};
		sprintf(szContent, "Content-Length: %u\r\n\r\n", (unsigned int)m_strRequestData.length());
		strRequest += szContent;
		strRequest += m_strRequestData + std::string("\r\n");
	}
	strRequest += "\r\n";
	return strRequest;
}
std::string CAoHttp::MethodString( ENHttpMethod enMth ) const{
	std::string strMethod("");
	switch(enMth){
	case EN_HTTP_METHOD_GET:strMethod = "GET ";	break;
	case EN_HTTP_METHOD_POST:strMethod = "POST ";break;
	case EN_HTTP_METHOD_PUT:strMethod = "PUT ";	break;
	default:break;
	}
	return strMethod;
}
std::string CAoHttp::FormatRange() const{
	std::string strRet("");
	if (0 != m_unOffset){
		char szRange[100] = {0};
		sprintf(szRange, "Range: bytes=%llu-\r\n", m_unOffset);
		strRet = szRange;
	}
	return strRet;
}
bool CAoHttp::SendRequest( CAoSock* pAS, ENHttpMethod enMt, const char* pszObj, const char* pszHost ){
	std::string strRequest = FormatRequest(enMt, pszObj, pszHost);
	int nWriteLen = (int)strRequest.length();
	return (nWriteLen == pAS->Send(strRequest.c_str(), nWriteLen));
}
bool CAoHttp::ReadData( CAoSock* pAS, CAoStream* pAoStream){
	const int n1KBytes = 2 * 1024;
	char szBuf[n1KBytes] = {0};
	int nReadLen = 0;
	if(0 == (nReadLen = pAS->Recv(szBuf, n1KBytes))){return false;}
	int nRepLen = nReadLen;
	if (!ParseResponse(szBuf, nRepLen)){return false;}
	if ((NULL == pAoStream) || (0 == m_unContentLen)){return true;}	
	unsigned long long unHasReadCntLen = 0;
	if (nRepLen < nReadLen){
		nReadLen -= nRepLen;
		if(nReadLen != pAoStream->Write(szBuf + nRepLen, nReadLen)){return false;}
		m_unOffset += nReadLen;
		unHasReadCntLen = nReadLen;
	}
	while (unHasReadCntLen < m_unContentLen){
		nReadLen = pAS->Recv(szBuf, n1KBytes);
		if (nReadLen <= 0){	return false;	}
		if(nReadLen != pAoStream->Write(szBuf, nReadLen)){	return false;}
		m_unOffset += nReadLen;
		unHasReadCntLen += nReadLen;
	}
	return true;
}
bool CAoHttp::ParseResponse( char* pszRep, int &nRepLen){
	int nIndex = CAoHelper::FindFirstBuf(pszRep, nRepLen, "\r\n\r\n", 4);
	if (nIndex < 0){return false;}
	nRepLen = nIndex + 4;
	static const int snMaxCntLen = 256;
	char szCnt[snMaxCntLen] = {0};
	int nEndPos = CAoHelper::FindFirstBuf(pszRep, nRepLen, "\r\n", 2);
	if (nEndPos < 0){return false;}
	memcpy(szCnt, pszRep, nEndPos);
	szCnt[nEndPos] = '\0';
	float fVersion = 0.0f;
	if (2 != sscanf(szCnt, "HTTP/%f %d", &fVersion, &m_nHttpErrcode)){return false;}
	const char* pszCntLen = "Content-Length:";
	int nOneCntLen = (int)strlen(pszCntLen);
	nIndex = CAoHelper::FindFirstBuf(pszRep, nRepLen, pszCntLen, nOneCntLen) + nOneCntLen;
	if (nIndex < nOneCntLen){return false;}
	nEndPos = CAoHelper::FindFirstBuf(pszRep + nIndex, nRepLen - nIndex, "\r\n", 2);
	if (nEndPos < 0){return false;}
	memcpy(szCnt, pszRep + nIndex, nEndPos);
	szCnt[nEndPos] = '\0';
    sscanf(szCnt, "%llu", &m_unContentLen);
	return true;
}
void CAoHttp::Encode( const unsigned char* pucSrc, unsigned short usLen, std::string& strResult ){
	char szText[10] = {0};
	if (EN_ENCODE_GB2312 == m_ucEncode){
		for (unsigned short n = 0; n < usLen; ++n){
			if(pucSrc[n] > 0x7f){sprintf(szText, "%%%02x", pucSrc[n]);}
			else{sprintf(szText, "%c", (char)pucSrc[n]);}
			strResult += szText;
		}
	}
	else if(EN_ENCDOE_UTF8 == m_ucEncode){
		unsigned short m = 0;
		unsigned int unUtfLen = 10;
		char szTemp[10] = {0};
		for (unsigned short n = 0; n < usLen; ++n){		
			if(pucSrc[n] > 0x7f){
				unUtfLen = 10;
				CAoHelper::Gb2312CvtUtf8((char*)(pucSrc + n), 2, szTemp, unUtfLen);
				for (m = 0; m < unUtfLen; ++m){
					sprintf(szText, "%%%02x", (unsigned char)szTemp[m]);
					strResult += szText;
				}
				++n;
			}else{sprintf(szText, "%c", (char)pucSrc[n]);strResult += szText;}
		}
	}
}
