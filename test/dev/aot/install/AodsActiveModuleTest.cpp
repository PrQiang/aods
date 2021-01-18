/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "../pch.h"
#include <stdlib.h>
#include "../../../../src/dev/install/AodsActiveModule.h"
#include "../../../../src/dev/util/ModuleDispatcher.h"
#include "AodsActiveModuleTestSuc.h"
#include "AodsActiveModuleTestFai.h"
TEST(AodsActiveModuleFai, ModuleTest) {
    CAodsActiveModule::Instance()->Start();
    CModuleDispatcher::Instance()->AppendModule(CAodsActiveModule::Instance());
    CAodsActiveModuleTestFai* pTest = new CAodsActiveModuleTestFai();
    pTest->Start();
    CModuleDispatcher::Instance()->AppendMonitorModule(pTest);
    EXPECT_TRUE(pTest->Test());
    CAodsActiveModule::Instance()->PostMsg(EN_AO_MSG_EXIT);
    CAodsActiveModule::Instance()->Join();
    pTest->PostMsg(EN_AO_MSG_EXIT);
    pTest->Join();
    delete pTest;
    CAodsActiveModule::Instance()->Release();
    CModuleDispatcher::Instance()->Release();
}
TEST(AodsActiveModuleSuc, ModuleTest) {
    CAodsActiveModule::Instance()->Start();
    CModuleDispatcher::Instance()->AppendModule(CAodsActiveModule::Instance());
    CAodsActiveModuleTestSuc* pTest = new CAodsActiveModuleTestSuc();
    pTest->Start();
    CModuleDispatcher::Instance()->AppendMonitorModule(pTest);
    EXPECT_TRUE(pTest->Test());
    CAodsActiveModule::Instance()->PostMsg(EN_AO_MSG_EXIT);
    CAodsActiveModule::Instance()->Join();
    pTest->PostMsg(EN_AO_MSG_EXIT);
    pTest->Join();
    delete pTest;
    CAodsActiveModule::Instance()->Release();
    CModuleDispatcher::Instance()->Release();
}