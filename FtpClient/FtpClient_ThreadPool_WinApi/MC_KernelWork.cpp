#include "MC_KernelWork.h"
//---------------------------------------------------------------------------
MC_KernelWork::MC_KernelWork(void)
{
	m_Available			= false;
	m_GetTaskThreadRun	= false;

	m_FtpClient			= NULL;
	m_Mutex				= NULL;
}
//---------------------------------------------------------------------------
MC_KernelWork::~MC_KernelWork(void)
{
}
//---------------------------------------------------------------------------
bool MC_KernelWork::OnLoad(int vMinThreads, int vMaxThreads)
{
	if(true == m_Available)
		return true;

	m_FtpClient = new MC_FtpClient();
	if(false == m_FtpClient->Init())
	{
		LOG_ERROR("init ftp client fail");
		return false;
	}

	MutexInit(&m_Mutex);

	//创建线程池
	m_ThreadPool = CreateThreadpool(NULL);

	//设置线程池最大最小的线程数量
	SetThreadpoolThreadMinimum(m_ThreadPool, vMinThreads);
	SetThreadpoolThreadMaximum(m_ThreadPool, vMaxThreads);

	//初始化线程池环境变量
	InitializeThreadpoolEnvironment(&m_Cbe);

	//为线程池设置线程池环境变量
	SetThreadpoolCallbackPool(&m_Cbe, m_ThreadPool);

	//创建工作对象
	m_PtpWork = CreateThreadpoolWork(WorkCallback, this, &m_Cbe);

	//任务获取线程
	m_GetTaskThreadRun = true;
	WINThreadCreate(&m_GetTaskThreadHandle, GetTaskThreadFunc, this);

	m_Available = true;
	return true;
}
//---------------------------------------------------------------------------
bool MC_KernelWork::OnExit(void)
{
	if(false == m_Available)
		return true;

	MutexDestroy(&m_Mutex);

	WaitForThreadpoolWorkCallbacks(m_PtpWork, false);	//等待工作结束	TRUE，试图取消提交的工作项。如果工作项已启动，则等待	FALSE，当前线程挂起，直到工作项完成
	CloseThreadpoolWork(m_PtpWork);						//关闭工作对象

	DestroyThreadpoolEnvironment(&m_Cbe);	//清理线程池的环境变量
	CloseThreadpool(m_ThreadPool);			//关闭线程池

	if(true == m_GetTaskThreadRun)
	{
		m_GetTaskThreadRun = false;
		WINThreadJoin(m_GetTaskThreadHandle);
	}

	if(NULL != m_FtpClient)
	{
		m_FtpClient->UnInit();
		delete m_FtpClient;
		m_FtpClient = NULL;
	}

	m_Available = false;
	return true;
}
//---------------------------------------------------------------------------
bool MC_KernelWork::DoWork()
{
	bool tBreak = false;
	//有句柄泄漏问题
	while(1)
	{
		MutexLock(&m_Mutex);
		if(true == m_TaskQueue.empty())
		{
			MutexUnlock(&m_Mutex);

			if(false != tBreak)
				return true;

			Sleep(1000);
			continue;
		}
		MutexUnlock(&m_Mutex);

		//有任务，则提交工作
		tBreak = true;
		SubmitThreadpoolWork(m_PtpWork);	
		Sleep(1000);
	}

	return true;
}
//---------------------------------------------------------------------------
VOID CALLBACK MC_KernelWork::WorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	MC_KernelWork* tKernelWork = (MC_KernelWork*)Context;
	tKernelWork->WorkFunc(Work);

	return;
}
//---------------------------------------------------------------------------
void MC_KernelWork::WorkFunc(PTP_WORK Work)
{
	MutexLock(&m_Mutex);
	std::string tInfo = m_TaskQueue.front();
	m_TaskQueue.pop();
	MutexUnlock(&m_Mutex);

	printf("当前线程：#0x%0X, 本地文件：%s\n", GetCurrentThreadId(), tInfo.c_str());

	std::string tFilePath	= tInfo;
	std::string tFileName	= tFilePath.substr(tFilePath.rfind("/")+1);

	SOCKET tCtrlSocket = 0;
	if(false == m_FtpClient->Connect(tCtrlSocket, "127.0.0.1", 21, "kinkoo", "6787."))
	{
		LOG_ERROR("connect fail, file: %s", tFilePath.c_str());
		m_FtpClient->DisConnect(tCtrlSocket);
		return;
	}

	LOG_INFO("准备上传文件, file: %s", tFilePath.c_str());
	std::string tUploadPath = "ThreadPool_WinApi/" + tFileName;
	if(false == m_FtpClient->UploadFile(tCtrlSocket, tFilePath.c_str(), tUploadPath.c_str(), true, NULL))
	{
		LOG_ERROR("UpLoad fail, file: %s", tFilePath.c_str());
		m_FtpClient->DisConnect(tCtrlSocket);
		return;
	}
	LOG_INFO("文件上传成功，file: %s", tFilePath.c_str());

	m_FtpClient->DisConnect(tCtrlSocket);
	return;
}
//---------------------------------------------------------------------------
int MC_KernelWork::ProgressCallback	(void *clientp, INT64 dltotal, INT64 dlnow, INT64 ultotal, INT64 ulnow)
{
	char* tFilePath = (char*)clientp;

	if(ultotal)
	{
		printf("%s\n", tFilePath);
		printf("ulnow: %10I64d, ultotal: %10I64d, progress: %2.1f\n", ulnow, ultotal, (float)ulnow / ultotal);
		
	}

	if(dltotal)
	{
		printf("%s\n", tFilePath);
		printf("dlnow: %10I64d, dltotal: %10I64d, progress: %2.1f\n", dlnow, dltotal, (float)dlnow / dltotal);
	}

	return 0;
}
//---------------------------------------------------------------------------
void* MC_KernelWork::GetTaskThreadFunc(void* p)
{
	MC_KernelWork* tKernelWork = (MC_KernelWork*)p;
	if(NULL == tKernelWork)
		return 0;

	tKernelWork->GetTaskWorkFunc();
	return 0;
}
//---------------------------------------------------------------------------
void MC_KernelWork::GetTaskWorkFunc()
{
	do
	{
		if(false == m_GetTaskThreadRun)
		{
			Sleep(1000);
			return;
		}

		std::vector<std::string> tFileVector;
		tFileVector.clear();

		if(false == TraversalFilesInDir("F:/test", tFileVector, "iso"))	//iso文件不传
		{
			LOG_ERROR("TraversalFilesInDir fail");
			
			Sleep(1000);
			continue;
		}

		
		for(int i=0; i<tFileVector.size(); i++)
		{
			std::string tInfo = STRReplace(tFileVector[i], "\\", "/").c_str();

			MutexLock(&m_Mutex);
			m_TaskQueue.push(tInfo);
			MutexUnlock(&m_Mutex);
		}
		Sleep(10);
		m_GetTaskThreadRun = false;
		//Sleep(1000 * 60);
	}while(1);
}
//---------------------------------------------------------------------------
bool MC_KernelWork::TraversalFilesInDir(const char* vDir, std::vector<std::string>&vFileList, const char* vSuffixName, bool vIsRecursive)
{
	if(vDir == NULL)			return false;
	if(strcmp(vDir, "") == 0)	return false;

	string tDir = STRReplace(vDir, "\\", "/");
	if('/' != tDir[tDir.size()-1])
		tDir += "/";

	//string tStr = tDir;
	//NULL==vSuffixName?(tStr += "*.*"):(tStr += "*." + vSuffixName);	//vDir第一层目录没有对应后缀文件时，返回错误，此时只能用*.*
	string tStr = tDir + "*.*";
	WIN32_FIND_DATAA tFindFileData;
	HANDLE tFindHandle = FindFirstFileA(tStr.c_str(), &tFindFileData);
	if(INVALID_HANDLE_VALUE == tFindHandle)
	{
		LOG_ERROR("INVALID_HANDLE_VALUE, tStr: %s, errno: %d", tStr.c_str(), GetLastError());
		return false;
	}

	do
	{
		if(0 == strcmp(tFindFileData.cFileName, ".") || 0 == strcmp(tFindFileData.cFileName, ".."))
		{
			continue;
		}

		if(tFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(false == vIsRecursive)
				continue;

			std::string tSubDir = tDir + tFindFileData.cFileName;
			TraversalFilesInDir(tSubDir.c_str(), vFileList, vSuffixName, vIsRecursive);
		}

		std::string tFileName = tDir + tFindFileData.cFileName;

		if(NULL != vSuffixName)
		{
			if(vSuffixName == tFileName.substr(tFileName.rfind(".")+1))
				//if(vSuffixName != tFileName.substr(tFileName.rfind(".")+1))
				continue;
		}

		vFileList.push_back(tFileName);
	}while(FindNextFileA(tFindHandle, &tFindFileData));

	FindClose(tFindHandle);
	return true;
}
//---------------------------------------------------------------------------
std::string MC_KernelWork::STRReplace(const std::string& vString, const std::string& vSubStrFrom, const std::string& vSubStrTo)
{
	std::string tString = vString;
	std::string::size_type tPos = 0;
	std::string::size_type tFormLen = vSubStrFrom.size();
	std::string::size_type tToLen = vSubStrTo.size();

	while( (tPos=tString.find(vSubStrFrom, tPos)) != std::string::npos)
	{
		tString.replace( tPos, tFormLen, vSubStrTo);
		tPos += tToLen;
	}

	return tString;
}
//---------------------------------------------------------------------------