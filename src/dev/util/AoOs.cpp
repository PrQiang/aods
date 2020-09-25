/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoOs.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
CAoOs::CAoOs(){}
CAoOs::~CAoOs(){}
int CAoOs::SysType(){
#ifdef WIN32
    typedef void (WINAPI* pfn)(LPSYSTEM_INFO lpSystemInfo);
    pfn GetNativeSystemInfo = (pfn)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
    if (NULL != GetNativeSystemInfo){
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        if ((PROCESSOR_ARCHITECTURE_AMD64 == si.wProcessorArchitecture) || (PROCESSOR_ARCHITECTURE_IA64 == si.wProcessorArchitecture)){
            return EN_SYS_TYPE_X64;
        }
    }
    return EN_SYS_TYPE_X86;
#else
    return (sizeof(void*) == 8)? EN_SYS_TYPE_X64 : EN_SYS_TYPE_X86;
#endif
}
int CAoOs::OsType(){
#ifdef WIN32
    return 0;
#elif defined LINUX
    return 1;
#endif
}
void CAoOs::Sleep(int nMilliSeconds){
#ifdef WIN32
    ::Sleep((DWORD)nMilliSeconds);
#else
    timeval tv={nMilliSeconds/1000, 1000 * (nMilliSeconds%1000)};
    select(0, NULL, NULL, NULL, &tv);
#endif
}