#include "../../../../src/dev/util/AoUnzip.h"
CAoUnzip::CAoUnzip() {}// ���캯��    
CAoUnzip::~CAoUnzip() {}// ��������
int CAoUnzip::OpenZipFile(const char* ) { return 0; }
int CAoUnzip::GotoFirstFile() { return 0; }
int CAoUnzip::GotoNextFile() { return 0; }
int CAoUnzip::GetCurrentFileInfo(char* , int , bool& ) { return 0; }
int CAoUnzip::UnzipCurentFile(CAoStream* , const char* ) { return 0; }