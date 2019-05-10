//---------------------------------------------------------------------------
#ifndef MC_FtpClientH
#define MC_FtpClientH 
//---------------------------------------------------------------------------
#include <WinSock2.h>	//������<Windows.h>��ͻ
#include <string>
using namespace std;
//---------------------------------------------------------------------------
typedef int(*ProgressCallback) (void *clientp, INT64 dltotal, INT64 dlnow, INT64 ultotal, INT64 ulnow);
//---------------------------------------------------------------------------
/*
 * ��֧��һ���������߳�ʹ�ã����߳�ʹ�õĻ���Ҫ����new MC_FtpClient
 * UploadFile: ���� �Դ��� 2M/S������ͨ������һ�ζ�ȡ��buff��С���������ʣ��迼��CPU����
 */
//---------------------------------------------------------------------------
class MC_FtpClient
{ 
public: 
	MC_FtpClient(void); 
	~MC_FtpClient(void); 

	bool			Init				(void);	//��ʼ��winsocket�����������нӿڵ���֮ǰ����
	bool			UnInit				(void);

	bool			Connect				(SOCKET& vCtrlSocket, const char* vIp, u_short vPort, const char* vUser, const char* vPwd, int vTimeOut = 5);	//���ӡ���¼ftp����ʱ�䲻ʹ��FTP�������������Ͽ�����
	bool			DisConnect			(SOCKET vCtrlSocket);																							//����ftp����	

	bool			SetTimeOut			(SOCKET vCtrlSocket, int vTimeOut=5*1000);	//���÷���/���ճ�ʱʱ�䣬Ĭ��5��

	bool			UploadFile			(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath, bool vResume=true, ProgressCallback vCB=NULL);	//�ϴ��ļ���ftp
	bool			DownloadFile		(SOCKET vCtrlSocket, const char* vLoaclFilePath, const char* vRemoteFilePath);

private:
	bool			IsConnect			(SOCKET& vCtrlSocket, const char* vIp, unsigned short vPort, int vTimeOut);
	bool			IsLogin				(SOCKET vCtrlSocket, const char* vUser, const char* vPwd);	

	long			GetRemoteFileSize	(SOCKET vCtrlSocket, const char* vFileName);

public:
	bool			MakeDir				(SOCKET vCtrlSocket, const char* vDir);		//����ftpĿ¼��Ŀ¼·��Ϊftp��Ŀ¼�����·�������ú�ǰĿ¼���ΪvDir��FTP��֧�ִ����༶Ŀ¼��
	bool			MakeDirSingle		(SOCKET vCtrlSocket, const char* vDir);		//MKD	����Ŀ¼���ڵ�ǰ·���´��� 

 	bool			ChangeDir			(SOCKET vCtrlSocket, const char* vDir);		//CWD	����Ŀ¼
 	bool			ChangeUpDir			(SOCKET vCtrlSocket);					//CDUP	�����ϲ�Ŀ¼
 	bool			PrintWorkDir		(SOCKET vCtrlSocket, char* vBuffer);		//PWD	��ʾ��ǰĿ¼  
 	bool			RemoveDir			(SOCKET vCtrlSocket, const char* vDir);		//RMD	ɾ��Ŀ¼ 

	bool			SetPasvMode			(SOCKET vCtrlSocket, SOCKET& vDataSocket);	//PASV	���ñ���ģʽ

private:
	//check
	bool			CheckResult			(SOCKET vCtrlSocket, const char vResult, const char* vCmd, char* vResponse=NULL);	//vCmd: ������Ҫ������ӡ����
	bool			CleanRecvBuffer		(SOCKET vCtrlSocket);
	bool			SendCmd				(SOCKET vCtrlSocket, const char* vCmd);
	char*			trim				(const char* vStr);		//FTP���շ�����\r\n��β����ӡ��ʱ��ȥ��	

	//common
	bool			IsVaildStrArg		(const char* vStr);
	void			SeparateFile		(const char* vPath, char* vFile, char* vDir);
	char*			ChStrReplace		(char* vStr, char* vOldStr, char* vNewStr);

private:
	bool			m_Available;
	//SOCKET			m_CtrlSocket;				//��������socket 
}; 
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------