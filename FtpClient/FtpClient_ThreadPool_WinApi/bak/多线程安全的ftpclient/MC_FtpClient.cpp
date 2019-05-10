//---------------------------------------------------------------------------
#include "MC_FtpClient.h"
#include "MC_Log.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std; 
//---------------------------------------------------------------------------
#define FTP_CMD_SIZE		256
#define SOCKET_BUFFER_SIZE	2*1024	//2K

#define	SafeCloseSockt(_X_)	{if(INVALID_SOCKET != _X_) {closesocket(_X_); _X_= INVALID_SOCKET;}}
#define	SafeCloseFp(_X_)	{if(NULL != _X_) {fclose(_X_); _X_= NULL;}}
//---------------------------------------------------------------------------
MC_FtpClient::MC_FtpClient(void)
{
	m_Available = false;
	//m_CtrlSocket = INVALID_SOCKET;
}
//---------------------------------------------------------------------------
MC_FtpClient::~MC_FtpClient(void)
{
}
//---------------------------------------------------------------------------
bool MC_FtpClient::Init()
{
	if(true == m_Available)				
		return true;

	//进行版本协商
	WORD	tWVersionRequested	= 0;
	WSADATA	tWsaData			= {0};
	int		tErrorNumber		= 0;
	//winsock版本
	const BYTE tSubVersion	= 2;	//副版本
	const BYTE tMainVersion	= 2;	//主版本

	tWVersionRequested = MAKEWORD(tSubVersion, tMainVersion);
	tErrorNumber = WSAStartup(tWVersionRequested, &tWsaData);	//直接返回错误码
	if(0 != tErrorNumber)
	{
		LOG_ERROR("WSAStartup fail, errno: %d", tErrorNumber);
		return false;
	}

	if (tMainVersion != LOBYTE(tWsaData.wVersion) || tSubVersion != HIBYTE(tWsaData.wVersion))
	{
		LOG_ERROR("winsock 版本号不一致");
		return false;
	}

	m_Available = true;
	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::UnInit()
{
	if(false == m_Available)
		return true;

	//SafeCloseSockt(vCtrlSocket);

	WSACleanup();

	m_Available = false;
	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::Connect(SOCKET& vCtrlSocket, const char* vIp, u_short vPort, const char* vUser, const char* vPwd, int vTimeOut)
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vIp))		return false;
	if(0 >  vPort)						return false;
	if(false == IsVaildStrArg(vUser))	return false;
	if(false == IsVaildStrArg(vPwd))	return false;

	if(false == IsConnect(vCtrlSocket, vIp, vPort, vTimeOut))
	{
		LOG_ERROR("连接FTP服务器失败");
		return false;
	}

	if(false == IsLogin(vCtrlSocket, vUser, vPwd))
	{
		LOG_ERROR("登录FTP服务器失败");
		return false;
	}

	return true; 
} 
//--------------------------------------------------------------------------- 
bool MC_FtpClient::DisConnect(SOCKET vCtrlSocket)
{
	if(false == m_Available)
		return false;

	//从ftp服务器退出
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "QUIT\r\n"); 
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//221 Goodbye
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false;

	SafeCloseSockt(vCtrlSocket);

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::SetTimeOut(SOCKET vCtrlSocket, int vTimeOut)
{
	if(SOCKET_ERROR == setsockopt(vCtrlSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&vTimeOut, sizeof(vTimeOut)))
	{
		LOG_ERROR("set send timeout fail, errno: %d", WSAGetLastError());
		return false;
	}

	if(SOCKET_ERROR == setsockopt(vCtrlSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&vTimeOut, sizeof(vTimeOut)))
	{
		LOG_ERROR("set recv timeout fail, errno: %d", WSAGetLastError());
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::UploadFile(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath, ProgressCallback vCB)
{
	if(false == m_Available)						return false;
	if(false == IsVaildStrArg(vLoaclFilePath))		return false;
	if(false ==	IsVaildStrArg(vRemoteFilePath))		return false;

	FILE*	tLocalFileFp		= NULL;
	if(0 != fopen_s(&tLocalFileFp, vLoaclFilePath, "rb"))
	{
		LOG_ERROR("open file fail, file: %s, errno: %d", vLoaclFilePath, GetLastError());
		return false; 
	}

	fseek(tLocalFileFp, 0, SEEK_END);
	long tLocalFileSize		= ftell(tLocalFileFp);
	long tRemoteFileSize	= GetRemoteFileSize(vCtrlSocket, vRemoteFilePath);
	if(tLocalFileSize == tRemoteFileSize)
	{
		if(NULL != vCB)
		{
			vCB((char*)vLoaclFilePath, 0, 0, tLocalFileSize, tRemoteFileSize);
		}
		return true;
	}

	//获取上传目录和文件名
	char tRemoteFile[MAX_PATH]	= {0};
	char tRemoteDir[MAX_PATH]	= {0};
	SeparateFile(vRemoteFilePath, tRemoteFile, tRemoteDir);

	//判断是否上传到ftp根目录	
	bool tIsRootDir = false;	
	if(0 == strcmp("", tRemoteDir))
		tIsRootDir = true;

	//LOG_INFO("指定FTP上传目录 [%s]", tRemoteDir);

	//如果不是根目录，需要先确认是否需要创建，如果是多级目录，上级目录不存在时，必须先创建上级目录，否则返回失败
	if(false == tIsRootDir)
	{
		if(false == MakeDir(vCtrlSocket, tRemoteDir))
		{
			LOG_ERROR("MakeDir fail, dir: %s", tRemoteDir);
			SafeCloseFp(tLocalFileFp);
			return false;
		}
	}

	//设置ftp被动模式
	SOCKET tDataSocket = SOCKET_ERROR;
	if(false == SetPasvMode(vCtrlSocket, tDataSocket))
	{
		SafeCloseFp(tLocalFileFp);
		return false;
	}

	if(SOCKET_ERROR == tDataSocket)
	{
		SafeCloseFp(tLocalFileFp);
		SafeCloseSockt(tDataSocket);
		return false;
	}

	//远程传输开始
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "APPE %s\r\n", tRemoteFile);
	if(false == SendCmd(vCtrlSocket, tCmd))
	{
		SafeCloseFp(tLocalFileFp);
		SafeCloseSockt(tDataSocket);
		return false;
	}

	//125 Data connection already open; Transfer starting
	if(false == CheckResult(vCtrlSocket, '1', tCmd))
	{
		SafeCloseFp(tLocalFileFp);
		SafeCloseSockt(tDataSocket);
		return false;
	}

	//写入远程文件
	char	tSendBuffer[SOCKET_BUFFER_SIZE]	= {0};	//通过改变buffer的大小可以调节上传速率 
	int		tReadNum			= 0; 
	int		tSendLen			= 0;
	int		tAllSend			= 0;

	fseek(tLocalFileFp, tRemoteFileSize, SEEK_SET);	//从中断的地方开始上传
	tReadNum = fread(tSendBuffer, 1, sizeof(tSendBuffer), tLocalFileFp);
	while(0 < tReadNum)
	{ 
		tSendLen = send(tDataSocket, tSendBuffer, tReadNum, 0); 
		if(SOCKET_ERROR == tSendLen)
		{
			//突然断网的情况，使线程不会卡住
			//传输未完成的文件在ftp服务器上依然会保存，如何处理？？？？ -->重传的话会覆盖，从断点处续传
			LOG_ERROR("send data fail when upload, file: %s, errno: %d", vLoaclFilePath, WSAGetLastError());
			SafeCloseSockt(tDataSocket);
			SafeCloseFp(tLocalFileFp);
			return false;
		}
		
		if(NULL != vCB)
		{
			tAllSend += tSendLen;
			vCB((char*)vLoaclFilePath, 0, 0, tLocalFileSize, tAllSend);
		}

		Sleep(1);	//睡眠用来控制cpu的消耗和上传速率，速率 = 2K/ms
		tReadNum = fread(tSendBuffer, 1, sizeof(tSendBuffer), tLocalFileFp);
	}

	//释放相应资源
	SafeCloseFp(tLocalFileFp);
	SafeCloseSockt(tDataSocket);

	//226 Transfer complete
	if(false == CheckResult(vCtrlSocket, '2', "upload complete"))
		return false;

	//LOG_INFO("文件上传成功：文件名:%s ", tLocalFileFp);

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::DownloadFile(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath)
{
	if(false == m_Available)						return false;
	if(false == IsVaildStrArg(vLoaclFilePath))		return false;
	if(false == IsVaildStrArg(vRemoteFilePath))		return false;

	//bool tResult = false;
	//bool tIsRootDir = false;

	////LOG_INFO("指定下载目录 [%s]", vRemotePath);
	//if (0 == strcmp(vRemotePath, "") || 0 == strcmp(vRemotePath, "/") || 0 == strcmp(vRemotePath, "\\"))
	//	tIsRootDir = true;

	////设置数据类型
	///*  sprintf(m_Cmd,"TYPE A\r\n",vRemoteFile);
	//int tResult = SendCmd(vCtrlSocket, );
	//if(false == tResult)
	//return false;

	//tResult = CheckResult(vCtrlSocket, '2');
	//if(false == tResult)
	//return false;
	//*/ 

	//设置被动模式
	SOCKET tDataSocket = SOCKET_ERROR;
	if(false == SetPasvMode(vCtrlSocket, tDataSocket))
		return false;

	if(SOCKET_ERROR == tDataSocket)
		return false;

	//开始下载
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "RETR %s\r\n", vRemoteFilePath);
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//125 Data connection already open; Transfer starting.226 Transfer complete.
	/*
	550-Access is denied. 
	Win32 error:   Access is denied. 
	Error details: Authorization rules denied the access.
	550 End
	*/

	/*
	550-The system cannot find the file specified. 
	Win32 error:   The system cannot find the file specified. 
	Error details: File system returned an error.
	550 End
	*/
	if(false == CheckResult(vCtrlSocket, '1', tCmd))
		return false;

	FILE*	tLocalFileFp		= NULL;
	char	tRecvBuffer[256]	= {0}; 
	int		tRecvLen			= 0; 
	int		tWriteNum			= 0;

	if(0 != fopen_s(&tLocalFileFp, vLoaclFilePath,"wb"))
	{
		LOG_ERROR("open file fail, file: %s, errno: %d", vLoaclFilePath, GetLastError());
		return false;
	}

	tRecvLen = recv(tDataSocket, tRecvBuffer, 256, 0);
	while(0 < tRecvLen)
	{ 
		tWriteNum = fwrite(tRecvBuffer, tRecvLen, 1, tLocalFileFp); 
		/*if(tLen<0)
		return false; */

		//Sleep(1);
		tRecvLen = recv(tDataSocket,tRecvBuffer,256,0);
	} 

	SafeCloseSockt(tDataSocket);
	SafeCloseFp(tLocalFileFp);

	//226 Transfer complete
	if(false == CheckResult(vCtrlSocket, '2', "download complete"))
		return false;

	return true;
}
//---------------------------------------------------------------------------
/************************************************************************/
/*          privarte function                                           */
/************************************************************************/
//---------------------------------------------------------------------------
bool MC_FtpClient::IsConnect(SOCKET& vCtrlSocket, const char* vIp, unsigned short vPort, int vTimeOut)
{
	if(false == IsVaildStrArg(vIp))		return false;	
	if(0 > vPort)						return false;

	//创建命令连接socket
	vCtrlSocket = socket(AF_INET, SOCK_STREAM, 0);	//必须在此处初始化？？？
	if(INVALID_SOCKET == vCtrlSocket)
	{
		LOG_ERROR("create sockCtrl fail, errno: %d", WSAGetLastError());
		return false;
	}

	//连接超时
	bool	tRetVal = false;
	int		tError	= -1;
	int		tLen	= sizeof(int);

	struct timeval tTimeOut ;
	fd_set tFdSet;
	unsigned long tMode = 1;
	ioctlsocket(vCtrlSocket, FIONBIO, &tMode);   //设置为非阻塞模式

	//填写服务器信息
	SOCKADDR_IN tServerAddr				= {0};
	tServerAddr.sin_addr.S_un.S_addr	= inet_addr(vIp);	//连接地址
	tServerAddr.sin_family				= AF_INET;
	tServerAddr.sin_port				= htons(vPort);		//端口号
	if(SOCKET_ERROR == connect(vCtrlSocket, (sockaddr*)&tServerAddr, sizeof(tServerAddr)))
	{	
		FD_ZERO(&tFdSet);
		FD_SET(vCtrlSocket, &tFdSet);
		tTimeOut.tv_sec = vTimeOut; //连接超时1秒
		tTimeOut.tv_usec =0;
		if(0 < select(0, NULL, &tFdSet, NULL, &tTimeOut))
		{
			 getsockopt(vCtrlSocket, SOL_SOCKET, SO_ERROR, (char*)&tError, &tLen);
			 if(0 == tError)
				 tRetVal = true;
			 else
				 tRetVal = false;
		}
		else
		{
			tRetVal = false;
		}
	}
	else
	{
		tRetVal = true;
	}

	tMode = 0;
	ioctlsocket(vCtrlSocket, FIONBIO, &tMode); //设置为阻塞模式 阻塞时间为tTimeOut时间

	if(false == tRetVal)
	{
		LOG_ERROR("connect sockCtrl fail, maybe timeout");
		return false;
	}

	//220 Microsoft FTP Service
	if(false == CheckResult(vCtrlSocket, '2', "FTP"))
		return false;

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::IsLogin(SOCKET vCtrlSocket, const char* vUser, const char* vPwd)
{
	if(false == IsVaildStrArg(vUser))	return false;	
	if(false == IsVaildStrArg(vPwd))	return false;	

	//用户名验证
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "USER %s\r\n", vUser); 
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false; 

	//331 Password required for [host]
	if(false == CheckResult(vCtrlSocket, '3', tCmd))
		return false; 

	//密码验证
	sprintf_s(tCmd, sizeof(tCmd), "PASS %s\r\n", vPwd); 
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//230 User logged in
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false; 

	return true; 
}
//---------------------------------------------------------------------------
long MC_FtpClient::GetRemoteFileSize(SOCKET vCtrlSocket, const char* vFileName)
{
	if(false == IsVaildStrArg(vFileName))		
		return 0;

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "SIZE %s\r\n", vFileName);
	if(false == SendCmd(vCtrlSocket, tCmd))
		return 0;

	//213 字节数，如213 20552
	/*
	550-The system cannot find the file specified. 
	Win32 error:   The system cannot find the file specified. 
	Error details: File system returned an error.
	550 End
	*/
	char tRes[1024] = {0};
	if(false == CheckResult(vCtrlSocket, '2', tCmd, tRes))
		return 0;

	const char* tRemoteFileSize = strrchr(tRes, ' ') + 1;
	
	return atol(tRemoteFileSize);
}
//---------------------------------------------------------------------------
bool MC_FtpClient::MakeDir(SOCKET vCtrlSocket, const char* vDir) 
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//简单起见，每次都从根目录开始，逐个创建
	if(false == ChangeDir(vCtrlSocket, "/"))
		return false;

	//切割路径
	char	tPath[MAX_PATH]	= {0};
	int		tPos			= 0;
	char	tDir[MAX_PATH]	= {0};
	int		tLen			= 0;

	sprintf_s(tDir, sizeof(tDir), "%s/", vDir);
	tLen = strlen(tDir);

	for(int i=0; i<tLen; i++)
	{
		tPath[tPos] = tDir[i];
		if('/' != tPath[tPos])
		{
			tPos++;
			continue;
		}

		tPath[tPos] = 0;	//遇到路径分隔符，则截取路径
		if(0 != strcmp("", tPath))	//预防是这样的参数：/dir,截取到空字符串
		{
			//LOG_INFO("create :%s", tPath); 
			if(false == MakeDirSingle(vCtrlSocket, tPath))
				return false;

			if(false == ChangeDir(vCtrlSocket, tPath))
				return false;
		}
		tPos = 0;
	}

	return true; 
} 
//---------------------------------------------------------------------------
bool MC_FtpClient::MakeDirSingle(SOCKET vCtrlSocket, const char* vDir) 
{ 
	if(false == IsVaildStrArg(vDir))	return false;

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "MKD %s\r\n", vDir);
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//257 [dir] directory created
	//550-Cannot create a file when that file already exists. 
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false;

	return true; 
} 
//---------------------------------------------------------------------------
bool MC_FtpClient::ChangeDir(SOCKET vCtrlSocket, const char* vDir)
{
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//改变ftp服务器上的工作目录
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "CWD %s\r\n", vDir);
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//250 Cd command: successful.
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false; 

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::ChangeUpDir(SOCKET vCtrlSocket)
{
	if(false == m_Available)		return false;

	//返回上一层目录
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "CDUP\r\n");
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false; 

	return true; 
}
//---------------------------------------------------------------------------
bool MC_FtpClient::PrintWorkDir(SOCKET vCtrlSocket, char* vBuffer)
{
	if(false == m_Available)		return false;
	if(0 == vBuffer)				return false;

	//显示当前工作目录
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "PWD\r\n");
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//257 "" is current directory
	char tResponse[FTP_CMD_SIZE] = {0};
	if(false == CheckResult(vCtrlSocket, '2', tCmd, tResponse))
		return false;

	char* temp = tResponse; 
	while(0 != *temp) 
	{ 
		//eg:
		if('"' == *temp++) 
		{
			while('"' != *temp) 
				*vBuffer++ = *temp++; 

			break;	//读完目录必须退出循环
		}
	} 
	*vBuffer = 0; 

	return true; 
}
//---------------------------------------------------------------------------
bool MC_FtpClient::RemoveDir(SOCKET vCtrlSocket, const char* vDir) 
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//删除ftp服务器上的指定目录
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "RMD %s\r\n", vDir);
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//250 XRMD command successful
	if(false == CheckResult(vCtrlSocket, '2', tCmd))
		return false; 

	return true; 
} 
//--------------------------------------------------------------------------- 
bool MC_FtpClient::SetPasvMode(SOCKET vCtrlSocket, SOCKET& vDataSocket)
{
	/*if(false == m_Available)
		return false;*/

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "PASV\r\n");
	if(false == SendCmd(vCtrlSocket, tCmd))
		return false;

	//227 Entering Passive Mode (10,10,17,153,43,85)
	//(<IP>, N1, N2) 临时端口号（大于1024）N1*256+N2
	char tRespose[FTP_CMD_SIZE] = {0};
	if(false == CheckResult(vCtrlSocket, '2', tCmd, tRespose))
		return false;
	
	//创建数据连接socket
	vDataSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if(INVALID_SOCKET  == vDataSocket)
	{
		LOG_ERROR("create SockData fail, error: %d", WSAGetLastError());
		return false;
	}

	unsigned int v[6]; 
	union 
	{ 
		struct sockaddr sa; 
		struct sockaddr_in in; 
	} sin; 

	sscanf_s(tRespose, "%*[^(](%u, %u, %u, %u, %u, %u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]); 

	//192.168.51.19 .236.187
	//LOG_INFO("%d.%d.%d.%d.%d.%d", v[2], v[3], v[4], v[5], v[0], v[1]);

	sin.sa.sa_family = AF_INET; 
	sin.sa.sa_data[2] = v[2]; 
	sin.sa.sa_data[3] = v[3]; 
	sin.sa.sa_data[4] = v[4]; 
	sin.sa.sa_data[5] = v[5]; 
	sin.sa.sa_data[0] = v[0]; 
	sin.sa.sa_data[1] = v[1]; 

	//允许套接口和一个已在使用中的地址捆绑
	int on = 1; 
	if (-1 == setsockopt(vDataSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on))) 
	{ 
		LOG_ERROR("setsockopt[1] fail, errno: %d", WSAGetLastError());
		return false; 
	} 

	//强制close，不逗留
	struct linger tLinger = {0, 0}; 
	if (-1 == setsockopt(vDataSocket, SOL_SOCKET, SO_LINGER, (const char*) &tLinger, sizeof(tLinger))) 
	{ 
		LOG_ERROR("setsockopt[2] fail, errno: %d", WSAGetLastError());
		return false; 
	} 

	int tConnectsocket = connect(vDataSocket, (sockaddr*)&sin, sizeof(sin)); 
	if(0 != tConnectsocket)
	{
		LOG_ERROR("connect SockData fail, errno: %d", WSAGetLastError());
		return false; 
	}

	return true; 
}
//---------------------------------------------------------------------------
/************************************************************************/
/*          私有函数                                                    */
/************************************************************************/
//---------------------------------------------------------------------------
bool MC_FtpClient::CheckResult(SOCKET vCtrlSocket, const char vResult, const char* vCmd, char* vResponse)
{
	if(0 == vResult)					return false;
	if(false == IsVaildStrArg(vCmd))	return false;

	Sleep(5);	//IIS-ftp在遇到错误时，可能还会再发一个包Win32 error。。。。，如果不等待的话可能会影响下次接收

	char tResponse[FTP_CMD_SIZE] = {0};
	int tLen = recv(vCtrlSocket, tResponse, sizeof(tResponse), 0);
	if(SOCKET_ERROR == tLen)
	{
		LOG_ERROR("recv[1] response fail, cmd: %s, errno: %d", trim(vCmd), WSAGetLastError());
		return false;
	}
	tResponse[tLen] = 0;	//最后一位置为结束符 	

	if(vResult != tResponse[0])
	{
		//创建已存在的目录，默认为创建成功
		if(NULL != strstr(vCmd, "MKD") && NULL != strstr(tResponse, "already exists"))
			return true;

		if(NULL != strstr(vCmd, "SIZE") && NULL != strstr(tResponse, "cannot find the file"))	//不输出日志
			return false;

		LOG_ERROR("undesired response, cmd: %s, response: %s", trim(vCmd), trim(tResponse));
		return false;
	}

	//LOG_INFO(desired response, cmd: %s, response: %s", trim(vCmd), trim(tResponse));
	if(NULL != vResponse)
	{
		strcpy_s(vResponse, FTP_CMD_SIZE, tResponse);
	}

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::CleanRecvBuffer(SOCKET vCtrlSocket)
{
	// 设置select立即返回
	timeval tTimeOut;
	tTimeOut.tv_sec		= 0;
	tTimeOut.tv_usec	= 0;

	// 设置select对vCtrlSocket的读取感兴趣
	fd_set tReadFds;
	FD_ZERO(&tReadFds);
	FD_SET(vCtrlSocket, &tReadFds);

	int tRes = -1;
	char tRecvData[2];
	memset(tRecvData, 0, sizeof(tRecvData));
	while(true)
	{
		tRes = select(FD_SETSIZE, &tReadFds, NULL, NULL, &tTimeOut);
		if (0 == tRes) 
			break;  //数据读取完毕，缓存区清空成功

		recv(vCtrlSocket, tRecvData, 1, 0);  //触发数据读取
	}

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::SendCmd(SOCKET vCtrlSocket, const char* vCmd)
{
	if(false == IsVaildStrArg(vCmd))	return false;

	int tRet = send(vCtrlSocket, vCmd, strlen(vCmd), 0); 
	if(SOCKET_ERROR == tRet)
	{
		LOG_ERROR("send cmd fail, cmd: %s, errno: %d", trim(vCmd), WSAGetLastError());
		return false; 
	}

	return true; 
}
//---------------------------------------------------------------------------
char* MC_FtpClient::trim(const char* vString)
{
	//FTP命令和回复都有换行，将换行去掉
	std::string tStr = std::string(vString);
	tStr = tStr.substr(0, tStr.rfind("\r"));

	int tLen = tStr.size()+1;
	char* tTemp = new char[tLen];
	strcpy_s(tTemp, tLen, tStr.c_str());

	return tTemp;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::IsVaildStrArg(const char* vStr)
{
	if(0 == vStr)	return false;
	if(0 == *vStr)	return false;

	return true;
}
//---------------------------------------------------------------------------
void MC_FtpClient::SeparateFile(const char* vPath, char* vFile, char* vDir)
{
	//将所有的\\转换为/
	char tPath[MAX_PATH] = {0};
	strcpy_s(tPath, strlen(vPath)+1, vPath);
	ChStrReplace(tPath, "\\", "/");

	strcpy_s(vFile, MAX_PATH, tPath);	//先转为char*，必须
	char* tPos = strrchr(vFile, '/');
	if(0 != tPos)
	{
		*tPos = 0;
		strcpy_s(vDir,	MAX_PATH, vFile);
		strcpy_s(vFile, MAX_PATH, tPos+1);
	}
}
//---------------------------------------------------------------------------
char* MC_FtpClient::ChStrReplace(char* vStr, char* vOldStr, char* vNewStr)
{
	char tStr[MAX_PATH];
	memset(tStr, 0, sizeof(tStr));

	for(int i=0; i<strlen(vStr); i++)
	{
		if(0 == strncmp(vStr+i, vOldStr, strlen(vOldStr)))	//查找目标字符串
		{
			strcat(tStr, vNewStr);
			i += strlen(vOldStr) -1;
		}
		else
		{
			strncat(tStr, vStr + i, 1);	//保存一字节进缓冲区
		}
	}

	strcpy_s(vStr, sizeof(tStr), tStr);
	return vStr;
}
