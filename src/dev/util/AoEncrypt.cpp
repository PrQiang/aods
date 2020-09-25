/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdlib.h>
#include <string.h>
#include "AoDef.h"
#include "AoEncrypt.h"
CAoEncrypt::CAoEncrypt(){    Default();}
CAoEncrypt::~CAoEncrypt() { SAFE_FREE(m_pucEnSkDict); SAFE_FREE(m_pucKey); }
void CAoEncrypt::SetSkDict(const unsigned char* pucSkDict, int nLen ) {
    if (nLen != EN_SKDICT_LEN) {  return;  }
    for (int n = 0; n < EN_SKDICT_LEN; ++n) {
        m_pucEnSkDict[n] = pucSkDict[n];
        m_pucDeSkDict[(n / 256) * 256 + m_pucEnSkDict[n]] = (unsigned char)n;
    }
}
void CAoEncrypt::SetKey(const unsigned char* pucKey, int nLen) {
    if (nLen != EN_KEY_LEN) { return; }
    memcpy(m_pucKey, pucKey, EN_KEY_LEN);
}
void CAoEncrypt::Encode( unsigned char* pucBuf, int nLen ){
    for (int n = 0; n < nLen; ++n) {
        pucBuf[n] = m_pucEnSkDict[((m_pucKey[n % EN_KEY_LEN] << 8) & 0xFF00) + pucBuf[n]];
    }
}
void CAoEncrypt::Decode( unsigned char* pucBuf, int nLen ){
    for (int n = 0; n < nLen; ++n) {
        pucBuf[n] = m_pucDeSkDict[((m_pucKey[n % EN_KEY_LEN] << 8) & 0xFF00) + pucBuf[n]];
    }
}
void CAoEncrypt::Default(){
    m_pucEnSkDict = (unsigned char*)malloc(EN_SKDICT_LEN * 2);
    m_pucDeSkDict = m_pucEnSkDict + EN_SKDICT_LEN;
    for (int n = 0; n < EN_SKDICT_LEN; ++n) {
        m_pucEnSkDict[n] = (unsigned char)(n + 10);
        m_pucDeSkDict[(n / 256) * 256 + m_pucEnSkDict[n]] = (unsigned char)n;
    }
    m_pucKey = (unsigned char*)malloc(EN_KEY_LEN);
    for (unsigned char n = 0; n < (unsigned char)EN_KEY_LEN; ++n) { m_pucKey[n] = (unsigned char)0x35 + n;}
}