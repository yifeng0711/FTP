 #ifndef MC_KernelWork_H
 #define MC_KernelWork_H
//---------------------------------------------------------------------------
#include <Windows.h>
#include <vector>
#include <queue>
#include "util.h"
#include "MC_FtpClient.h"
#include "MC_Log.h"
//---------------------------------------------------------------------------
struct TaskInfo
{
	std::string strFileName;
};
//---------------------------------------------------------------------------
class MC_KernelWork
{
public:
	MC_KernelWork(void);
	~MC_KernelWork(void);

	bool	OnLoad		(int vMinThreads, int vMaxThreads);
	bool	OnExit		(void);

	bool	DoWork		(void);

private:
	bool		TraversalFilesInDir	(const char* vDir, std::vector<std::string>&vFileList, const char* vSuffixName=NULL, bool vIsRecursive=false);
	std::string STRReplace			(const std::string& vString, const std::string& vSubStrFrom, const std::string& vSubStrTo);

	static	VOID CALLBACK	WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
	void					WorkFunc	(PTP_WORK Work);


	static	void	DealFtpEvent	(FTPEvent event, void* pData, void* pUserData);

private:	//Ïß³Ìº¯Êý

	//
	bool            m_GetTaskThreadRun;
	ThreadHandle    m_GetTaskThreadHandle;
	static void*    GetTaskThreadFunc	(void* p);
	void			GetTaskWorkFunc		(void);

private:
	bool					m_Available;

	PTP_POOL				m_ThreadPool;
	TP_CALLBACK_ENVIRON		m_Cbe;

	MutexHandle				m_Mutex;
	std::queue<std::string>	m_TaskQueue;
	PTP_WORK				m_PtpWork;

	MC_FtpClient*			m_FtpClient;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
