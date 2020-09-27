/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "InstallDef.h"
#include "AodsActiveModule.h"
#include "../util/CJson.h"
#include "../util/CommuModule.h"
#include "../util/AoProcess.h"
#include "../util/AoCfgModule.h"
#include "../util/AoLogModule.h"
#include "../util/AoFileControl.h"
#include "../util/ModuleDispatcher.h"
int install(const char* pszExe, unsigned char ucIndex);
int main(int argc, char* argv[]){
    if(argc > 1)return install(argv[0], (unsigned char)atoi(argv[1]));
    int nIndex = 0;
    do {
        std::cout << "请输入有效安装序列段[1,255]:\t" << std::endl;
        std::cin >> nIndex;
        if (nIndex < 1 || nIndex > 255) {continue;}
        return install(argv[0], (unsigned char)nIndex);
    } while (1);
    return 0;
}
int install(const char* pszExe, unsigned char ucIndex){
    char szFile[1024] = { 0 };
#ifdef WIN32
    if (pszExe[1] == ':'){strcpy(szFile, pszExe);}// 绝对路径
    else{// 相对路径
        char szDir[EN_MAX_PATH_LEN] = { 0 };
        CAoFileControl::GetCwd(szDir, EN_MAX_PATH_LEN);
        sprintf(szFile, "%s\\%s", szDir, pszExe);
    }
#else
    if (pszExe[0] == '/'){strcpy(szFile, pszExe);} //绝对路径
    else{ // 相对路径
        char szDir[EN_MAX_PATH_LEN] = { 0 };
        CAoFileControl::GetCwd(szDir, EN_MAX_PATH_LEN);
        sprintf(szFile, "%s/%s", szDir, pszExe);
    }
#endif
    int n = (int)strlen(szFile) - 1;
    for (; n >= 0 && szFile[n] != '\\' && szFile[n] != '/'; --n);
    szFile[n] = '\0';
    CAoFileControl::SetCurAppPath(szFile);
    CAoFileControl::SetCurModuleFileName(szFile + n + 1);
    CModule* pModules[] = { CCommuModule::Instance(), CAoCfgModule::Instance(), CAoLogModule::Instance(), CAodsActiveModule::Instance(), NULL };
    for (int n = 0; NULL != pModules[n]; ++n){
        pModules[n]->Start();
        CModuleDispatcher::Instance()->AppendModule(pModules[n]);
    }
    // 向激活模块发送激活消息
    CAodsActiveModule::Instance()->PostMsg(EN_AO_MSG_ACTIVE_PARA, &ucIndex, 1);
    // 向配置模块发送加载配置
    CAoCfgModule::Instance()->PostMsg(EN_AO_MSG_CFG_START);
    for (int n = 0; NULL != pModules[n]; ++n){pModules[n]->Join();}
    return 0;
}