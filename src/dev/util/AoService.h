#ifndef AoService_h__
#define AoService_h__
#ifdef WIN32
#include <Windows.h>
#endif
typedef void (*pfnService)();

class CAoService{
private:
	CAoService();
    ~CAoService();
public:
	static int Install(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc);
	static int Start(const char* pszServiceName);
    static int Stop(const char* pszServiceName);
	static int Uninstall(const char* pszServiceName);
	static void OnDispatch(const char* pszServiceName, pfnService pfnStop, pfnService pfnSuspend, pfnService pfnResum);
protected:
#ifdef LINUX
    static int InstallCentos6xService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc);
    static int InstallCentos7xService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc);
	static int InstallUbuntuService(const char* pszServiceName, const char* pszExe, const char* pszDisplay, const char* pszDesc);
#endif
protected:
	static pfnService m_pfnStop;
	static pfnService m_pfnSuspend;
	static pfnService m_pfnResumn;
#ifdef WIN32
	friend void WINAPI ServiceHandler(DWORD Opcode);
#endif
};
#endif // AoService_h__