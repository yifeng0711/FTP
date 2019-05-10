//---------------------------------------------------------------------------
#include "MC_FtpClient.h" 
#include <iostream>
#include "MC_Log.h"
#include <time.h>
//---------------------------------------------------------------------------
using namespace std; 
//---------------------------------------------------------------------------
int main(int argc, char* argv[]) 
{ 
	bool	tChangeId	= false;
	bool	tResult		= false;

	MC_FtpClient	tFtpClient; 
	tFtpClient.Init();

	int i = 0;
	do 
	{
		i++;
		char tBuffer[128] = {0};
		sprintf_s(tBuffer, sizeof(tBuffer), "/FtpClient_WinSock/%04d.txt", i);

		std::string	tIp		= "";
		std::string	tUser	= "";
		std::string	tPwd	= "";
		if(false == tChangeId)
		{
			tIp		= "192.168.51.19";
			tUser	= "kinkoo";
			tPwd	= "6787.";
			//tChangeId = true;
		}
		else
		{
			tIp			= "192.168.184.133";
			tUser		= "gmvcsws";
			tPwd		= "1";	
			tChangeId = false;
		}

		tResult = tFtpClient.Connect(tIp.c_str(), 21, tUser.c_str(), tPwd.c_str()); 
		if(false == tResult)
		{ 
			printf("Connect fail, ErrorNum: %d\n", GetLastError());	
			break; 
		} 
		printf("Connect ok\n"); 

		clock_t tStart = clock();
		//if(false == tFtpClient.UploadFile("../3rdData/ftp_upload_test_file.txt", tBuffer))
		tFtpClient.UploadFile("../3rdData/ftp_upload_test_file.txt", "/FtpClient_WinSock/1.txt");
		tFtpClient.UploadFile("../3rdData/ftp_upload_test_file.txt", "/FtpClient_WinSock/3.txt");
		if(false == tFtpClient.UploadFile("../3rdData/ftp_upload_test_file.txt", "/FtpClient_WinSock/2.txt"))
		{ 
			printf("UpLoad fail, ErrorNum: %d\n", GetLastError()); 
			break;
		}
		clock_t tEnd = clock();
		printf("UpLoad ok, time: %d\n", tEnd - tStart);

		tResult = tFtpClient.DisConnect(); 
		if(false == tResult)
		{ 
			printf("Quit fail, ErrorNum: %d\n", GetLastError());
			break;
		} 
		printf("You quit\n"); 
		

		break;
		Sleep(1000);
	} while (1);

	tFtpClient.UnInit();

	system("pause");
	return 0; 
} 
//---------------------------------------------------------------------------