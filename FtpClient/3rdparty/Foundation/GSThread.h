/*
******************************************
Copyright (C): GOSUN
Filename : GSTHREAD.H
Description: �̶߳���
********************************************
*/

#ifndef _GS_H_GSTHREAD_H_
#define _GS_H_GSTHREAD_H_

#include "GSFdtDefs.h"
#include "GSMutex.h"


class GSThread;

/*******************************************************************************
����˵�����߳��࣬��װ�̳߳��û�������
*******************************************************************************/
typedef  void  (GS_CALLBACK *FunPtrThreadCallback)(GSThread &thread, void *pThreadData );

class GSFDT_CLASS GSThread
{
protected :
#if  defined(_MSWINDOWS_)
	HANDLE m_hThread;
#else
	pthread_t m_hThread;
#endif

	int m_flags;
	GSMutex m_mutex;
	GSCond m_cond;
	FunPtrThreadCallback m_fnUser;
	void *m_pFnThreadData;

	void *m_pUserData;
protected :
	long m_threadId;
public:
	GSThread(void);
	virtual ~GSThread(void);

	bool Start(FunPtrThreadCallback fnOnEvent, void *pThreadData=NULL);	 //��ʼ�̣߳��ɹ�����TRUE,ʧ�ܷ���FALSE	

	void Stop(void);	//ֹͣ�߳�

	bool TestExit(void) ;	//�ж��߳��Ƿ��˳������̺߳�����ѭ���е��ã�ִ����ͣ�ͻ��Ѳ���	

	void Join(void);	//�ȴ��߳̽���

	void Suspend(void);		//�߳���ͣ

	bool Resume(void);		//�����߳�,�ɹ�����TRUE,ʧ�ܷ���FALSE	

	bool IsPause(void) ;    //�߳��Ƿ�����ͣ
	bool IsRunning(void) ;	//�����߳�����״̬��TRUEΪ�������У�FALSEΪδ����	

	long GetThreadID(void); //�����߳�ID ��

	void SetUserData(void *pUserData);
	void *GetUserData(void);


	static long  GetCurrentThreadID(void); //���ص�ǰ�߳�ID

private :
	void JoinInner(void);

	void Detach(void);

	GS_CLASS_NOCOPY(GSThread)
};

#endif //end _GS_H_GSTHREAD_H_
