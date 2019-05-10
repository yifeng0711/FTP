
#include "util.h"
#include <memory.h>
#include <process.h>
#include <stdio.h>

typedef struct _ThreadParam{
	void * (*func)(void *);
	void * arg;
} ThreadParam;

static unsigned int UTILAPI ThreadStarter(void *data){
	ThreadParam *params=(ThreadParam*)data;
	void *ret=params->func(params->arg);
	free(params);
	return (DWORD)ret;
}

int UTILAPI WINThreadCreate(ThreadHandle *handle,void *(*func)(void*), void *arg)
{
	ThreadParam *params=(ThreadParam*)malloc(sizeof(ThreadParam));
	params->func=func;
	params->arg=arg;
	*handle=(HANDLE)_beginthreadex( NULL, 0, ThreadStarter, params, 0, NULL);
	return 0;
}
int UTILAPI WINThreadJoin(ThreadHandle handle)
{
	if(handle!=NULL)
	{
		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}
	return 0;
}
int UTILAPI MutexInit(MutexHandle* handle)
{
	*handle=CreateMutex(NULL, FALSE, NULL);
	return 0;
}
int UTILAPI MutexLock(MutexHandle* handle)
{
	WaitForSingleObject(*handle, INFINITE); /* == WAIT_TIMEOUT; */
	return 0;
}
int UTILAPI MutexUnlock(MutexHandle* handle)
{
	ReleaseMutex(*handle);
	return 0;
}
int UTILAPI MutexDestroy(MutexHandle* handle)
{
	CloseHandle(*handle);
	return 0;
}

int UTILAPI GetExeCurrentDir(char* buffer,unsigned int* len)
{
	char chPath[2048];
	DWORD dwLen=0;
	char* p=NULL;
	if(NULL==buffer||NULL==len)
	{
		if(NULL!=len)
			*len=0;
		return 0;
	}
	memset(chPath,0,sizeof(chPath));
	memset(buffer,0,*len);
	dwLen=GetModuleFileNameA(NULL,chPath,sizeof(chPath));
	if(0!=dwLen)
	{
		p=strrchr(chPath,'\\');
		if(p)
			*(++p)='\0';
		dwLen=(DWORD)strlen(chPath);
		if(dwLen+1<=(*len))
		{
			*len=dwLen;
			memcpy(buffer,chPath,dwLen);
			return dwLen;
		}
		else
		{
			*len=dwLen;
			return -1;
		}
	}
	*len=0;
	return 0;
}
int UTILAPI GetDllCurrentDir(const char* moduleName,char* buffer,unsigned int* len)
{
	char chPath[2048];
	DWORD dwLen=0;
	char* p=NULL;
	HMODULE hModule = NULL;
	if(NULL==moduleName||NULL==buffer||NULL==len)
	{
		if(NULL!=len)
			*len=0;
		return 0;
	}
	memset(chPath,0,sizeof(chPath));
	memset(buffer,0,*len);
	hModule=GetModuleHandleA(moduleName);
	if(NULL!=hModule)
	{
		dwLen=GetModuleFileNameA(hModule,chPath,sizeof(chPath));
		if(0!=dwLen)
		{
			p=strrchr(chPath,'\\');
			if(p)
				*(++p)='\0';
			dwLen=(DWORD)strlen(chPath);
			if(dwLen+1<=(*len))
			{
				*len=dwLen;
				memcpy(buffer,chPath,dwLen);
				return dwLen;
			}
			else
			{
				*len=dwLen;
				return -1;
			}
		}
	}
	*len=0;
	return 0;
}

int UTILAPI MakeLower(char* string)
{
	if(NULL==string)
		return -1;
	while('\0'!=*string)
	{
		if('A'<=*string&&*string<='Z')
		{
			*string-='A'-'a';
		}
		string++;
	}
	return 0;
}
int UTILAPI MakeUpper(char* string)
{
	if(NULL==string)
		return -1;
	while('\0'!=*string)
	{
		if('a'<=*string&&*string<='z')
		{
			*string+='A'-'a';
		}
		string++;
	}
	return 0;
}

