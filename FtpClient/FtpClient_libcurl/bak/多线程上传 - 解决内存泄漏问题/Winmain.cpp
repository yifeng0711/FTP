//---------------------------------------------------------------------------
#include "MC_GxxThreadPool.h"

#include <Windows.h>
#include <iostream>
#include <time.h>
#include "MC_Log.h"

#include "MC_KernelWork.h"
#include "MC_FtpClient.h"
//---------------------------------------------------------------------------
using namespace std; 
//---------------------------------------------------------------------------
DWORD ThreadFunc1(void* p)
{
	MC_FtpClient* tFtpClient = (MC_FtpClient*)p;
	tFtpClient->FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/ftp_upload_test_file.txt", "FtpClient_libcurl/1.txt");

	return 0;
}
//---------------------------------------------------------------------------
DWORD ThreadFunc2(void* p)
{
	MC_FtpClient* tFtpClient = (MC_FtpClient*)p;
	tFtpClient->FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/ftp_upload_test_file.txt", "FtpClient_libcurl/2.txt");

	return 0;
}
//---------------------------------------------------------------------------
void DealFtpEvent(FTPEvent event, void* pData, void* pUserData)
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
			printf("\rulnow: %10I64d, ultotal: %10I64d, progress: %1.2lf%%", tNow, tTotal, (float)tNow / tTotal  * 100);
		}
		break;

	default:
		break;
	}

	return;
}
//---------------------------------------------------------------------------
int main(int argc, char* argv[]) 
{ 
	/************************************************************************/
	////接口测试
	//MC_FtpClient tFtpClient;
	//tFtpClient.Init();
	//tFtpClient.SetEventCB(DealFtpEvent, 0);*/

	//tFtpClient.FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/ftp_upload_test_file.txt", "FtpClient_libcurl/1.txt");

	////tFtpClient.FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/2.flv", "FtpClient_libcurl/1.flv");
	////Sleep(10);
	////tFtpClient.FTPDeleteFile("127.0.0.1", 21, "kinkoo", "6787.", "FtpClient_libcurl/1.flv");

	////tFtpClient.FtpDownloadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/1.txt", "FtpClient_libcurl/ftp_download_test_file.txt");

	//Sleep(5 * 1000);
	//tFtpClient.UnInit();
	/************************************************************************/

	/************************************************************************/
	////单线程测试 去除事件队列没有内存泄露
	////加入事件处理后，由于libcurl的进度回调调用太频繁，导致事件内存没有及时释放……

	//MC_FtpClient tFtpClient;
	//tFtpClient.Init();
	//tFtpClient.SetEventCB(DealFtpEvent, 0);
	
	//for(int i=0; i<10000; i++)
	//{
	//	char tBuffer[128] = {0};
	//	sprintf_s(tBuffer, sizeof(tBuffer), "/test/%04d.txt", i);

	//	tFtpClient.FtpUploadFile("127.0.0.1", 21, "kinkoo", "6787.", "../3rdData/test.txt", tBuffer);

	//	if(i == 9999)
	//	{
	//		printf("pause\n");
	//		system("pause");
	//	}
	//}

	//tFtpClient.UnInit();
	/************************************************************************/
	

	/************************************************************************/
	//MC_FtpClient tFtpClient;
	//tFtpClient.Init();
	//tFtpClient.SetEventCB(DealFtpEvent, 0);

	////多线程测试
	//while(1)
	//{
	//	
	//	HANDLE tTread1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadFunc1, &tFtpClient, 0, 0);
	//	HANDLE tTread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadFunc2, &tFtpClient, 0, 0);

	//	Sleep(3000);
	//	TerminateThread(tTread1, 0);
	//	TerminateThread(tTread2, 0);
	//	CloseHandle(tTread1);
	//	CloseHandle(tTread2);
	//}

	//tFtpClient.UnInit();
	/************************************************************************/

	/*MC_KernelWork tKernelWork;
	tKernelWork.OnLoad(1, 3);
	tKernelWork.DoWork();
	tKernelWork.OnExit();*/

	MC_GxxThreadPool tGxxThreadPool;
	tGxxThreadPool.OnLoad(1, 3);
	tGxxThreadPool.DoWork();
	tGxxThreadPool.OnExit();
	

	system("pause");
	return 0; 
} 
//---------------------------------------------------------------------------
