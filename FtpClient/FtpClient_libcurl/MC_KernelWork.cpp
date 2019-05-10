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
	if(eErrSuccess != m_FtpClient->Init())
	{
		LOG_ERROR("init ftp client fail");
		return false;
	}

	m_FtpClient->SetEventCB(DealFtpEvent, this);

	MutexInit(&m_Mutex);

	//�����̳߳�
	m_ThreadPool = CreateThreadpool(NULL);

	//�����̳߳������С���߳�����
	SetThreadpoolThreadMinimum(m_ThreadPool, vMinThreads);
	SetThreadpoolThreadMaximum(m_ThreadPool, vMaxThreads);

	//��ʼ���̳߳ػ�������
	InitializeThreadpoolEnvironment(&m_Cbe);

	//Ϊ�̳߳������̳߳ػ�������
	SetThreadpoolCallbackPool(&m_Cbe, m_ThreadPool);

	//������������
	m_PtpWork = CreateThreadpoolWork(WorkCallback, this, &m_Cbe);

	//�����ȡ�߳�
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

	WaitForThreadpoolWorkCallbacks(m_PtpWork, false);	//�ȴ���������	TRUE����ͼȡ���ύ�Ĺ�����������������������ȴ�	FALSE����ǰ�̹߳���ֱ�����������
	CloseThreadpoolWork(m_PtpWork);						//�رչ�������

	DestroyThreadpoolEnvironment(&m_Cbe);	//�����̳߳صĻ�������
	CloseThreadpool(m_ThreadPool);			//�ر��̳߳�

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
	//�о��й©����
	while(1)
	{
		MutexLock(&m_Mutex);
		if(true == m_TaskQueue.empty())
		{
			MutexUnlock(&m_Mutex);

			if(false != tBreak)
				break;

			Sleep(1000);
			continue;
		}
		MutexUnlock(&m_Mutex);

		tBreak = true;

		//���������ύ����
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

	printf("��ǰ�̣߳�#0x%0X, �����ļ���%s\n", GetCurrentThreadId(), tInfo.c_str());

	std::string tFilePath	= tInfo;
	std::string tFileName	= tFilePath.substr(tFilePath.rfind("/")+1);
	std::string tUploadPath = "ThreadPool_libcurl/" + tFileName;

	if(eErrSuccess != m_FtpClient->FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", tInfo.c_str(), tUploadPath.c_str()))
	{
		LOG_ERROR("�ļ��ϴ�ʧ�ܣ�file: %s", tFilePath.c_str());
	}
	else
	{
		LOG_INFO("�ļ��ϴ��ɹ���file: %s", tFilePath.c_str());
	}
	
	return;
}
//---------------------------------------------------------------------------
void MC_KernelWork::DealFtpEvent(FTPEvent event, void* pData, void* pUserData)
{
	switch(event)
	{
	case eUploadResult:
		{
			StructUploadResult* tResult = (StructUploadResult*) pData;
			printf("\n�ϴ������\n");
			printf("errno: %d, errstr: %s\n", tResult->eErrcode, tResult->eErrstr.c_str());
		}
		break;

	case eUploadProgress:
		{
			StructUploadProgress* tProgress = (StructUploadProgress*) pData;
			INT64 tNow		= tProgress->ulNow;
			INT64 tTotal	= tProgress->ulTotal;

			printf("\n�ϴ����ȣ�\n");
			printf("\rulnow: %10I64d, ultotal: %10I64d, progress: %1.2lf%% --- file: %s", tNow, tTotal, (float)tNow / tTotal  * 100, tProgress->strLocalFile);
		}
		break;

	default:
		break;
	}

	return;
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

		if(false == TraversalFilesInDir("F:/test", tFileVector, "iso"))	//iso�ļ�����
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
	//NULL==vSuffixName?(tStr += "*.*"):(tStr += "*." + vSuffixName);	//vDir��һ��Ŀ¼û�ж�Ӧ��׺�ļ�ʱ�����ش��󣬴�ʱֻ����*.*
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