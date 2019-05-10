#include "MC_KernelWork.h"
//---------------------------------------------------------------------------
typedef struct
{
	FtpClientHandle*	hFtpClient;

	bool*			bTaskFlag;
	MutexHandle*	hMutex;
	map<std::string,std::string>* mapTask;
} FtpClientContext;
//---------------------------------------------------------------------------
MC_KernelWork::MC_KernelWork(void)
{
	m_Avaliable = false;
}
//---------------------------------------------------------------------------
MC_KernelWork::~MC_KernelWork(void)
{
}
//---------------------------------------------------------------------------
bool MC_KernelWork::OnLoad(void)
{
	if(true == m_Avaliable)
		return true;

	m_Avaliable = true;
	return true;
}
//---------------------------------------------------------------------------
bool MC_KernelWork::OnExit(void)
{
	if(false == m_Avaliable)
		return true;

	m_Avaliable = false;
	return true;
}
//---------------------------------------------------------------------------
void __stdcall MC_KernelWork::OnEvent(const char* vJson, unsigned int vLen, void* vContext)
{
	FtpClientContext* tFtpClientContext=(FtpClientContext*)vContext;
	// {"code":"1","desc":"Upload progress.","dir":"0","dnow":"0.000000","dtotal":"0.000000","filename":"1.mp4","id":"81033EBA-F377-4783-897D-F541A457E0F5","unow":"8601600.000000","utotal":"8609448.000000"}

	Json::Reader	tReader;
	Json::Value		tRoot;
	if(tReader.parse(std::string(vJson), tRoot))
	{
		std::string tDesc		= tRoot["desc"].asString();
		std::string tFileName	= tRoot["filename"].asString();
		std::string tNow		= tRoot["unow"].asString();
		std::string tTotal		= tRoot["utotal"].asString();
		std::string tId			= tRoot["id"].asString();
		std::string tDir		= tRoot["dir"].asString();
		std::string tCode		= tRoot["code"].asString();

		if("0" == tDir && "1" != tCode)	//上传且不是进度事件
		{
			MutexLock(tFtpClientContext->hMutex);
			map<std::string,std::string>::iterator tIter = tFtpClientContext->mapTask->find(tId);
			if(tIter != tFtpClientContext->mapTask->end())
			{
				if("0" == tCode)
					LOG_INFO("上传成功，文件：%s, taskId: %s", tFileName.c_str(), tIter->first.c_str());
				else
				{
					LOG_ERROR("上传失败，文件：%s, code: %s, desc: %s", tFileName.c_str(), tCode.c_str(), tDesc.c_str());
				}

				*tFtpClientContext->bTaskFlag = false; //标记当前任务完成
				tFtpClientContext->mapTask->erase(tIter);

			}
			MutexUnlock(tFtpClientContext->hMutex);
		}


		/*else if("Upload progress." == tDesc)
		{
			__int64 tNowSize	= _atoi64(tNow.c_str());
			__int64 tTotalSize	= _atoi64(tTotal.c_str());

			double tProgress = 0;
			if(0 != tTotalSize)
			{
				tProgress = (double)tNowSize / tTotalSize * 100;
			}

			printf("\rFile: %s, Progress: %.2lf%%", tFileName.c_str(), tProgress);
			Sleep(10);
		}*/
	}
}
//---------------------------------------------------------------------------
bool MC_KernelWork::Upload()
{
	if(false == m_Avaliable)
		return false;

	std::queue<std::string> tFileVector;
	if(false == TraversalFilesInDir("F:/test", tFileVector, "iso"))	//iso文件不传
	{
		LOG_ERROR("TraversalFilesInDir fail, errno: %d", GetLastError());
		return false;
	}

	bool	tChangeIp	= false;
	BOOL_T	tRetVal		= FALSE;
	FtpClientHandle	tFtpClientHandle = NULL;
	std::vector<FtpClientHandle> tHandleList;

	MutexHandle tTaskMutex = NULL;
	MutexInit(&tTaskMutex);	//句柄+1

	map<std::string,std::string> tTaskMap;//记录任务<TaskId,sid>

	bool tTaskFlag	=	false;//标记是否正在执行任务
	FtpClientContext tContext;
	tContext.bTaskFlag	= &tTaskFlag;
	tContext.hMutex		= &tTaskMutex;
	tContext.mapTask	= &tTaskMap;

	while(false == tFileVector.empty())
	{
		/************************************************************************/
		MutexLock(&tTaskMutex);

		//开始上传任务, 依次上传队列中的文件
		std::string tSrcFileName	= tFileVector.front();
		std::string	tSrcFile		= tSrcFileName.substr(tSrcFileName.rfind("\\")+1);
		std::string	tDestFileName	= "SDK\\" + tSrcFile;	

		std::string	tIp		= "";
		std::string	tUser	= "";
		std::string	tPwd	= "";
		if(false == tChangeIp)	//验证是否能传到不同ftp地址，结果：能
		{
			tIp			= "ftp://127.0.0.1/";
			tUser		= "kinkoo";
			tPwd		= "6787.";
			//tChangeIp	= true;
		}
		else
		{
			tIp			= "ftp://10.10.17.86/";
			tUser		= "gmvcsws";
			tPwd		= "1";
			tChangeIp	= false;
		}

		//句柄+6 +1
		int tRet = CreateFtpClient(&tFtpClientHandle, tIp.c_str(), tUser.c_str(), tPwd.c_str(), OnEvent, &tContext);
		if(0 != tRet)
		{
			LOG_ERROR("CreateFtpClient fail");
			tFileVector.pop();
			continue;
		}

		char tTaskId[128] = {0};
		//句柄+1
		tRetVal = NewTaskid(tTaskId, sizeof(tTaskId));//为这次任务生成GUID
		if(FALSE == tRetVal)
		{
			LOG_ERROR("NewTaskid fail");
			tFileVector.pop();
			CloseFtpClient(&tFtpClientHandle, TRUE);
			continue;
		}

		//句柄+1
		tRetVal = UploadFile(tFtpClientHandle, tTaskId, tSrcFileName.c_str(), tDestFileName.c_str());
		if(FALSE == tRetVal)
		{
			LOG_ERROR("UploadFile fail");
			tFileVector.pop();
			CloseFtpClient(&tFtpClientHandle, TRUE);
			continue;
		}

		tTaskMap.insert(pair<std::string,std::string>(std::string(tTaskId), tSrcFileName.c_str()));//记录任务<TaskId,sid>
		tTaskFlag = true;	//标记正在执行任务
		tFileVector.pop();	//删除队列中的第一个任务
		LOG_INFO("正在上传文件: %s, taskId: %s, ip: %s", tSrcFileName.c_str(), tTaskId, tIp.c_str());
		
		MutexUnlock(&tTaskMutex);
		/************************************************************************/

		bool tIsLog = false;
		LOG_INFO("当前任务数量：%d", tTaskMap.size());	//句柄+1 +2？
		while(5 == tTaskMap.size())	//最多同时进行任务数
		{
			if(false == tIsLog)
			{
				LOG_INFO("上传队列已满, 请等待... ...");
				tIsLog = true;
			}

			Sleep(1000);
		}

		Sleep(1000);
	}

	bool tIsLog = false;
	while(0 != tTaskMap.size())		//等待当前任务完成 - 任务队列
	{
		if(false == tIsLog)
		{
			LOG_INFO("队列残留任务，等待... ...");
			tIsLog = true;
		}

		Sleep(1000);
	}

	CloseFtpClient(&tFtpClientHandle, TRUE);	//关闭FtpClient
	tFtpClientHandle = NULL;
	MutexDestroy(&tTaskMutex);
	tTaskMutex = NULL;

	LOG_INFO("上传完毕");
	return true;
}
//---------------------------------------------------------------------------
bool MC_KernelWork::TraversalFilesInDir(const char* vDir, std::queue<std::string>&vFileList, const char* vSuffixName, bool vIsRecursive)
{
	if(vDir == NULL)			return false;
	if(strcmp(vDir, "") == 0)	return false;

	string tDir = STRReplace(vDir, "/", "\\");
	if('\\' != tDir[tDir.size()-1])
		tDir += "\\";

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

		vFileList.push(tFileName);
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
