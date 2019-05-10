#include "MC_GxxThreadPool.h"
//---------------------------------------------------------------------------
MC_GxxThreadPool::MC_GxxThreadPool(void)
{
	m_Available		= false;
	m_RefThreadPool = NULL;
	m_Mutex			= NULL;
}
//---------------------------------------------------------------------------
MC_GxxThreadPool::~MC_GxxThreadPool(void)
{
}
//---------------------------------------------------------------------------
bool MC_GxxThreadPool::OnLoad(int vMinThreads, int vMaxThreads)
{
	if(true == m_Available)
		return true;

	m_FtpClient = new MC_FtpClient();
	if(eErrSuccess != m_FtpClient->Init())
	{
		LOG_ERROR("init ftp client fail");
		return false;
	}
	m_FtpClient->SetEventCB(DealFtpEvent, this);

	MutexInit(&m_Mutex);

	//创建线程池	
	//设置线程池最大最小的线程数量
	m_RefThreadPool = GSRefThreadPool::Create(vMinThreads, vMaxThreads);

	//任务获取线程
	if(false == m_RefThreadPool->Start(GetTaskThreadFunc, this))
	{
		LOG_ERROR("start notify thread fail..");
		return false;
	}

	m_Available = true;
	return true;
}
//---------------------------------------------------------------------------
bool MC_GxxThreadPool::OnExit(void)
{
	if(false == m_Available)
		return true;

	if(NULL != m_FtpClient)
	{
		m_FtpClient->UnInit();
		delete m_FtpClient;
		m_FtpClient = NULL;
	}

	if(NULL != m_RefThreadPool)
	{
		m_RefThreadPool->StopAll();
		m_RefThreadPool->JoinAll();
		m_RefThreadPool->Unref();
		m_RefThreadPool = NULL;
	}

	MutexDestroy(&m_Mutex);

	m_Available = false;
	return true;
}
//---------------------------------------------------------------------------
bool MC_GxxThreadPool::DoWork()
{
	while(1)
	{
		
		MutexLock(&m_Mutex);
		if(true == m_TaskQueue.empty())
		{
			MutexUnlock(&m_Mutex);
			Sleep(1000);
			continue;
		}
		MutexUnlock(&m_Mutex);
		
		m_RefThreadPool->Start(WorkCallback, this);
		Sleep(1000);
	}

	return true;
}
//---------------------------------------------------------------------------
void MC_GxxThreadPool::DealFtpEvent(FTPEvent event, void* pData, void* pUserData)
{
	switch(event)
	{
	case eUploadResult:
		{
			StructUploadResult* tResult = (StructUploadResult*) pData;
			printf("\n上传结果：\n");
			printf("errno: %d, errstr: %s\n", tResult->eErrcode, tResult->eErrstr.c_str());
		}
		break;

	case eUploadProgress:
		{
			StructUploadProgress* tProgress = (StructUploadProgress*) pData;
			INT64 tNow		= tProgress->ulNow;
			INT64 tTotal	= tProgress->ulTotal;

			printf("\n上传进度：\n");
			printf("\rulnow: %10I64d, ultotal: %10I64d, progress: %1.2lf%% --- file: %s", tNow, tTotal, (float)tNow / tTotal  * 100, tProgress->strLocalFile);
		}
		break;

	default:
		break;
	}

	return;
}
//---------------------------------------------------------------------------
void GS_CALLBACK MC_GxxThreadPool::GetTaskThreadFunc(void *pUserParam, long lThreadID)
{
	MC_GxxThreadPool* tKernelWork = (MC_GxxThreadPool*)pUserParam;
	if(NULL == tKernelWork)
		return;

	tKernelWork->GetTaskWorkFunc();
	return;
}
//---------------------------------------------------------------------------
void MC_GxxThreadPool::GetTaskWorkFunc()
{
	do
	{
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
		Sleep(1000);
		//Sleep(10);		//泄露
		//Sleep(1000 * 60);	//没有泄露
	}while(1);
}
//---------------------------------------------------------------------------
void GS_CALLBACK MC_GxxThreadPool::WorkCallback( void *pUserParam, long lThreadID  )
{
	MC_GxxThreadPool* tKernelWork = (MC_GxxThreadPool*)pUserParam;
	tKernelWork->WorkFunc(lThreadID);

	return;
}
//---------------------------------------------------------------------------
void MC_GxxThreadPool::WorkFunc(long lThreadID)
{
	MutexLock(&m_Mutex);
	std::string tInfo = m_TaskQueue.front();
	m_TaskQueue.pop();
	MutexUnlock(&m_Mutex);


	printf("当前线程：#0x%0X, 本地文件：%s\n", lThreadID, tInfo.c_str());

	std::string tFilePath	= tInfo;
	std::string tFileName	= tFilePath.substr(tFilePath.rfind("/")+1);
	std::string tUploadPath = "ThreadPool_libcurl/" + tFileName;

	if(eErrSuccess != m_FtpClient->FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", tInfo.c_str(), tUploadPath.c_str()))
	{
		LOG_ERROR("文件上传失败，file: %s", tFilePath.c_str());
	}
	else
	{
		LOG_INFO("文件上传成功，file: %s", tFilePath.c_str());
	}

	return;
}
//---------------------------------------------------------------------------
bool MC_GxxThreadPool::TraversalFilesInDir(const char* vDir, std::vector<std::string>&vFileList, const char* vSuffixName, bool vIsRecursive)
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
std::string MC_GxxThreadPool::STRReplace(const std::string& vString, const std::string& vSubStrFrom, const std::string& vSubStrTo)
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