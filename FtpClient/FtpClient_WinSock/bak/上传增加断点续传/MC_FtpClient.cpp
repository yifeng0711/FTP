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
	m_SockCtrl = INVALID_SOCKET;
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

	//���а汾Э��
	WORD	tWVersionRequested	= 0;
	WSADATA	tWsaData			= {0};
	int		tErrorNumber		= 0;
	//winsock�汾
	const BYTE tSubVersion	= 2;	//���汾
	const BYTE tMainVersion	= 2;	//���汾

	tWVersionRequested = MAKEWORD(tSubVersion, tMainVersion);
	tErrorNumber = WSAStartup(tWVersionRequested, &tWsaData);	//ֱ�ӷ��ش�����
	if(0 != tErrorNumber)
	{
		LOG_ERROR("WSAStartup fail, errno: %d", tErrorNumber);
		return false;
	}

	if (tMainVersion != LOBYTE(tWsaData.wVersion) || tSubVersion != HIBYTE(tWsaData.wVersion))
	{
		LOG_ERROR("winsock �汾�Ų�һ��");
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

	SafeCloseSockt(m_SockCtrl);

	WSACleanup();

	m_Available = false;
	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::Connect(const char* vIp, u_short vPort, const char* vUser, const char* vPwd, int vTimeOut)
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vIp))		return false;
	if(0 >  vPort)						return false;
	if(false == IsVaildStrArg(vUser))	return false;
	if(false == IsVaildStrArg(vPwd))	return false;

	if(false == IsConnect(vIp, vPort, vTimeOut))
	{
		LOG_ERROR("����FTP������ʧ��");
		return false;
	}

	if(false == IsLogin(vUser, vPwd))
	{
		LOG_ERROR("��¼FTP������ʧ��");
		return false;
	}

	return true; 
} 
//--------------------------------------------------------------------------- 
bool MC_FtpClient::DisConnect()
{
	if(false == m_Available)
		return false;

	//��ftp�������˳�
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "QUIT\r\n"); 
	if(false == SendCmd(tCmd))
		return false;

	//221 Goodbye
	if(false == CheckResult('2', tCmd))
		return false;

	SafeCloseSockt(m_SockCtrl);

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::SetTimeOut(int vTimeOut)
{
	if(SOCKET_ERROR == setsockopt(m_SockCtrl, SOL_SOCKET, SO_SNDTIMEO, (char*)&vTimeOut, sizeof(vTimeOut)))
	{
		LOG_ERROR("set send timeout fail, errno: %d", WSAGetLastError());
		return false;
	}

	if(SOCKET_ERROR == setsockopt(m_SockCtrl, SOL_SOCKET, SO_RCVTIMEO, (char *)&vTimeOut, sizeof(vTimeOut)))
	{
		LOG_ERROR("set recv timeout fail, errno: %d", WSAGetLastError());
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::UploadFile(const char* vLoaclFilePath, const char* vRemoteFilePath)
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
	long tRemoteFileSize	= GetRemoteFileSize(vRemoteFilePath);
	if(tLocalFileSize == tRemoteFileSize)
		return true;

	//��ȡ�ϴ�Ŀ¼���ļ���
	char tRemoteFile[MAX_PATH]	= {0};
	char tRemoteDir[MAX_PATH]	= {0};
	SeparateFile(vRemoteFilePath, tRemoteFile, tRemoteDir);

	//�ж��Ƿ��ϴ���ftp��Ŀ¼	
	bool tIsRootDir = false;	
	if(0 == strcmp("", tRemoteDir))
		tIsRootDir = true;

	//LOG_INFO("ָ��FTP�ϴ�Ŀ¼ [%s]", tRemoteDir);

	//������Ǹ�Ŀ¼����Ҫ��ȷ���Ƿ���Ҫ����������Ƕ༶Ŀ¼���ϼ�Ŀ¼������ʱ�������ȴ����ϼ�Ŀ¼�����򷵻�ʧ��
	if(false == tIsRootDir)
	{
		if(false == MakeDir(tRemoteDir))
		{
			LOG_ERROR("MakeDir fail, dir: %s", tRemoteDir);
			SafeCloseFp(tLocalFileFp);
			return false;
		}
	}

	//����ftp����ģʽ
	SOCKET tDataSocket = SOCKET_ERROR;
	if(false == SetPasvMode(tDataSocket))
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

	//Զ�̴��俪ʼ
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "APPE %s\r\n", tRemoteFile);
	if(false == SendCmd(tCmd))
	{
		SafeCloseFp(tLocalFileFp);
		SafeCloseSockt(tDataSocket);
		return false;
	}

	//125 Data connection already open; Transfer starting
	if(false == CheckResult('1', tCmd))
	{
		SafeCloseFp(tLocalFileFp);
		SafeCloseSockt(tDataSocket);
		return false;
	}

	//д��Զ���ļ�
	char	tSendBuffer[SOCKET_BUFFER_SIZE]	= {0};	//ͨ���ı�buffer�Ĵ�С���Ե����ϴ����� 
	int		tReadNum			= 0; 
	int		tSendLen			= 0;

	fseek(tLocalFileFp, tRemoteFileSize, SEEK_SET);	//���жϵĵط���ʼ�ϴ�
	tReadNum = fread(tSendBuffer, 1, sizeof(tSendBuffer), tLocalFileFp);
	while(0 < tReadNum)
	{ 
		tSendLen = send(tDataSocket, tSendBuffer, tReadNum, 0); 
		if(SOCKET_ERROR == tSendLen)
		{
			//ͻȻ�����������ʹ�̲߳��Ῠס
			//����δ��ɵ��ļ���ftp����������Ȼ�ᱣ�棬��δ��������� -->�ش��Ļ��Ḳ�ǣ��Ӷϵ㴦����
			LOG_ERROR("send data fail when upload, file: %s, errno: %d", vLoaclFilePath, WSAGetLastError());
			SafeCloseSockt(tDataSocket);
			SafeCloseFp(tLocalFileFp);
			return false;
		}

		Sleep(1);	//˯����������cpu�����ĺ��ϴ����ʣ����� = 2K/ms
		tReadNum = fread(tSendBuffer, 1, sizeof(tSendBuffer), tLocalFileFp);
	}

	//�ͷ���Ӧ��Դ
	SafeCloseFp(tLocalFileFp);
	SafeCloseSockt(tDataSocket);

	//226 Transfer complete
	if(false == CheckResult('2', "upload complete"))
		return false;

	//LOG_INFO("�ļ��ϴ��ɹ����ļ���:%s ", tLocalFileFp);

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::DownloadFile(const char* vLoaclFilePath, const char* vRemoteFilePath)
{
	if(false == m_Available)						return false;
	if(false == IsVaildStrArg(vLoaclFilePath))		return false;
	if(false == IsVaildStrArg(vRemoteFilePath))		return false;

	//bool tResult = false;
	//bool tIsRootDir = false;

	////LOG_INFO("ָ������Ŀ¼ [%s]", vRemotePath);
	//if (0 == strcmp(vRemotePath, "") || 0 == strcmp(vRemotePath, "/") || 0 == strcmp(vRemotePath, "\\"))
	//	tIsRootDir = true;

	////������������
	///*  sprintf(m_Cmd,"TYPE A\r\n",vRemoteFile);
	//int tResult = SendCmd();
	//if(false == tResult)
	//return false;

	//tResult = CheckResult('2');
	//if(false == tResult)
	//return false;
	//*/ 

	//���ñ���ģʽ
	SOCKET tDataSocket = SOCKET_ERROR;
	if(false == SetPasvMode(tDataSocket))
		return false;

	if(SOCKET_ERROR == tDataSocket)
		return false;

	//��ʼ����
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "RETR %s\r\n", vRemoteFilePath);
	if(false == SendCmd(tCmd))
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
	if(false == CheckResult('1', tCmd))
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
	if(false == CheckResult('2', "download complete"))
		return false;

	return true;
}
//---------------------------------------------------------------------------
/************************************************************************/
/*          privarte function                                           */
/************************************************************************/
//---------------------------------------------------------------------------
bool MC_FtpClient::IsConnect(const char* vIp, unsigned short vPort, int vTimeOut)
{
	if(false == IsVaildStrArg(vIp))		return false;	
	if(0 > vPort)						return false;

	//������������socket
	m_SockCtrl = socket(AF_INET, SOCK_STREAM, 0);	//�����ڴ˴���ʼ��������
	if(INVALID_SOCKET == m_SockCtrl)
	{
		LOG_ERROR("create sockCtrl fail, errno: %d", WSAGetLastError());
		return false;
	}

	//���ӳ�ʱ
	bool	tRetVal = false;
	int		tError	= -1;
	int		tLen	= sizeof(int);

	struct timeval tTimeOut ;
	fd_set tFdSet;
	unsigned long tMode = 1;
	ioctlsocket(m_SockCtrl, FIONBIO, &tMode);   //����Ϊ������ģʽ

	//��д��������Ϣ
	SOCKADDR_IN tServerAddr				= {0};
	tServerAddr.sin_addr.S_un.S_addr	= inet_addr(vIp);	//���ӵ�ַ
	tServerAddr.sin_family				= AF_INET;
	tServerAddr.sin_port				= htons(vPort);		//�˿ں�
	if(SOCKET_ERROR == connect(m_SockCtrl, (sockaddr*)&tServerAddr, sizeof(tServerAddr)))
	{	
		FD_ZERO(&tFdSet);
		FD_SET(m_SockCtrl, &tFdSet);
		tTimeOut.tv_sec = vTimeOut; //���ӳ�ʱ1��
		tTimeOut.tv_usec =0;
		if(0 < select(0, NULL, &tFdSet, NULL, &tTimeOut))
		{
			 getsockopt(m_SockCtrl, SOL_SOCKET, SO_ERROR, (char*)&tError, &tLen);
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
	ioctlsocket(m_SockCtrl, FIONBIO, &tMode); //����Ϊ����ģʽ ����ʱ��ΪtTimeOutʱ��

	if(false == tRetVal)
	{
		LOG_ERROR("connect sockCtrl fail, maybe timeout");
		return false;
	}

	//220 Microsoft FTP Service
	if(false == CheckResult('2', "FTP"))
		return false;

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::IsLogin(const char* vUser, const char* vPwd)
{
	if(false == IsVaildStrArg(vUser))	return false;	
	if(false == IsVaildStrArg(vPwd))	return false;	

	//�û�����֤
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "USER %s\r\n", vUser); 
	if(false == SendCmd(tCmd))
		return false; 

	//331 Password required for [host]
	if(false == CheckResult('3', tCmd))
		return false; 

	//������֤
	sprintf_s(tCmd, sizeof(tCmd), "PASS %s\r\n", vPwd); 
	if(false == SendCmd(tCmd))
		return false;

	//230 User logged in
	if(false == CheckResult('2', tCmd))
		return false; 

	return true; 
}
//---------------------------------------------------------------------------
long MC_FtpClient::GetRemoteFileSize(const char* vFileName)
{
	if(false == IsVaildStrArg(vFileName))		
		return 0;

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "SIZE %s\r\n", vFileName);
	if(false == SendCmd(tCmd))
		return 0;

	//213 �ֽ�������213 20552
	/*
	550-The system cannot find the file specified. 
	Win32 error:   The system cannot find the file specified. 
	Error details: File system returned an error.
	550 End
	*/
	char tRes[1024] = {0};
	if(false == CheckResult('2', tCmd, tRes))
		return -1;

	const char* tRemoteFileSize = strrchr(tRes, ' ') + 1;
	
	return atol(tRemoteFileSize);
}
//---------------------------------------------------------------------------
bool MC_FtpClient::MakeDir(const char* vDir) 
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//�������ÿ�ζ��Ӹ�Ŀ¼��ʼ���������
	if(false == ChangeDir("/"))
		return false;

	//�и�·��
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

		tPath[tPos] = 0;	//����·���ָ��������ȡ·��
		if(0 != strcmp("", tPath))	//Ԥ���������Ĳ�����/dir,��ȡ�����ַ���
		{
			//LOG_INFO("create :%s", tPath); 
			if(false == MakeDirSingle(tPath))
				return false;

			if(false == ChangeDir(tPath))
				return false;
		}
		tPos = 0;
	}

	return true; 
} 
//---------------------------------------------------------------------------
bool MC_FtpClient::MakeDirSingle(const char* vDir) 
{ 
	if(false == IsVaildStrArg(vDir))	return false;

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "MKD %s\r\n", vDir);
	if(false == SendCmd(tCmd))
		return false;

	//257 [dir] directory created
	//550-Cannot create a file when that file already exists. 
	if(false == CheckResult('2', tCmd))
		return false;

	return true; 
} 
//---------------------------------------------------------------------------
bool MC_FtpClient::ChangeDir(const char* vDir)
{
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//�ı�ftp�������ϵĹ���Ŀ¼
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "CWD %s\r\n", vDir);
	if(false == SendCmd(tCmd))
		return false;

	//250 Cd command: successful.
	if(false == CheckResult('2', tCmd))
		return false; 

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::ChangeUpDir()
{
	if(false == m_Available)		return false;

	//������һ��Ŀ¼
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "CDUP\r\n");
	if(false == SendCmd(tCmd))
		return false;

	//
	if(false == CheckResult('2', tCmd))
		return false; 

	return true; 
}
//---------------------------------------------------------------------------
bool MC_FtpClient::PrintWorkDir(char* vBuffer)
{
	if(false == m_Available)		return false;
	if(0 == vBuffer)				return false;

	//��ʾ��ǰ����Ŀ¼
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "PWD\r\n");
	if(false == SendCmd(tCmd))
		return false;

	//257 "" is current directory
	char tResponse[FTP_CMD_SIZE] = {0};
	if(false == CheckResult('2', tCmd, tResponse))
		return false;

	char* temp = tResponse; 
	while(0 != *temp) 
	{ 
		//eg:
		if('"' == *temp++) 
		{
			while('"' != *temp) 
				*vBuffer++ = *temp++; 

			break;	//����Ŀ¼�����˳�ѭ��
		}
	} 
	*vBuffer = 0; 

	return true; 
}
//---------------------------------------------------------------------------
bool MC_FtpClient::RemoveDir(const char* vDir) 
{ 
	if(false == m_Available)			return false;
	if(false == IsVaildStrArg(vDir))	return false;

	//ɾ��ftp�������ϵ�ָ��Ŀ¼
	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "RMD %s\r\n", vDir);
	if(false == SendCmd(tCmd))
		return false;

	//250 XRMD command successful
	if(false == CheckResult('2', tCmd))
		return false; 

	return true; 
} 
//--------------------------------------------------------------------------- 
bool MC_FtpClient::SetPasvMode(SOCKET& vDataSocket)
{
	/*if(false == m_Available)
		return false;*/

	char tCmd[FTP_CMD_SIZE] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "PASV\r\n");
	if(false == SendCmd(tCmd))
		return false;

	//227 Entering Passive Mode (10,10,17,153,43,85)
	//(<IP>, N1, N2) ��ʱ�˿ںţ�����1024��N1*256+N2
	char tRespose[FTP_CMD_SIZE] = {0};
	if(false == CheckResult('2', tCmd, tRespose))
		return false;
	
	//������������socket
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

	//�����׽ӿں�һ������ʹ���еĵ�ַ����
	int on = 1; 
	if (-1 == setsockopt(vDataSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on))) 
	{ 
		LOG_ERROR("setsockopt[1] fail, errno: %d", WSAGetLastError());
		return false; 
	} 

	//ǿ��close��������
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
/*          ˽�к���                                                    */
/************************************************************************/
//---------------------------------------------------------------------------
bool MC_FtpClient::CheckResult(const char vResult, const char* vCmd, char* vResponse)
{
	if(0 == vResult)					return false;
	if(false == IsVaildStrArg(vCmd))	return false;

	Sleep(5);	//IIS-ftp����������ʱ�����ܻ����ٷ�һ����Win32 error����������������ȴ��Ļ����ܻ�Ӱ���´ν���

	char tResponse[FTP_CMD_SIZE] = {0};
	int tLen = recv(m_SockCtrl, tResponse, sizeof(tResponse), 0);
	if(SOCKET_ERROR == tLen)
	{
		LOG_ERROR("recv[1] response fail, cmd: %s, errno: %d", trim(vCmd), WSAGetLastError());
		return false;
	}
	tResponse[tLen] = 0;	//���һλ��Ϊ������ 	

	if(vResult != tResponse[0])
	{
		//�����Ѵ��ڵ�Ŀ¼��Ĭ��Ϊ�����ɹ�
		if(NULL != strstr(vCmd, "MKD") && NULL != strstr(tResponse, "already exists"))
			return true;

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
bool MC_FtpClient::CleanRecvBuffer()
{
	// ����select��������
	timeval tTimeOut;
	tTimeOut.tv_sec		= 0;
	tTimeOut.tv_usec	= 0;

	// ����select��m_SockCtrl�Ķ�ȡ����Ȥ
	fd_set tReadFds;
	FD_ZERO(&tReadFds);
	FD_SET(m_SockCtrl, &tReadFds);

	int tRes = -1;
	char tRecvData[2];
	memset(tRecvData, 0, sizeof(tRecvData));
	while(true)
	{
		tRes = select(FD_SETSIZE, &tReadFds, NULL, NULL, &tTimeOut);
		if (0 == tRes) 
			break;  //���ݶ�ȡ��ϣ���������ճɹ�

		recv(m_SockCtrl, tRecvData, 1, 0);  //�������ݶ�ȡ
	}

	return true;
}
//---------------------------------------------------------------------------
bool MC_FtpClient::SendCmd(const char* vCmd)
{
	if(false == IsVaildStrArg(vCmd))	return false;

	int tRet = send(m_SockCtrl, vCmd, strlen(vCmd), 0); 
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
	//FTP����ͻظ����л��У�������ȥ��
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
	//�����е�\\ת��Ϊ/
	char tPath[MAX_PATH] = {0};
	strcpy_s(tPath, strlen(vPath)+1, vPath);
	ChStrReplace(tPath, "\\", "/");

	strcpy_s(vFile, MAX_PATH, tPath);	//��תΪchar*������
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
		if(0 == strncmp(vStr+i, vOldStr, strlen(vOldStr)))	//����Ŀ���ַ���
		{
			strcat(tStr, vNewStr);
			i += strlen(vOldStr) -1;
		}
		else
		{
			strncat(tStr, vStr + i, 1);	//����һ�ֽڽ�������
		}
	}

	strcpy_s(vStr, sizeof(tStr), tStr);
	return vStr;
}
