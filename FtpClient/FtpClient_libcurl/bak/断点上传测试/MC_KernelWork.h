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

	//vConnectTimeout <=0 表示300秒超时， vRequestTimeout <= 0表示永久不超时，vResponseTimeout<=0表示不启用超时

	/**
	 * Method		: SetTimeout
	 * @brief		: 设置超时时间
	 * @param[in]	: vConnectTimeout	FTP连接超时时间，单位秒，<=0时表示默认选项，300秒超时
	 * @param[in]	: vRequestTimeout	FTP请求超时时间，单位秒，<=0时表示默认选项，永不超时
	 * @param[in]	: vResponseTimeout	FTP应答超时时间，单位秒，<=0时表示默认选项，？？？
	 */
	FTPErrcode	SetTimeout	(int vConnectTimeout=5, int vRequestTimeout=5, int vResponseTimeout=0);

	/**
	 * Method		: SetEventCB
	 * @brief		: 设置FTP事件回调
	 * @param[in]	: vCb		FTP事件回调函数		void CB(FTPEvent event, void* pData, void* pUserData)
	 * @param[in]	: vUserData	传递给回调函数的用户参数
	 */
	FTPErrcode	SetEventCB	(FTPEventCB vCb, void* vUserData);

	//支持断点续传，对传输部分文件，新建0K文件，未创建文件测试均通过，能够正常显示进度
	FTPErrcode	FtpUpload			(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);
	FTPErrcode	FtpDownload			(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile);

private:
	static	int	ProgressCB			(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	long		GetLocalFileSize	(FILE* vFp);
	long		GetRemoteFileSize	(const char* vFtpUrl, const char* vKey);


private:
	bool		m_Available;
	int			m_ConnectTimeout;		//连接超时
	int			m_RequestTimeout;		//请求超时
	int			m_ResponseTimeout;		//应答超时，这个时间应小于应答超时

	FTPEventCB	m_FTPEventCB;
	void*		m_EventUserData;
};
//---------------------------------------------------------------------------
#endif