/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoEncrypt_h__
#define AoEncrypt_h__
class CAoEncrypt{
public:
    CAoEncrypt();
    ~CAoEncrypt();
    void SetSkDict(const unsigned char* pucSkDict, int nLen = 65536);
    void SetKey(const unsigned char* pucKey, int nLen);
	void Encode(unsigned char* pucBuf, int nLen);
	void Decode(unsigned char* pucBuf, int nLen);
private:
    void Default();
private:
    enum{
        EN_SKDICT_LEN = 65536,
        EN_KEY_LEN = 16
    };
    unsigned char* m_pucEnSkDict;
    unsigned char* m_pucDeSkDict;
    unsigned char* m_pucKey;
};

#endif // AoEncrypt_h__
