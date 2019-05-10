//---------------------------------------------------------------------------
#ifndef MC_FtpClientH
#define MC_FtpClientH 
//---------------------------------------------------------------------------
#include <WinSock2.h>	//避免与<Windows.h>冲突
#include <string>
using namespace std;
//---------------------------------------------------------------------------
typedef int(*ProgressCallback) (void *clientp, INT64 dltotal, INT64 dlnow, INT64 ultotal, INT64 ulnow);
//---------------------------------------------------------------------------
/*
 * 不支持一个对象多个线程使用，多线程使用的话需要重新new MC_FtpClient
 * UploadFile: 速率 略大于 2M/S，可以通过设置一次读取的buff大小来控制速率，需考虑CPU性能
 */
//---------------------------------------------------------------------------
class MC_FtpClient
{ 
public: 
	MC_FtpClient(void); 
	~MC_FtpClient(void); 

	bool			Init				(void);	//初始化winsocket，必须在所有接口调用之前调用
	bool			UnInit				(void);

	bool			Connect				(SOCKET& vCtrlSocket, const char* vIp, u_short vPort, const char* vUser, const char* vPwd, int vTimeOut = 5);	//连接、登录ftp，长时间不使用FTP服务器会主动断开连接
	bool			DisConnect			(SOCKET vCtrlSocket);																							//结束ftp连接	

	bool			SetTimeOut			(SOCKET vCtrlSocket, int vTimeOut=5*1000);	//设置发送/接收超时时间，默认5秒

	bool			UploadFile			(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath, bool vResume=true, ProgressCallback vCB=NULL);	//上传文件到ftp
	bool			DownloadFile		(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath);

private:
	bool			IsConnect			(SOCKET& vCtrlSocket, const char* vIp, unsigned short vPort, int vTimeOut);
	bool			IsLogin				(SOCKET vCtrlSocket, const char* vUser, const char* vPwd);	

	long			GetRemoteFileSize	(SOCKET vCtrlSocket, const char* vFileName);

public:
	bool			MakeDir				(SOCKET vCtrlSocket, const char* vDir);		//创建ftp目录，目录路径为ftp根目录的相对路径，调用后当前目录会变为vDir（FTP不支持创建多级目录）
	bool			MakeDirSingle		(SOCKET vCtrlSocket, const char* vDir);		//MKD	创建目录，在当前路径下创建 

 	bool			ChangeDir			(SOCKET vCtrlSocket, const char* vDir);		//CWD	更改目录
 	bool			ChangeUpDir			(SOCKET vCtrlSocket);					//CDUP	返回上层目录
 	bool			PrintWorkDir		(SOCKET vCtrlSocket, char* vBuffer);		//PWD	显示当前目录  
 	bool			RemoveDir			(SOCKET vCtrlSocket, const char* vDir);		//RMD	删除目录 

	bool			SetPasvMode			(SOCKET vCtrlSocket, SOCKET& vDataSocket);	//PASV	设置被动模式

private:
	//check
	bool			CheckResult			(SOCKET vCtrlSocket, const char vResult, const char* vCmd, char* vResponse=NULL);	//vCmd: 传参主要用来打印错误
	bool			CleanRecvBuffer		(SOCKET vCtrlSocket);
	bool			SendCmd				(SOCKET vCtrlSocket, const char* vCmd);
	char*			trim				(const char* vStr);		//FTP的收发都是\r\n结尾，打印的时候去掉	

	//common
	bool			IsVaildStrArg		(const char* vStr);
	void			SeparateFile		(const char* vPath, char* vFile, char* vDir);
	char*			ChStrReplace		(char* vStr, char* vOldStr, char* vNewStr);

private:
	bool			m_Available;
	//SOCKET			m_CtrlSocket;				//命令连接socket 
}; 
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------