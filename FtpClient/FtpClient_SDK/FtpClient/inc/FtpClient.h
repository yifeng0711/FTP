#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
FtpClientΪ�ļ��ϴ������ؿͻ��ˣ�֧�ֶ��ļ�ͬʱ�ϴ������أ�֧������ļ�1G��֧������ٶ�Ϊ100M/S��֧��ȡ���ϴ������ء�
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

//////////////////////JSON�¼�//////////////////////////////
//�¼�JSON
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
//�����¼�JSON
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

#define       DIR_UPLOAD                         0//�ϴ�
#define       DIR_DOWNLOAD                       1//����

#define       CODE_OK                            0//�ɹ��¼�
#define       CODE_PROGRESS                      1//�����¼�
#define       CODE_UPLOAD_FAIL                   2//�ϴ�ʧ���¼�
#define       CODE_DOWNLOAD_FAIL                 3//����ʧ���¼�
#define       CODE_ERROR                         4//���������¼�
#define       CODE_OPEN_FILE_FAIL                5//���ļ�ʧ���¼�
#define       CODE_CREATE_FILE_FAIL              6//�����ļ�ʧ���¼�

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
����һ��FtpClient���󣬲�����FtpClientHandle���handle��ftppathΪFTP·������/��β��userΪFTP�˺ţ�passwordΪFTP���룻oneventΪ�¼��ص�������contextΪ�ص�������
*/
int FTPCLIENT CreateFtpClient(FtpClientHandle* handle,const char* ftppath,const char* user,const char* password,ONEVENT onevent,void* context);
/*
�ر�FtpClient��handleΪFtpClient������ָ�룻waitΪTRUEʱ��������ļ������ϴ��������أ����ȴ�����ɣ�ΪFALSEʱ��������ļ������ϴ��������أ�����ȡ����
*/
int FTPCLIENT CloseFtpClient(FtpClientHandle* handle,BOOL_T wait);
/*
�ϴ��ļ���handleΪFtpClient��������ftpfilenameΪFTP��������·����
*/
BOOL_T FTPCLIENT DeleteFtpFile(FtpClientHandle handle,const char* ftpfilename);
/*
�ϴ��ļ���handleΪFtpClient��������taskidΪ�ļ�ID����NewTaskid���ɣ�filenameΪ�ļ�ȫ·����ftpfilenameΪFTP��������·����
*/
BOOL_T FTPCLIENT UploadFile(FtpClientHandle handle,const char* taskid,const char* filename,const char* ftpfilename=NULL);
/*
�����ļ���handleΪFtpClient��������taskidΪ�ļ�ID��filenameΪ�ļ�ȫ·����ftpfilenameΪFTP��������·����
*/
BOOL_T FTPCLIENT DownloadFile(FtpClientHandle handle,const char* taskid,const char* filename,const char* ftpfilename=NULL);
/*
ȡ�������ϴ��������أ�handleΪFtpClient��������taskidΪ�ļ�ID��
*/
BOOL_T FTPCLIENT CancelLoadFile(FtpClientHandle handle,const char* taskid);
/*
�����ļ�ID����һ��GUID��taskid�����ļ�ID����Ҫ�����㹻�Ŀռ䣻sizeΪ����ռ�Ĵ�С��
*/
BOOL_T FTPCLIENT NewTaskid(char* taskid,unsigned int size);

#ifdef __cplusplus
}
#endif

#endif//FTPCLIENT_H