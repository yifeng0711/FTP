#include "MC_KernelWork.h"

#include <sys/stat.h>
//---------------------------------------------------------------------------
#define MAX_BUFFER_SIZE								1024
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3000000
//---------------------------------------------------------------------------
typedef struct CustomProgress_tag
{
	CURL*	curl;
}CustomProgress;
//---------------------------------------------------------------------------
MC_KernelWork::MC_KernelWork(void)
{
	m_Available	= false;
	
	m_ConnectTimeout	= 0;
	m_RequestTimeout	= 0;
	m_ResponseTimeout	= 0;
}
//---------------------------------------------------------------------------
MC_KernelWork::~MC_KernelWork(void)
{
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::Init()
{
	if(true == m_Available)
		return eErrSuccess;

	//加载配置文件
	//加载其他库

	CURLcode tRet = curl_global_init(CURL_GLOBAL_ALL);
	if(CURLE_OK != tRet)
		return eErrFail;

	m_Available = true;
	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::UnInit()
{
	if(false == m_Available)
		return eErrSuccess;

	curl_global_cleanup();	//调用完后还是有泄漏句柄，没有清理干净

	m_Available = false;
	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::SetTimeout(int vConnectTimeout/* =5 */, int vRequestTimeout/* =5 */, int vResponseTimeout/* =0 */)
{
	if(false == m_Available)
		return eErrNotInit;

	#define SET_TIME_OUT(_X_, _Y_) {\
			if(0 >= _X_){\
				_Y_ = 0;\
			}\
			else{\
				_Y_ = _X_;\
			}\
		}

	SET_TIME_OUT(vConnectTimeout,	m_ConnectTimeout);
	SET_TIME_OUT(vRequestTimeout,	m_RequestTimeout);
	SET_TIME_OUT(vResponseTimeout,	m_ResponseTimeout);

	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::SetEventCB(FTPEventCB vCb, void* vUserData)
{
	if(false == m_Available)
		return eErrNotInit;

	m_FTPEventCB	= vCb;
	m_EventUserData	= vUserData;

	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::FtpUpload(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile)
{
	if(false == m_Available)		return eErrNotInit;

	if(0 == vIp)					return eErrInvalidParam;
	if(0 == *vIp)					return eErrInvalidParam;

	if(0 > vPort || 65535 < vPort)	return eErrInvalidParam;

	if(0 == vUser)					return eErrInvalidParam;
	if(0 == *vUser)					return eErrInvalidParam;

	if(0 == vPwd)					return eErrInvalidParam;
	if(0 == *vPwd)					return eErrInvalidParam;

	if(0 == vLocalFile)				return eErrInvalidParam;
	if(0 == *vLocalFile)			return eErrInvalidParam;

	if(0 == vRemoteFile)			return eErrInvalidParam;
	if(0 == *vRemoteFile)			return eErrInvalidParam;
	
	char tUrl[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tUrl, sizeof(tUrl), "ftp://%s:%d/%s", vIp, vPort, vRemoteFile);

	char tKey[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tKey, sizeof(tKey), "%s:%s", vUser, vPwd);

	CURL*		tCurlHandle	= 0; 
	CURLcode	tCurlRet	= CURLE_GOT_NOTHING;
	FILE*		tLocalFp	= NULL;
	
	tLocalFp = fopen(vLocalFile, "rb");
	if (0 == tLocalFp) 
	{
		return eErrFileOpenFail;
	}

	//init curl handle
	tCurlHandle = curl_easy_init();
	if(0 == tCurlHandle)
	{
		return eErrFail;
	}

#ifdef _DEBUG
	//curl_easy_setopt(tCurlHandle, CURLOPT_VERBOSE, 1L); // if set 1, debug mode will print some low level msg
#endif
	
	curl_easy_setopt(tCurlHandle, CURLOPT_UPLOAD, 1L);	//设置上传模式

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, tUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, tKey);

	/** 
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。 
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	curl_easy_setopt(tCurlHandle, CURLOPT_READDATA, tLocalFp);			//指定上传文件

	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

	// set upload progress
	CustomProgress tProgress;
	tProgress.curl = tCurlHandle;
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCB);
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFODATA, &tProgress);
	curl_easy_setopt(tCurlHandle, CURLOPT_NOPROGRESS, 0);

	//upload: 断点续传
	long tRemoteFileSize	= GetRemoteFileSize(tUrl, tKey);
	long tLocalFileSize		= GetLocalFileSize(tLocalFp);
	long tSize				= tLocalFileSize - tRemoteFileSize;
	curl_easy_setopt(tCurlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)tSize);	//声明需要上传的大小，否则进度回调的时候无法获取总大小, tSize必须转为curl_off_t类型，否则出错

	fseek(tLocalFp, tRemoteFileSize, SEEK_SET);
	curl_easy_setopt(tCurlHandle, CURLOPT_APPEND, 1L);

	//CURLE_LOGIN_DENIED	CURLE_COULDNT_RESOLVE_HOST
	tCurlRet = curl_easy_perform(tCurlHandle);
	
	fclose(tLocalFp);

	FTPErrcode tCurlState = eErrSuccess;
	if (CURLE_OK == tCurlRet)
	{
		tCurlState = eErrSuccess;
	}
	else
	{
		//fprintf(stderr, "%s\n", curl_easy_strerror(tCurlRet));
		tCurlState = eErrFail;
	}

	// exit curl handle
	curl_easy_cleanup(tCurlHandle);
	return tCurlState;
}
//---------------------------------------------------------------------------
FTPErrcode MC_KernelWork::FtpDownload(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile)
{
	if(false == m_Available)		return eErrNotInit;

	if(0 == vIp)					return eErrInvalidParam;
	if(0 == *vIp)					return eErrInvalidParam;

	if(0 > vPort || 65535 < vPort)	return eErrInvalidParam;

	if(0 == vUser)					return eErrInvalidParam;
	if(0 == *vUser)					return eErrInvalidParam;

	if(0 == vPwd)					return eErrInvalidParam;
	if(0 == *vPwd)					return eErrInvalidParam;

	if(0 == vLocalFile)				return eErrInvalidParam;
	if(0 == *vLocalFile)			return eErrInvalidParam;

	if(0 == vRemoteFile)			return eErrInvalidParam;
	if(0 == *vRemoteFile)			return eErrInvalidParam;

	char tUrl[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tUrl, sizeof(tUrl), "ftp://%s:%d/%s", vIp, vPort, vRemoteFile);

	char tKey[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tKey, sizeof(tKey), "%s:%s", vUser, vPwd);

	CURL*		tCurlHandle	= 0; 
	CURLcode	tCurlRet	= CURLE_GOT_NOTHING;
	FILE*		tLocalFp	= NULL;
	long		tFileSize		= 0;

	// read tLocalFp in append mode: 断点续传
	tLocalFp = fopen(vLocalFile, "ab+");
	if (NULL == tLocalFp)
	{
		return eErrFileOpenFail;
	}

	// init curl handle
	tCurlHandle = curl_easy_init();
	if(0 == tCurlHandle)
	{
		return eErrFail;
	}

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, tUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, tKey);

	/** 
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。 
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	// set header process, get content length callback
	//curl_easy_setopt(tCurlHandle, CURLOPT_HEADERFUNCTION, getContentLengthFunc);
	//curl_easy_setopt(tCurlHandle, CURLOPT_HEADERDATA, &tFileSize);

	// 断点续传 set download resume, if use resume, set current local tLocalFp length
	curl_easy_setopt(tCurlHandle, CURLOPT_RESUME_FROM_LARGE, curl_off_t(GetLocalFileSize(tLocalFp)));	//必须将size转为curl_off_t类型，否则出错

	//curl_easy_setopt(tCurlHandle, CURLOPT_WRITEFUNCTION, writeFunc);
	curl_easy_setopt(tCurlHandle, CURLOPT_WRITEDATA, tLocalFp);

	// set download progress
	CustomProgress tProgress;
	tProgress.curl = tCurlHandle;
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCB);
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFODATA, &tProgress);
	curl_easy_setopt(tCurlHandle, CURLOPT_NOPROGRESS, 0);

	tCurlRet = curl_easy_perform(tCurlHandle);
	fclose(tLocalFp);

	FTPErrcode tCurlState = eErrSuccess;
	if (tCurlRet == CURLE_OK)
	{
		tCurlState = eErrSuccess;
	}
	else
	{
		//fprintf(stderr, "%s\n", curl_easy_strerror(tCurlRet));
		tCurlState = eErrFail;
	}

	// exit curl handle
	curl_easy_cleanup(tCurlHandle);

	return tCurlState;
}
//---------------------------------------------------------------------------
int MC_KernelWork::ProgressCB(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	CustomProgress*	tProgress	= (CustomProgress*)p;

	double ulspeed_curl = 0.0;
	curl_easy_getinfo(tProgress->curl, CURLINFO_SPEED_UPLOAD, &ulspeed_curl);

	/*fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
		"  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
		" AVRUP: %.1f"
		"\r\n",
		ulnow, ultotal, dlnow, dltotal, ulspeed_curl);*/


	//bytes
	printf("UP: %I64d of %I64d, DOWN: %I64d of %I64d, AVRUP: %.1f\n", ulnow, ultotal, dlnow, dltotal, ulspeed_curl);
    if (ultotal)
        printf("UP tProgress: %0.2f\n\n", (float)ulnow / ultotal);
    if (dltotal)
        printf("DOWN tProgress: %0.2f\n\n", (float)dlnow / dltotal);

    return 0;
}
//---------------------------------------------------------------------------
long MC_KernelWork::GetLocalFileSize(FILE* vFp)
{
	long	tCurrentPos	= 0;
	long	tFileSize	= 0;

	tCurrentPos = ftell(vFp);
	fseek(vFp, 0, SEEK_END);

	tFileSize = ftell(vFp);
	if(0 > tFileSize)
		tFileSize = 0;

	fseek(vFp, tCurrentPos, SEEK_SET);

	return tFileSize;
}
//---------------------------------------------------------------------------
long MC_KernelWork::GetRemoteFileSize(const char* vFtpUrl, const char* vKey)
{
	if(false == m_Available)	return 0;

	if(0 == vFtpUrl)			return 0;
	if(0 == *vFtpUrl)			return 0;

	if(0 == vKey)				return 0;
	if(0 == *vKey)				return 0;

	CURL*		tCurlHandle	= 0;
	CURLcode	tCurlRet	= CURLE_GOT_NOTHING;
	double		tFileSize	= 0.0;

	tCurlHandle = curl_easy_init();
	if(0 == tCurlHandle)
		return 0;

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, vFtpUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, vKey);

	/** 
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。 
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	/* No download if the file */ 
	curl_easy_setopt(tCurlHandle, CURLOPT_NOBODY, 1L);
	/* Switch on full protocol/debug output */ 
	/* curl_easy_setopt(tCurlHandle, CURLOPT_VERBOSE, 1L); */ 

	//CURLE_FTP_COULDNT_RETR_FILE
	tCurlRet = curl_easy_perform(tCurlHandle);
	if(CURLE_OK != tCurlRet)
	{
		return 0;
	}

	tCurlRet = curl_easy_getinfo(tCurlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &tFileSize);
	if((CURLE_OK != tCurlRet) || (0.0 > tFileSize))
	{
		return 0;
	}

	return (long)tFileSize;
}
//---------------------------------------------------------------------------
