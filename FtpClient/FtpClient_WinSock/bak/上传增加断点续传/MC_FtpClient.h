//---------------------------------------------------------------------------
#ifndef MC_FtpClientH
#define MC_FtpClientH 
//---------------------------------------------------------------------------
#include <WinSock2.h>	//������<Windows.h>��ͻ
#include <string>
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

	bool			Connect				(const char* vIp, u_short vPort, const char* vUser, const char* vPwd, int vTimeOut = 5);	//���ӡ���¼ftp����ʱ�䲻ʹ��FTP�������������Ͽ�����
	bool			DisConnect			();																							//����ftp����	

	bool			SetTimeOut			(int vTimeOut=5*1000);	//���÷���/���ճ�ʱʱ�䣬Ĭ��5��

	bool			UploadFile			(const char* vLoaclFilePath, const char* vRemoteFilePath);	//�ϴ��ļ���ftp
	bool			DownloadFile		(const char* vLoaclFilePath, const char* vRemoteFilePath);

private:
	bool			IsConnect			(const char* vIp, unsigned short vPort, int vTimeOut);
	bool			IsLogin				(const char* vUser, const char* vPwd);	

	long			GetRemoteFileSize	(const char* vFileName);

public:
	bool			MakeDir				(const char* vDir);		//����ftpĿ¼��Ŀ¼·��Ϊftp��Ŀ¼�����·�������ú�ǰĿ¼���ΪvDir��FTP��֧�ִ����༶Ŀ¼��
	bool			MakeDirSingle		(const char* vDir);		//MKD	����Ŀ¼���ڵ�ǰ·���´��� 

 	bool			ChangeDir			(const char* vDir);		//CWD	����Ŀ¼
 	bool			ChangeUpDir			(void);					//CDUP	�����ϲ�Ŀ¼
 	bool			PrintWorkDir		(char* vBuffer);		//PWD	��ʾ��ǰĿ¼  
 	bool			RemoveDir			(const char* vDir);		//RMD	ɾ��Ŀ¼ 

	bool			SetPasvMode			(SOCKET& vDataSocket);	//PASV	���ñ���ģʽ

private:
	//check
	bool			CheckResult			(const char vResult, const char* vCmd, char* vResponse=NULL);	//vCmd: ������Ҫ������ӡ����
	bool			CleanRecvBuffer		();
	bool			SendCmd				(const char* vCmd);
	char*			trim				(const char* vStr);		//FTP���շ�����\r\n��β����ӡ��ʱ��ȥ��	

	//common
	bool			IsVaildStrArg		(const char* vStr);
	void			SeparateFile		(const char* vPath, char* vFile, char* vDir);
	char*			ChStrReplace		(char* vStr, char* vOldStr, char* vNewStr);

private:
	bool		m_Available;
	SOCKET		m_SockCtrl;				//��������socket 
}; 
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------