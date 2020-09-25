/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoProcess_h__
#define AoProcess_h__
class CAoProcess{
public:
    CAoProcess();
    ~CAoProcess();
	int RunCmd(const char* pszCmd);
};
#endif // AoProcess_h__