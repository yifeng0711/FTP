#ifndef MC_KernelWork_H
#define MC_KernelWork_H
//---------------------------------------------------------------------------
#include "WinMain.h"
#include <queue>

#include "FtpClient/inc/FtpClient.h"
#pragma comment(lib, "FtpClient/lib/FtpClient.lib")
//---------------------------------------------------------------------------
class MC_KernelWork
{
public:
	MC_KernelWork(void);
	~MC_KernelWork(void);

	bool	OnLoad	(void);
	bool	OnExit	(void);

	bool	Upload	(void);

private:
	bool		TraversalFilesInDir	(const char* vDir, std::queue<std::string>&vFileList, const char* vSuffixName=NULL, bool vIsRecursive=true);	//±éÀúÄ¿Â¼
	std::string	STRReplace			(const std::string& vString, const std::string& vSubStrFrom, const std::string& vSubStrTo);

	static	void __stdcall OnEvent(const char* vJson, unsigned int vLen, void* vContext);
private:
	bool	m_Avaliable;

	FtpClientHandle	m_FtpClientHandle;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------