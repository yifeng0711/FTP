#ifndef MC_KernelWork_H
#define MC_KernelWork_H
//---------------------------------------------------------------------------
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
//---------------------------------------------------------------------------
#include <Windows.h>
#include <string>
using namespace std;

#include "curl/curl.h"
#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

#include "InnerDef.h"
//---------------------------------------------------------------------------
class MC_KernelWork
{
public:
	MC_KernelWork(void);
	~MC_KernelWork(void);

	FTPErrcode	Init		(void);
	FTPErrcode	UnInit		(void);

	//vConnectTimeout <=0 ��ʾ300�볬ʱ�� vRequestTimeout <= 0��ʾ���ò���ʱ��vResponseTimeout<=0��ʾ�����ó�ʱ

	/**
	 * Method		: SetTimeout
	 * @brief		: ���ó�ʱʱ��
	 * @param[in]	: vConnectTimeout	FTP���ӳ�ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�300�볬ʱ
	 * @param[in]	: vRequestTimeout	FTP����ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�������ʱ
	 * @param[in]	: vResponseTimeout	FTPӦ��ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�������
	 */
	FTPErrcode	SetTimeout	(int vConnectTimeout=5, int vRequestTimeout=5, int vResponseTimeout=0);

	/**
	 * Method		: SetEventCB
	 * @brief		: ����FTP�¼��ص�
	 * @param[in]	: vCb		FTP�¼��ص�����		void CB(FTPEvent event, void* pData, void* pUserData)
	 * @param[in]	: vUserData	���ݸ��ص��������û�����
	 */
	FTPErrcode	SetEventCB	(FTPEventCB vCb, void* vUserData);

	//֧�ֶϵ��������Դ��䲿���ļ����½�0K�ļ���δ�����ļ����Ծ�ͨ�����ܹ�������ʾ����
	FTPErrcode	FtpUpload			(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);
	FTPErrcode	FtpDownload			(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);

private:
	static	int	ProgressCB			(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	long		GetLocalFileSize	(FILE* vFp);
	long		GetRemoteFileSize	(const char* vFtpUrl, const char* vKey);


private:
	bool		m_Available;
	int			m_ConnectTimeout;		//���ӳ�ʱ
	int			m_RequestTimeout;		//����ʱ
	int			m_ResponseTimeout;		//Ӧ��ʱ�����ʱ��ӦС��Ӧ��ʱ

	FTPEventCB	m_FTPEventCB;
	void*		m_EventUserData;
};
//---------------------------------------------------------------------------
#endif