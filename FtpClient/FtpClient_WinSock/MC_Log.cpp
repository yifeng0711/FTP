#include "MC_Log.h"
//---------------------------------------------------------------------------
MC_Log gLog;
//---------------------------------------------------------------------------
std::string _GetModuleName()
{
	char tRunPath[MAX_PATH] = {0};
	if(FALSE == GetModuleFileNameA(NULL, tRunPath, MAX_PATH))
		return "";

	char* tPos = strrchr(tRunPath, '\\');
	if(0 == tPos)
		return "";

	tPos++;
	char * tPos1 = strrchr(tPos, '.');
	if(0 == tPos1)
		return tPos;

	*tPos1 = 0;
	return tPos;
}
//---------------------------------------------------------------------------
MC_Log::MC_Log(const char* vLogFileName, int vBufferSize)
{
	m_BufferSize = vBufferSize;
	if(0 == vLogFileName)
	{
		std::string tModuleName = _GetModuleName();
		if(0 == tModuleName.size())
			tModuleName = "zap_undefined";

		CreateDirectoryA("c:/workstation-logs",NULL);
		m_LogFileName = std::string("c:/workstation-logs/")+tModuleName+".log";
	}
	else
	{
		m_LogFileName = vLogFileName;
	}

	m_EventHandle	= CreateEvent(0, FALSE, TRUE, 0);
	if(0 == m_EventHandle)
	{
		OutputDebugStringA("Log Init Failed,CreateEvent failed\n");
	}
}
//---------------------------------------------------------------------------
MC_Log::~MC_Log(void)
{
	if(0!=m_EventHandle)
		CloseHandle(m_EventHandle);
}
//---------------------------------------------------------------------------
void MC_Log::Log(const char* vLogData)
{
	if(0 == vLogData)
		return ;

	DWORD tWaitRet = WaitForSingleObject(m_EventHandle, 1000*3);
	if(WAIT_TIMEOUT == tWaitRet || WAIT_FAILED == tWaitRet)
	{
		return ;
	}

	do
	{
		//检查日志文件大小
		FILE* fp = 0;
		if(0 != fopen_s(&fp, m_LogFileName.c_str(), "a+") || 0 == fp)
		{
			break;
		}
		fseek(fp, 0L, SEEK_END );
		int tFileSize = ftell(fp);
		fclose(fp);
		tFileSize = tFileSize / 1024;	 //KB
		
		if(tFileSize > m_BufferSize)
		{
			/*
			//保留原来的日志，test.log为最新的日志，test_1.log为最旧的日志
			BOOL		tCopyRet		= FALSE;
			int			tIndex			= 0;
			std::string tLogFileName	= m_LogFileName.substr(0, m_LogFileName.rfind("."));

			do 
			{
				char		tNewFileName[MAX_PATH]	= {0};
				sprintf_s(tNewFileName, sizeof(tNewFileName), "%s_%d.log", tLogFileName.c_str(), tIndex++);
				tCopyRet = CopyFileA(m_LogFileName.c_str(), tNewFileName, TRUE);
			} while (!tCopyRet);
			*/
			remove(m_LogFileName.c_str());
		}

		//添加日志
		SYSTEMTIME tCurSysTime;
		GetLocalTime(&tCurSysTime);
		char tCurTime[128] = {0};
		sprintf_s(tCurTime,sizeof(tCurTime),"%02d-%02d-%02d_%02d:%02d:%02d",tCurSysTime.wYear,tCurSysTime.wMonth,tCurSysTime.wDay,tCurSysTime.wHour,tCurSysTime.wMinute,tCurSysTime.wSecond);
		string tLog = string(tCurTime) + string(" ") + string(vLogData) + "\n";

		if(0 != fopen_s(&fp, m_LogFileName.c_str(), "a+")  || 0 == fp)
		{
			break;
		}

		OutputDebugStringA(tLog.c_str());
		//printf("%s\n", tLog.c_str());
		fwrite(tLog.c_str(), tLog.size(), 1, fp);
		fclose(fp);
	}while(0);

	SetEvent(m_EventHandle);

	return ;
}
//---------------------------------------------------------------------------
