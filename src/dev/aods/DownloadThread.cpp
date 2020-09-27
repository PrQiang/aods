/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include <string.h>
#include "AodsDef.h"
#include "DownloadThread.h"
#include "../util/AoHttp.h"
#include "../util/AoFile.h"
#include "../util/Module.h"
#include "../util/AoEncrypt.h"
#include "../util/AoFileControl.h"
CDownloadThread::CDownloadThread(CModule* pModule):m_pModule(pModule){ m_bRun = true;m_nLoad = 0;}
CDownloadThread::~CDownloadThread(){    m_bRun = false;    m_evSignal.Signal();    Join();}
int CDownloadThread::Run(){
    std::string strTempFile;	DI_LST_IT it;	DI_LST lstTempDi;
	CAoFile af;	CAoHttp ah;
	int nOffset = 0;	int nRetryTimes = 0;
	unsigned int unCrc1 = 0;	unsigned int unCrc2 = 0;
	unsigned int unCrc3 = 0;    unsigned int unFileLen = 0;
	char szFileHash[64] = {0};
	ah.SetRecvSendTimeout(3000, 3000);
	unsigned char ucResult[5120] = {0};
	DOWNLOAD_RESULT* pDR = (DOWNLOAD_RESULT*)ucResult;
	while(m_bRun){
		m_lock.Lock();
		lstTempDi = m_lstDi;
		m_lstDi.clear();
		m_lock.Unlock();
		if (lstTempDi.size() == 0){
			m_evSignal.Wait(1000);
			continue;
		}
		for (it = lstTempDi.begin(); it != lstTempDi.end(); ++it, --m_nLoad){
			DOWNLOAD_ITEM& di = (*it);
            strTempFile = di.strSaveFileName + ".temp";
			for (nRetryTimes = 0; nRetryTimes < 3; ++nRetryTimes){
                if (EN_NOERROR != af.Open(strTempFile.c_str(), "wb")){ // 保存文件打开失败				
					pDR->ucRlt = EN_DR_SFA;
					continue;
				}
                ah.Clear();
				if(EN_NOERROR !=  ah.HttpRun(di.strUrl.c_str(), EN_HTTP_METHOD_GET, &af)){ // 下载文件失败				
					pDR->ucRlt = EN_DR_DFA;
					af.Close();
					continue;
				}
                af.Close();
                DecryptFile(strTempFile.c_str(), di.strSaveFileName.c_str(), di.strPwd.c_str()); // 解密文件
				unCrc1 = 0;unCrc2 = 0;unCrc3 = 0;// 校验文件
                CAoFileControl::CalcFileCrc32_3(di.strSaveFileName.c_str(), unCrc1, unCrc2, unCrc3, unFileLen);
                sprintf(szFileHash, "%08x%08x%08x%08x", unFileLen, unCrc1, unCrc2, unCrc3);
				pDR->ucRlt = (unsigned char)((0 == strcmp(szFileHash, di.strFileHash.c_str())) ? EN_DR_SUC : EN_DR_CFA);
                if (EN_DR_SUC != pDR->ucRlt){
                    CAoFileControl::DelFile(di.strSaveFileName.c_str());
                    m_pModule->LogErr("CDownloadThread::Run", "文件%s校验失败: %s!=%s", di.strSaveFileName.c_str(), szFileHash, di.strFileHash.c_str());
                }
				break;
			}
            CAoFileControl::DelFile(strTempFile.c_str());  // 删除中间文件
			memcpy(pDR->szBuf, di.strPrj.c_str(), di.strPrj.length() + 1);// 项目名称
			nOffset = (int)di.strPrj.length() + 1;			
			memcpy(pDR->szBuf + nOffset, di.strModule.c_str(), di.strModule.length() + 1);// 模块名称
			nOffset += (int)di.strModule.length() + 1;			
			memcpy(pDR->szBuf + nOffset, di.strFileVer.c_str(), di.strFileVer.length() + 1);// 文件版本
			nOffset += (int)di.strFileVer.length() + 1;			
			memcpy(pDR->szBuf + nOffset, di.strFileHash.c_str(), di.strFileHash.length() + 1);// 文件hash
			nOffset += (int)di.strFileHash.length() + 1;			
			memcpy(pDR->szBuf + nOffset, di.strUrl.c_str(), di.strUrl.length() + 1);// 文件url
			nOffset += (int)di.strUrl.length() + 1;
			memcpy(pDR->szBuf + nOffset, di.strSaveFileName.c_str(), di.strSaveFileName.length() + 1);// 文件保存路径
			nOffset += (int)di.strSaveFileName.length() + 1;
			memcpy(pDR->szBuf + nOffset, di.strPwd.c_str(), di.strPwd.length() + 1);// 文件加密码
			nOffset += (int)di.strPwd.length() + 1;
			m_pModule->PostMsg(EN_AO_MSG_DOWNLOAD_RESULT, ucResult, EN_DOWNLOAD_RESULT_SIZE + nOffset);
		}
	}
	return 0;
}
void CDownloadThread::Stop(){	m_bRun = false;	m_evSignal.Signal();Join();}
void CDownloadThread::AppendDownloadTask(const char* pszUrl, const char* pszPwd, const char* pszFileName, const char* pszPrj, const char* pszModule, const char*pszFileVer, const char* pszHash){
	++m_nLoad;
	DOWNLOAD_ITEM di;
	di.strUrl = pszUrl;	di.strPwd = pszPwd;	di.strSaveFileName = pszFileName;	di.strPrj = pszPrj;
	di.strModule = pszModule;	di.strFileVer = pszFileVer;	di.strFileHash = pszHash;	
	m_lock.Lock();	m_lstDi.push_back(di);	m_lock.Unlock();
	m_evSignal.Signal();
}
int CDownloadThread::Load(){return m_nLoad;}
void CDownloadThread::DecryptFile(const char* pszSrcFile, const char* pszDstFileName, const char* pszPwd){
	CAoFile afDst;	CAoFile afSrc;
    if (EN_NOERROR != afDst.Open(pszDstFileName, "wb") || EN_NOERROR != afSrc.Open(pszSrcFile, "rb")){return;}
	int nReaded = 0;	int nPwdLen = (int)strlen(pszPwd);	int nBufCap = nPwdLen * 200;
	CAoEncrypt ae;
	ae.SetKey((const unsigned char*)pszPwd, 16);
	char* pszData = (char*)MALLOC(nBufCap);
    while (0 < (nReaded = afSrc.Read(pszData, nBufCap))){
		ae.Decode((unsigned char*)pszData, nReaded);
		afDst.Write(pszData, nReaded);
	}
	SAFE_FREE(pszData);
}
