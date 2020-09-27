#include "../../../../src/dev/aods/DownloadThread.h"
#include "../../../../src/dev/aods/AodsDef.h"
#include "../../../../src/dev/util/Module.h"
CDownloadThread::CDownloadThread(CModule* pModule):m_pModule(pModule){}
CDownloadThread::~CDownloadThread(){}
int CDownloadThread::Run() { return 0; }
void CDownloadThread::Stop() { m_bRun = false;	m_evSignal.Signal(); Join(); }
void CDownloadThread::AppendDownloadTask(const char* pszUrl, const char* pszPwd, const char* pszFileName, const char* pszPrj, const char* pszModule, const char* pszFileVer, const char* pszHash) {
    unsigned char ucResult[5120] = { 0 };
    DOWNLOAD_RESULT* pDR = (DOWNLOAD_RESULT*)ucResult;
    if (0 == strcmp(pszPrj, "aom-suc0")) {pDR->ucRlt = EN_DR_SUC;   
	}else if (0 == strcmp(pszPrj, "aom-fai1")) {pDR->ucRlt = EN_DR_DFA;
	}else if (0 == strcmp(pszPrj, "aom-fai2")) {pDR->ucRlt = EN_DR_SFA;
    }else if (0 == strcmp(pszPrj, "aom-fai3")) {pDR->ucRlt = EN_DR_CFA;
    }else if (0 == strcmp(pszPrj, "aom-fai4")) {pDR->ucRlt = EN_DR_UFA;
	}else if (0 == strcmp(pszPrj, "aom-fai5")) {pDR->ucRlt = EN_DR_SUC;
    }
	memcpy(pDR->szBuf, pszPrj, strlen(pszPrj) + 1);// 项目名称
	int nOffset = (int)strlen(pszPrj) + 1;
	memcpy(pDR->szBuf + nOffset, pszModule, strlen(pszModule) + 1);// 模块名称
	nOffset += (int)strlen(pszModule) + 1;
	memcpy(pDR->szBuf + nOffset, pszFileVer, strlen(pszFileVer) + 1);// 文件版本
	nOffset += (int)strlen(pszFileVer) + 1;
	memcpy(pDR->szBuf + nOffset, pszHash, strlen(pszHash) + 1);// 文件hash
	nOffset += (int)strlen(pszHash) + 1;
	memcpy(pDR->szBuf + nOffset, pszUrl, strlen(pszUrl) + 1);// 文件url
	nOffset += (int)strlen(pszUrl) + 1;
	memcpy(pDR->szBuf + nOffset, pszFileName, strlen(pszFileName) + 1);// 文件保存路径
	nOffset += (int)strlen(pszFileName) + 1;
	memcpy(pDR->szBuf + nOffset, pszPwd, strlen(pszPwd) + 1);// 文件加密码
	nOffset += (int)strlen(pszPwd) + 1;
	m_pModule->PostMsg(EN_AO_MSG_DOWNLOAD_RESULT, ucResult, EN_DOWNLOAD_RESULT_SIZE + nOffset);
}
int CDownloadThread::Load() { return 0; }