#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
FtpClient为文件上传和下载客户端，支持多文件同时上传和下载，支持最大文件1G，支持最大速度为100M/S，支持取消上传和下载。
*/

typedef unsigned char BOOL_T;
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#ifndef       FTPCLIENT
#define       FTPCLIENT                          __stdcall
#endif

#ifndef       ONEVENT
typedef void (__stdcall *ONEVENT)(const char* json, unsigned int len, void* context);
#endif

//////////////////////JSON事件//////////////////////////////
//事件JSON
/*
{
	"id":""
	"filename":""
	"filesize":""
	"dir":""
	"code":""
	"desc":""
}
*/
//进度事件JSON
/*
{
	"id":""
	"filename":""
	"dir":""
	"code":""
	"desc":""
	"dtotal":""
	"dnow":""
	"utotal":""
	"unow":""
}
*/
////////////////////////////////////////////////////////////

#define       DIR_UPLOAD                         0//上传
#define       DIR_DOWNLOAD                       1//下载

#define       CODE_OK                            0//成功事件
#define       CODE_PROGRESS                      1//进度事件
#define       CODE_UPLOAD_FAIL                   2//上传失败事件
#define       CODE_DOWNLOAD_FAIL                 3//下载失败事件
#define       CODE_ERROR                         4//发生错误事件
#define       CODE_OPEN_FILE_FAIL                5//打开文件失败事件
#define       CODE_CREATE_FILE_FAIL              6//创建文件失败事件

struct _FtpClientStruct;
typedef struct _Task
{
	struct _Task* Prev;
	struct _Task* Next;
	struct _FtpClientStruct* handle;
	char* taskid;
	char* filename;
	char* ftpfilename;
	ThreadHandle worker;
	int loaddir;
	BOOL_T running;
	FILE* f;
}Task;

typedef struct _FtpClientStruct
{
	char* ftppath;
	char* user;
	char* password;
	Task* firsttask;
	Task* lasttask;
	MutexHandle tasklock;
	ONEVENT onevent;
	void* context;
}FtpClientStruct,*FtpClientHandle;
/*
创建一个FtpClient对象，并返回FtpClientHandle句柄handle；ftppath为FTP路径，以/结尾；user为FTP账号；password为FTP密码；onevent为事件回调函数；context为回调上下文
*/
int FTPCLIENT CreateFtpClient(FtpClientHandle* handle,const char* ftppath,const char* user,const char* password,ONEVENT onevent,void* context);
/*
关闭FtpClient，handle为FtpClient对象句柄指针；wait为TRUE时，如果有文件正在上传或者下载，将等待其完成，为FALSE时，如果有文件正在上传或者下载，将被取消。
*/
int FTPCLIENT CloseFtpClient(FtpClientHandle* handle,BOOL_T wait);
/*
上传文件，handle为FtpClient对象句柄；ftpfilename为FTP服务端相对路径；
*/
BOOL_T FTPCLIENT DeleteFtpFile(FtpClientHandle handle,const char* ftpfilename);
/*
上传文件，handle为FtpClient对象句柄；taskid为文件ID，用NewTaskid生成；filename为文件全路径；ftpfilename为FTP服务端相对路径；
*/
BOOL_T FTPCLIENT UploadFile(FtpClientHandle handle,const char* taskid,const char* filename,const char* ftpfilename=NULL);
/*
下载文件，handle为FtpClient对象句柄；taskid为文件ID；filename为文件全路径；ftpfilename为FTP服务端相对路径；
*/
BOOL_T FTPCLIENT DownloadFile(FtpClientHandle handle,const char* taskid,const char* filename,const char* ftpfilename=NULL);
/*
取消正在上传或者下载，handle为FtpClient对象句柄；taskid为文件ID；
*/
BOOL_T FTPCLIENT CancelLoadFile(FtpClientHandle handle,const char* taskid);
/*
生成文件ID，即一个GUID，taskid接收文件ID，需要分配足够的空间；size为分配空间的大小；
*/
BOOL_T FTPCLIENT NewTaskid(char* taskid,unsigned int size);

#ifdef __cplusplus
}
#endif

#endif//FTPCLIENT_H