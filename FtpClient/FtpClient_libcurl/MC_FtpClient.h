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

	//vConnectTimeout <=0 表示300秒超时， vRequestTimeout <= 0表示永久不超时，vResponseTimeout<=0表示不启用超时

	/**
	 * Method		: SetTimeout
	 * @brief		: 设置超时时间，重新调用，可以使下个任务重置超时时间
	 * @param[in]	: vConnectTimeout	FTP连接超时时间，单位秒，<=0时表示默认选项，300秒超时
	 * @param[in]	: vRequestTimeout	FTP请求超时时间，单位秒，<=0时表示默认选项，永不超时
	 * @param[in]	: vResponseTimeout	FTP应答超时时间，单位秒，<=0时表示默认选项，？？？
	 */
	FTPErrcode		SetTimeout	(int vConnectTimeout=5, int vRequestTimeout=5, int vResponseTimeout=0);

	/**
	 * Method		: SetEventCB
	 * @brief		: 设置FTP事件回调
	 * @param[in]	: vCb		FTP事件回调函数		void CB(FTPEvent event, void* pData, void* pUserData)
	 * @param[in]	: vUserData	传递给回调函数的用户参数
	 */
	FTPErrcode		SetEventCB	(FTPEventCB vEventCb, void* vEvnetUserData);

	/**
	 * 支持断点续传，对传输部分文件，新建0K文件，未创建文件测试均通过，能够正常显示进度
	 * 采用被动模式：	(1)客户端向服务器端发起连接
						(2)对管理客户端有利，不需担心客户端防火墙阻塞数据端口
	 *					(2)可以通过为FTP服务器指定 一个有限的端口范围来减小服务器高位端口的暴露
	 */
	FTPErrcode		FtpUploadFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);
	FTPErrcode		FtpDownloadFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);

	bool			FTPDeleteFile		(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vRemoteFile);

private:
	long			GetLocalFileSize	(FILE* vFp);
	long			GetRemoteFileSize	(const char* vFtpUrl, const char* vKey);

	void			PushNotify			(FTPEvent vEvent, void* vData);
	void			DealNotify			(NOTIFY_tag* pNotify);

private:	//回调
	static	int		ProgressCB			(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	static	size_t	ReadFunc			(void *ptr, size_t size, size_t nmemb, void *stream);
	static	size_t	DiscardFunc			(void *ptr, size_t size, size_t nmemb, void *stream);

	static size_t	FtpReqReply			(void *ptr, size_t size, size_t nmemb, void *stream);

private:
	//事件通知线程
	bool            		m_NotifyThreadRun;
	ThreadHandle    		m_NotifyThreadHandle;
	MutexHandle				m_NotifyMutex;
	std::queue<Notify_tag*>	m_NotifyQueue;

	static	void*			NotifyThreadCB		(void* p);
	void					NotifyThreadWork	(void);	

private:
	bool		m_Available;
	int			m_ConnectTimeout;		//连接超时
	int			m_RequestTimeout;		//请求超时
	int			m_ResponseTimeout;		//应答超时，这个时间应小于请求超时

	FTPEventCB	m_EventCB;
	void*		m_EventUserData;

};
//---------------------------------------------------------------------------
#endif