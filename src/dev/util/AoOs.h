/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#ifndef AoOs_h__
#define AoOs_h__
class CAoOs{
private:
    CAoOs();
    ~CAoOs();
public:
	static int SysType();
	static int OsType();    
    enum{
        EN_OS_TYPE_WINDOWS = 0,
        EN_OS_TYPE_CENTOS,
        EN_SYS_TYPE_X64 = 0,
        EN_SYS_TYPE_X86        
    };
    static void Sleep(int nMilliSeconds);
};
#endif // AoOs_h__