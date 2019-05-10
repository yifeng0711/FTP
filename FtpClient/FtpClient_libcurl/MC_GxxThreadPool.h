//---------------------------------------------------------------------------
/**
 * 解决多线程上传中的内存泄漏问题
 */
//---------------------------------------------------------------------------
#ifndef MC_GxxThreadPool_H
#define MC_GxxThreadPool_H
//---------------------------------------------------------------------------
#include "GSThreadPool.h"
//#include "GSDefs.h"
//#include "GSSysApi.h"
//#include "GSTime.h"
#include <vector>
#include <queue>
#include "MC_FtpClient.h"
#include "MC_Log.h"
#include "util.h"

#pragma comment(lib, "../3rdparty/Foundation/GSFoundation.lib")
//---------------------------------------------------------------------------
class MC_GxxThreadPool
{
public:
	MC_GxxThreadPool(void);
	~MC_GxxThreadPool(void);

	bool	OnLoad		(int vMinThreads, int vMaxThreads);
	bool	OnExit		(void);

	bool	DoWork		(void);

private:
	static	void	DealFtpEvent	(FTPEvent event, void* pData, void* pUserData);

	//任务队列获取线程
	static	void	GS_CALLBACK	GetTaskThreadFunc	(void *pUserParam, long lThreadID);
	void						GetTaskWorkFunc		(void);

	//任务队列处理/FTP上传线程
	static	void	GS_CALLBACK	WorkCallback		(void *pUserParam, long lThreadID);
	void						WorkFunc			(long lThreadID);

private:
	bool			TraversalFilesInDir	(const char* vDir, std::vector<std::string>&vFileList, const char* vSuffixName=NULL, bool vIsRecursive=false);
	std::string 	STRReplace			(const std::string& vString, const std::string& vSubStrFrom, const std::string& vSubStrTo);


private:
	bool					m_Available;
	GSRefThreadPool*		m_RefThreadPool;

	MutexHandle				m_Mutex;

	std::queue<std::string>	m_TaskQueue;
	MC_FtpClient*			m_FtpClient;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------