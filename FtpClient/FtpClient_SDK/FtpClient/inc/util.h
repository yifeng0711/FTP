
#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#ifndef UTILAPI
#define UTILAPI      __stdcall
#endif

typedef HANDLE  MutexHandle;
typedef HANDLE  ThreadHandle;

#ifdef __cplusplus
extern "C"
{
#endif

	int UTILAPI WINThreadCreate(ThreadHandle *handle,void *(*func)(void*), void *arg);
	int UTILAPI WINThreadJoin(ThreadHandle handle);

	int UTILAPI MutexInit(MutexHandle* handle);
	int UTILAPI MutexLock(MutexHandle* handle);
	int UTILAPI MutexUnlock(MutexHandle* handle);
	int UTILAPI MutexDestroy(MutexHandle* handle);

	int UTILAPI GetExeCurrentDir(char* buffer,unsigned int* len);//����-1��ʾbuffer���Ȳ���������0��ʾʧ�ܣ��ɹ��򷵻�·������
	int UTILAPI GetDllCurrentDir(const char* moduleName,char* buffer,unsigned int* len);//����-1��ʾbuffer���Ȳ���������0��ʾʧ�ܣ��ɹ��򷵻�·������

	int UTILAPI MakeLower(char* string);
	int UTILAPI MakeUpper(char* string);

#ifdef __cplusplus
}
#endif

#endif//UTIL_H