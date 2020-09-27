/* 
*/
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "AoStream.h"
#include "AoUnzip.h"
#include "./zlib/unzip.h"
CAoUnzip::CAoUnzip() {   m_pzf = NULL;}
CAoUnzip::~CAoUnzip() { if (NULL != m_pzf) {unzClose(m_pzf);m_pzf = NULL;}}
int CAoUnzip::OpenZipFile(const char* pszFileName) {
    m_pzf = unzOpen(pszFileName);
    return (NULL == m_pzf) ? 1 : UNZ_OK;    
}
int CAoUnzip::GotoFirstFile() {return (NULL == m_pzf) ? 1 : unzGoToFirstFile(m_pzf);}
int CAoUnzip::GotoNextFile(){return (NULL == m_pzf) ? 1 : unzGoToNextFile(m_pzf); }
int CAoUnzip::GetCurrentFileInfo(char* pszFileName, int nFileNameLen, bool& bIsDir) {
    if (NULL == m_pzf) {return 1;}
    unz_file_info64 ufi;
    int nRet = unzGetCurrentFileInfo64(m_pzf, &ufi, pszFileName, nFileNameLen, NULL, 0, NULL, 0);
    if (UNZ_OK == nRet) { bIsDir = (0 != (ufi.external_fa & EN_FILE_ATTRIBUTE_DIR)); }    
    return nRet;
}
int CAoUnzip::UnzipCurentFile(CAoStream* pAS, const char* pszPwd) {
    if (NULL == m_pzf)return 1;
    int nRet = ('\0' == *pszPwd) ? unzOpenCurrentFile(m_pzf) : unzOpenCurrentFilePassword(m_pzf, pszPwd);
    if (UNZ_OK != nRet) { return nRet; }
    char* pszBuf = (char*)malloc(10240);
    while(0 <(nRet = unzReadCurrentFile(m_pzf, pszBuf, 10240))) {pAS->Write(pszBuf, nRet);}
    free(pszBuf);
    return unzCloseCurrentFile(m_pzf);
}