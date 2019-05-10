#ifndef MC_FtpClient_H
#define MC_FtpClient_H
//---------------------------------------------------------------------------
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
//---------------------------------------------------------------------------
#include <Windows.h>
#include <queue>

#include <string>
using namespace std;

/*7.50.3*/
#include "curl/curl.h"
#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

#include "InnerDef.h"
#include "util.h"
//---------------------------------------------------------------------------
class MC_FtpClient
{
public:
	MC_FtpClient(void);
	~MC_FtpClient(void);

	FTPErrcode		Init		(void);
	FTPErrcode		UnInit		(void);

	//vConnectTimeout <=0 ��ʾ300�볬ʱ�� vRequestTimeout <= 0��ʾ���ò���ʱ��vResponseTimeout<=0��ʾ�����ó�ʱ

	/**
	 * Method		: SetTimeout
	 * @brief		: ���ó�ʱʱ�䣬���µ��ã�����ʹ�¸��������ó�ʱʱ��
	 * @param[in]	: vConnectTimeout	FTP���ӳ�ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�300�볬ʱ
	 * @param[in]	: vRequestTimeout	FTP����ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�������ʱ
	 * @param[in]	: vResponseTimeout	FTPӦ��ʱʱ�䣬��λ�룬<=0ʱ��ʾĬ��ѡ�������
	 */
	FTPErrcode		SetTimeout	(int vConnectTimeout=5, int vRequestTimeout=5, int vResponseTimeout=0);

	/**
	 * Method		: SetEventCB
	 * @brief		: ����FTP�¼��ص�
	 * @param[in]	: vCb		FTP�¼��ص�����		void CB(FTPEvent event, void* pData, void* pUserData)
	 * @param[in]	: vUserData	���ݸ��ص��������û�����
	 */
	FTPErrcode		SetEventCB	(FTPEventCB vEventCb, void* vEvnetUserData);

	/**
	 * ֧�ֶϵ��������Դ��䲿���ļ����½�0K�ļ���δ�����ļ����Ծ�ͨ�����ܹ�������ʾ����
	 * ���ñ���ģʽ��	(1)�ͻ�����������˷�������
						(2)�Թ���ͻ������������赣�Ŀͻ��˷���ǽ�������ݶ˿�
	 *					(2)����ͨ��ΪFTP������ָ�� һ�����޵Ķ˿ڷ�Χ����С��������λ�˿ڵı�¶
	 */
	FTPErrcode		FtpUploadFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);
	FTPErrcode		FtpDownloadFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);

	bool			FTPDeleteFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vRemoteFile);

private:
	long			GetLocalFileSize	(FILE* vFp);
	long			GetRemoteFileSize	(const char* vFtpUrl, const char* vKey);

	void			PushNotify			(FTPEvent vEvent, void* vData);
	void			DealNotify			(NOTIFY_tag* pNotify);

private:	//�ص�
	static	int		ProgressCB			(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	static	size_t	ReadFunc			(void *ptr, size_t size, size_t nmemb, void *stream);
	static	size_t	DiscardFunc			(void *ptr, size_t size, size_t nmemb, void *stream);

	static size_t	FtpReqReply			(void *ptr, size_t size, size_t nmemb, void *stream);

private:
	//�¼�֪ͨ�߳�
	bool            		m_NotifyThreadRun;
	ThreadHandle    		m_NotifyThreadHandle;
	MutexHandle				m_NotifyMutex;
	std::queue<Notify_tag*>	m_NotifyQueue;

	static	void*			NotifyThreadCB		(void* p);
	void					NotifyThreadWork	(void);	

private:
	bool		m_Available;
	int			m_ConnectTimeout;		//���ӳ�ʱ
	int			m_RequestTimeout;		//����ʱ
	int			m_ResponseTimeout;		//Ӧ��ʱ�����ʱ��ӦС������ʱ

	FTPEventCB	m_EventCB;
	void*		m_EventUserData;

};
//---------------------------------------------------------------------------
#endif