/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoDef.h"
#include "AoProcess.h"
#include <stdio.h>
#include <stdlib.h>
CAoProcess::CAoProcess(){}
CAoProcess::~CAoProcess(){}
int CAoProcess::RunCmd( const char* pszCmd ){
	if (pszCmd[0] == '\0'){return 0;}
	return system(pszCmd);
}