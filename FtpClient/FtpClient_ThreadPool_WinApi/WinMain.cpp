#include "WinMain.h"
#include "MC_KernelWork.h"

#include <time.h>
#include <iostream>
using namespace std;
//---------------------------------------------------------------------------
int main()
{
	////单线程测试 内存泄漏 貌似没问题了
	//MC_FtpClient tFtpClient;
	//tFtpClient.Init();
	//for(int i=0; i<10000; i++)
	//{
	//	char tBuffer[128] = {0};
	//	sprintf_s(tBuffer, sizeof(tBuffer), "/test/%04d.txt", i);

	//	SOCKET tCtrlSocket = 0;
	//	tFtpClient.Connect(tCtrlSocket, "127.0.0.1", 21, "kinkoo", "6787.");
	//
	//	tFtpClient.UploadFile(tCtrlSocket, "../3rdData/test.txt", tBuffer);
	//
	//	tFtpClient.DisConnect(tCtrlSocket);	

	//	if(i == 9999)
	//	{
	//		printf("pause\n");
	//		system("pause");
	//	}
	//}

	MC_KernelWork tKernelWork;
	tKernelWork.OnLoad(1, 3);

	tKernelWork.DoWork();

	tKernelWork.OnExit();

	printf("\n ----------end---------- ");
	fflush(stdin);
	getchar();
	return 0;
}
//---------------------------------------------------------------------------
