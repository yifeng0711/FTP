#include "MC_FtpClient.h"

#include <sys/stat.h>
//---------------------------------------------------------------------------
#define MAX_BUFFER_SIZE								1024
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3000000
//---------------------------------------------------------------------------
typedef struct CustomProgress_tag
{
	CURL*			hCurl;
	MC_FtpClient*	pThis;
	curl_off_t		iNow;
}CustomProgress;
//---------------------------------------------------------------------------
MC_FtpClient::MC_FtpClient(void)
{
	m_Available	= false;
	
	m_ConnectTimeout	= 0;
	m_RequestTimeout	= 0;
	m_ResponseTimeout	= 0;

	m_NotifyThreadRun	= false;
	m_NotifyThreadHandle= NULL;	
	m_NotifyMutex		= NULL;

	m_EventCB			= NULL;
	m_EventUserData		= NULL;
}
//---------------------------------------------------------------------------
MC_FtpClient::~MC_FtpClient(void)
{
}
//---------------------------------------------------------------------------
FTPErrcode MC_FtpClient::Init()
{
	if(true == m_Available)
		return eErrSuccess;

	//���������ļ�
	//����������

	CURLcode tRet = curl_global_init(CURL_GLOBAL_ALL);
	if(CURLE_OK != tRet)
		return eErrFail;

	//�����¼�֪ͨ
	m_NotifyThreadRun = true;
	WINThreadCreate(&m_NotifyThreadHandle, NotifyThreadCB, this);
	MutexInit(&m_NotifyMutex);

	m_Available = true;
	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_FtpClient::UnInit()
{
	if(false == m_Available)
		return eErrSuccess;

	curl_global_cleanup();	//�����������й©�����û������ɾ�

	if(true == m_NotifyThreadRun)
	{
		m_NotifyThreadRun = false;
		WINThreadJoin(m_NotifyThreadHandle);
	}

	if(NULL != m_NotifyMutex)
	{
		MutexDestroy(&m_NotifyMutex);
	}

	m_Available = false;
	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_FtpClient::SetTimeout(int vConnectTimeout/* =5 */, int vRequestTimeout/* =5 */, int vResponseTimeout/* =0 */)
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
FTPErrcode MC_FtpClient::SetEventCB(FTPEventCB vEventCb, void* vEventUserData)
{
	if(false == m_Available)
		return eErrNotInit;

	m_EventCB		= vEventCb;
	m_EventUserData	= vEventUserData;

	return eErrSuccess;
}
//---------------------------------------------------------------------------
FTPErrcode MC_FtpClient::FtpUploadFile(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile)
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
		fclose(tLocalFp);
		return eErrFail;
	}

#ifdef _DEBUG
	//curl_easy_setopt(tCurlHandle, CURLOPT_VERBOSE, 1L); // if set 1, debug mode will print some low level msg
#endif
	
	curl_easy_setopt(tCurlHandle, CURLOPT_UPLOAD, 1L);	//�����ϴ�ģʽ

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, tUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, tKey);

	/** 
    * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ����� 
    * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳��� 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	//���ö�ȡ����ʱ�Ļص������ݻص��û�ָ��
	curl_easy_setopt(tCurlHandle, CURLOPT_READFUNCTION, ReadFunc);
	curl_easy_setopt(tCurlHandle, CURLOPT_READDATA, tLocalFp);

	//���ý��Ȼص������ݻص��û�ָ��
	CustomProgress tProgress;
	tProgress.hCurl	= tCurlHandle;
	tProgress.pThis	= this;
	tProgress.iNow	= 0;
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCB);
	curl_easy_setopt(tCurlHandle, CURLOPT_XFERINFODATA, &tProgress);
	curl_easy_setopt(tCurlHandle, CURLOPT_NOPROGRESS, 0);

	//curl_easy_setopt(curlhandle, CURLOPT_FTPPORT, "-");	//���ñ���ģʽ
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

	//upload: �ϵ�����
	long tRemoteFileSize	= GetRemoteFileSize(tUrl, tKey);
	long tLocalFileSize		= GetLocalFileSize(tLocalFp);
	long tSize				= tLocalFileSize - tRemoteFileSize;
	curl_easy_setopt(tCurlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)tSize);	//������Ҫ�ϴ��Ĵ�С��������Ȼص���ʱ���޷���ȡ�ܴ�С, tSize����תΪcurl_off_t���ͣ��������

	fseek(tLocalFp, tRemoteFileSize, SEEK_SET);
	curl_easy_setopt(tCurlHandle, CURLOPT_APPEND, 1L);


	//curl_easy_setopt(tCurlHandle, CURLOPT_MAX_SEND_SPEED_LARGE, 1024L);

	//CURLE_LOGIN_DENIED	CURLE_COULDNT_RESOLVE_HOST
	tCurlRet = curl_easy_perform(tCurlHandle);
	
	fclose(tLocalFp);

	FTPErrcode tCurlState = eErrSuccess;
	if (CURLE_OK == tCurlRet)
	{
		StructUploadResult* tUploadResult = new StructUploadResult();
		tUploadResult->eErrcode			= eErrSuccess;
		tUploadResult->strLocalFile		= vLocalFile;
		tUploadResult->strRemoteFile	= vRemoteFile;
		PushNotify(eUploadResult, tUploadResult);

		tCurlState = eErrSuccess;
	}
	else
	{
		StructUploadResult* tUploadResult = new StructUploadResult();
		tUploadResult->eErrcode			= eErrFail;
		tUploadResult->strLocalFile		= vLocalFile;
		tUploadResult->strRemoteFile	= vRemoteFile;
		tUploadResult->eErrstr	= curl_easy_strerror(tCurlRet);
		PushNotify(eUploadResult, tUploadResult);

		tCurlState = eErrFail;
	}

	// exit curl handle
	curl_easy_cleanup(tCurlHandle);
	return tCurlState;
}
//---------------------------------------------------------------------------
FTPErrcode MC_FtpClient::FtpDownloadFile(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vLocalFile, const char* vRemoteFile)
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

	// read tLocalFp in append mode: �ϵ�����
	tLocalFp = fopen(vLocalFile, "ab+");
	if (NULL == tLocalFp)
	{
		return eErrFileOpenFail;
	}

	// init curl handle
	tCurlHandle = curl_easy_init();
	if(0 == tCurlHandle)
	{
		fclose(tLocalFp);
		return eErrFail;
	}

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, tUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, tKey);

	/** 
    * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ����� 
    * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳��� 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	//curl_easy_setopt(curlhandle, CURLOPT_FTPPORT, "-");	//���ñ���ģʽ

	// �ϵ����� set download resume, if use resume, set current local tLocalFp length
	curl_easy_setopt(tCurlHandle, CURLOPT_RESUME_FROM_LARGE, curl_off_t(GetLocalFileSize(tLocalFp)));	//���뽫sizeתΪcurl_off_t���ͣ��������

	//curl_easy_setopt(tCurlHandle, CURLOPT_WRITEFUNCTION, writeFunc);
	curl_easy_setopt(tCurlHandle, CURLOPT_WRITEDATA, tLocalFp);

	// set download progress
	CustomProgress tProgress;
	tProgress.hCurl = tCurlHandle;
	tProgress.pThis = this;
	tProgress.iNow	= 0;
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
bool MC_FtpClient::FTPDeleteFile(const char* vIp, int vPort, const char* vUser, const char* vPwd, const char* vRemoteFile)
{
	char tUrl[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tUrl, sizeof(tUrl), "ftp://%s:%d/", vIp, vPort);

	char tKey[MAX_BUFFER_SIZE] = {0};
	sprintf_s(tKey, sizeof(tKey), "%s:%s", vUser, vPwd);

	char tCmd[1024] = {0};
	sprintf_s(tCmd, sizeof(tCmd), "DELE %s", vRemoteFile);

	CURL*		tCurlHandle	= 0; 
	CURLcode	tCurlRet	= CURLE_GOT_NOTHING;
	
	//init curl handle
	tCurlHandle = curl_easy_init();
	if(0 == tCurlHandle)
	{
		return eErrFail;
	}

	curl_easy_setopt(tCurlHandle, CURLOPT_URL, tUrl);
	curl_easy_setopt(tCurlHandle, CURLOPT_USERPWD, tKey);

	struct curl_slist* tHeadList = NULL;
	tHeadList = curl_slist_append(tHeadList, tCmd);
	curl_easy_setopt(tCurlHandle, CURLOPT_QUOTE, tHeadList);

	/** 
    * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ����� 
    * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳��� 
    */  
    curl_easy_setopt(tCurlHandle, CURLOPT_NOSIGNAL,	1);  
    curl_easy_setopt(tCurlHandle, CURLOPT_CONNECTTIMEOUT, m_ConnectTimeout);  
    curl_easy_setopt(tCurlHandle, CURLOPT_TIMEOUT, m_RequestTimeout);  
	curl_easy_setopt(tCurlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, m_ResponseTimeout);

	curl_easy_setopt(tCurlHandle, CURLOPT_WRITEFUNCTION, DiscardFunc);	//���ûص���������

	tCurlRet = curl_easy_perform(tCurlHandle);
	if (tHeadList != NULL)
	{
		curl_slist_free_all(tHeadList); //free the list again
	}
	// Check for errors
	if (tCurlRet != CURLE_OK)
	{
		curl_easy_cleanup(tCurlHandle);
		return false;
	}
	
	curl_easy_cleanup(tCurlHandle);
	return true;
}
//---------------------------------------------------------------------------
long MC_FtpClient::GetLocalFileSize(FILE* vFp)
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
long MC_FtpClient::GetRemoteFileSize(const char* vFtpUrl, const char* vKey)
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
    * ������̶߳�ʹ�ó�ʱ�����ʱ��ͬʱ���߳�����sleep����wait�Ȳ����� 
    * ������������ѡ�libcurl���ᷢ�źŴ�����wait�Ӷ����³����˳��� 
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
		curl_easy_cleanup(tCurlHandle);
		return 0;
	}

	tCurlRet = curl_easy_getinfo(tCurlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &tFileSize);
	if((CURLE_OK != tCurlRet) || (0.0 > tFileSize))
	{
		curl_easy_cleanup(tCurlHandle);
		return 0;
	}

	curl_easy_cleanup(tCurlHandle);
	return (long)tFileSize;
}
//---------------------------------------------------------------------------
void MC_FtpClient::PushNotify(FTPEvent vEvent, void* vData)
{
	if(NULL == m_EventCB)	
		return;

	NOTIFY_tag* pNotify = new NOTIFY_tag;
	pNotify->evt = vEvent;
	pNotify->data = vData;

	MutexLock(&m_NotifyMutex);
	m_NotifyQueue.push(pNotify);
	MutexUnlock(&m_NotifyMutex);
}
//---------------------------------------------------------------------------
void MC_FtpClient::DealNotify(NOTIFY_tag* pNotify)
{
	if(NULL == m_EventCB)	return;
	if(NULL == pNotify)		return;

	m_EventCB(pNotify->evt, pNotify->data, m_EventUserData);

	//�ͷ�pNotify->data
	switch(pNotify->evt)
	{
	case eUploadResult:
		{
			StructUploadResult* tUploadResult = (StructUploadResult*)pNotify->data;
			SafeDeletePoint(tUploadResult);
		}
		break;

	case eUploadProgress:
		{
			StructUploadProgress* tUploadProgress = (StructUploadProgress*)pNotify->data;
			SafeDeletePoint(tUploadProgress);
		}
		break;

	default:
		break;
	}

	SafeDeletePoint(pNotify);
}
//---------------------------------------------------------------------------
/********** ********** libcurl�ص� ********** **********/
//---------------------------------------------------------------------------
int MC_FtpClient::ProgressCB(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	CustomProgress*	tProgress	= (CustomProgress*)p;
	curl_off_t		tNow		= tProgress->iNow;
	double			tSpeed		= 0.0;

	if(ultotal)
	{
		
		curl_easy_getinfo(tProgress->hCurl, CURLINFO_SPEED_UPLOAD, &tSpeed);	//ƽ���ϴ��ٶȣ���λ��bytes/second
		//printf("\rulnow: %10I64d, ultotal: %10I64d, Speed: %10.1lf, progress: %1.2lf%%", ulnow, ultotal, tSpeed, (float)ulnow / ultotal  * 100);
		//printf("\n\n");

		if(tNow != ulnow)	//ȥ����ͬ�����¼����¼�̫Ƶ���ᵼ���ڴ��ͷŲ���ʱ������ڴ�й©
		{
			StructUploadProgress* tUploadProgress = new StructUploadProgress();
			tUploadProgress->ulNow		= (INT64)ulnow;
			tUploadProgress->ulTotal	= (INT64)ultotal;
			tProgress->pThis->PushNotify(eUploadProgress, tUploadProgress);
		}
		tProgress->iNow = ulnow;
	}

	if(dltotal)
	{
		curl_easy_getinfo(tProgress->hCurl, CURLINFO_SPEED_DOWNLOAD, &tSpeed);
		//printf("\rdlnow: %10I64d, dltotal: %10I64d, Speed: %10.1lf, progress: %1.2lf%%", dlnow, dltotal, tSpeed, (float)dlnow / dltotal * 100);

		if(tNow != dlnow)	//ȥ����ͬ�����¼����¼�̫Ƶ���ᵼ���ڴ��ͷŲ���ʱ������ڴ�й©
		{
			StructUploadProgress* tUploadProgress = new StructUploadProgress();
			tUploadProgress->ulNow		= (INT64)dlnow;
			tUploadProgress->ulTotal	= (INT64)dltotal;
			tProgress->pThis->PushNotify(eUploadProgress, tUploadProgress);
		}
		tProgress->iNow = dlnow;
	}

	return 0;
}
//---------------------------------------------------------------------------
size_t MC_FtpClient::ReadFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	FILE*	tFp			= (FILE*)stream;
	size_t	tReadNum	= 0;

	if(0 != ferror(tFp))
	{
		return CURL_READFUNC_ABORT;
	}

	tReadNum = fread(ptr, size, nmemb, tFp) * size;
	return tReadNum;
}
//---------------------------------------------------------------------------
size_t MC_FtpClient::DiscardFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	(void)ptr;
	(void)stream;
	return size * nmemb;
}
//---------------------------------------------------------------------------
//ftp��������Ӧ�����ݻص�����
size_t MC_FtpClient::FtpReqReply(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if (stream == NULL || ptr == NULL || size == 0)
		return 0;
 
	size_t realsize = size * nmemb;
	std::string *buffer = (std::string*)stream;
	if (buffer != NULL)
	{
		buffer->append((const char *)ptr, realsize);
	}
	return realsize;
}
//---------------------------------------------------------------------------
/********** ********** �̻߳ص����� ********** **********/
//---------------------------------------------------------------------------
void* MC_FtpClient::NotifyThreadCB(void* p)
{
	MC_FtpClient* tKernelWork = (MC_FtpClient*)p;
	tKernelWork->NotifyThreadWork();

	return 0;
}
//---------------------------------------------------------------------------
void MC_FtpClient::NotifyThreadWork()
{
	while(false != m_NotifyThreadRun)
	{
		NOTIFY_tag* pNotify = NULL;
		MutexLock(&m_NotifyMutex);

		if(0 != m_NotifyQueue.size())
		{
			pNotify = m_NotifyQueue.front();
			m_NotifyQueue.pop();
		}

		MutexUnlock(&m_NotifyMutex);

		if(NULL != pNotify)
			DealNotify(pNotify);

		//Sleep(5);	//���ڶ�����ʱ�����Ȼص��¼�̫�࣬sleep���ܵ����ڴ��ͷŲ���ʱ
	}
}
//---------------------------------------------------------------------------