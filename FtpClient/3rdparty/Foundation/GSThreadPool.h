/*
******************************************
Copyright (C): GOSUN
Filename : GSTHREADPOOL.H
Author :  zouyx
Description: �̳߳�
********************************************
*/

#ifndef _GS_H_GSTHREADPOOL_H_
#define _GS_H_GSTHREADPOOL_H_
#include "GSThread.h"
#include <list>
#include <vector>
#include <set>
#include "GxxList.hpp"

typedef  void  (GS_CALLBACK *FunPtrThreadPoolCallback)( void *pUserParam  );

class GSFDT_CLASS GSThreadPool
{
private :
	typedef struct _StruTask
	{
		FunPtrThreadPoolCallback fnOnCallback;
		void *pUserParam;
	}StruTask;
	int m_minCapacity;
	int m_maxCapacity;
	int m_idleTime; //��λ��
	GSMutex m_mutex;
	GSCond m_cond;
	std::vector<GSThread *> *m_threads;
	std::list<StruTask> *m_tasks;
	bool m_isrun;
	int m_idles; //�����߳�
	int m_nCapacity;
public:
	/*
	minCapacity �̳߳ؿ���ʱ��С�������߳���
	maxCapacity �̳߳���������߳���
	idleTime �߳̿��ж�ú�����٣� ��λ �� , <1 ��ʾ���˳�
	*/
	GSThreadPool(int minCapacity = 2,int maxCapacity = 16,int idleTime = 60);
	~GSThreadPool(void);

	//���ӻ�����߳������Ƿ���Ҫ��
	void AddCapacity(int n);


	int GetCapacity(void) const;//�����̳߳ص�ǰ�߳���


	void StopAll(void); //ֹͣ�̳߳�


	void JoinAll(void);//�ȴ������߳̽���


	bool Start(FunPtrThreadPoolCallback fnOnCallback,void *pUserParam);

	bool IsSelfThread( long threaID);

	//���صȴ����ȵ��߳���
	int GetWaits(void);

private :
	static  void GS_CALLBACK ThreadProxy(GSThread &thread, void *data);
	void ThreadEntry(GSThread &thread);

	void AddThread(void);
	GS_CLASS_NOCOPY(GSThreadPool)
};


class GSTaskPool;
class GSTaskPoolCore;

//�����̳߳� pUserParam �̵߳��������� start ����, lThreadID �߳�ID
typedef  void  (GS_CALLBACK *FunPtrRefThreadPoolCallback)( void *pUserParam, long lThreadID  );

//�����̳߳�
class GSFDT_CLASS GSRefThreadPool
{
private :	
	friend class GSTaskPool;
	friend class GSTaskPoolCore;
	class GSFDT_CLASS MyThread : public GSThread
	{
	public :
		MyThread(void);
		virtual ~MyThread(void);
		GSCond &GetCond(void);
		void Signal(void);
	};
	typedef struct GSFDT_CLASS _StruRefTask
	{
		FunPtrRefThreadPoolCallback fnOnCallback;
		void *pUserParam;
		GSTaskPoolCore *pCore;
		_StruRefTask() : fnOnCallback(NULL),pUserParam(NULL),pCore(NULL)
		{

		}
	}StruRefTask;

	typedef GxxListInner<StruRefTask> ListTask;
	typedef ListTask::MyListNode  MyTaskNode;


	int m_minCapacity;
	int m_maxCapacity;
	int m_iIdleTime; //��λ��
	GSMutex m_mutex;
	GxxListInner<MyThread*> m_listThreads;
	GxxListInner<MyThread*> m_listIdleThreads;
	ListTask m_listTask;
	ListTask m_listTaskCache;
	bool m_isRun;	
	int m_nCapacity;
	GSAtomic m_nRefs;
private :
	//�����̳߳�ʹ��
	typedef GxxListInner<void *> ListTPLTask;
	typedef ListTPLTask::MyListNode  ListTPLTaskNode;

	GSMutex m_mutexTPCache;
	ListTPLTask m_listTPLCache;


	


public :
	//�����̳߳� maxThread ����߳��� 0 ��ϵͳ����
	//idleTime ������ʱ�� ��λ��, ����߳���������ʱ����û������ ���߳̽��˳�
	static GSRefThreadPool *Create( int maxThread = 0 , int idleTime=60);
	//�����̳߳ص�ǰ�߳���
	int GetCapacity(void) const;
	void StopAll(void); //ֹͣ�̳߳�
	void JoinAll(void);//�ȴ������߳̽���
	bool Start(FunPtrRefThreadPoolCallback fnOnCallback,void *pUserParam);	
	//���صȴ����ȵ��߳���
	int GetWaits(void);
	//��������, û������ʱ�����ͷ�
	void Unref(void);
	//��������
	void Ref(void);

	static GSRefThreadPool &Global(void);
protected :
	GSRefThreadPool(int minCapacity,int maxCapacity, int idleTime );
	~GSRefThreadPool(void);


	void Clean(GSTaskPoolCore *pCore, ListTask &listObj);

	void FreeCleanResult(ListTask &listObj);


	bool Start(FunPtrRefThreadPoolCallback fnOnCallback,void *pUserParam, GSTaskPoolCore *pCore);	

	static  void GS_CALLBACK ThreadProxy(GSThread &thread, void *data);
	void ThreadEntry(GSThread &thread);

	void AddThread(void);

	static MyTaskNode *CallocTaskNodeFromCache(void *pUserData);
	static void FreeTaskNodeToCache(MyTaskNode *p,void *pUserData);

	static MyTaskNode *NewTaskNode(void *pUserData);
	static void DelTaskNode(MyTaskNode *p,void *pUserData);


	static ListTPLTaskNode *NewTPLTaskNode(void *pUserData);
	static void DelTPLTaskNode(ListTPLTaskNode *p,void *pUserData);

	ListTPLTaskNode *GetTPLTaskNode(void);
	void FreeTPLTaskNode(ListTPLTaskNode *pNode);


	GS_CLASS_NOCOPY(GSRefThreadPool)
};


class GSTaskPool;

//������лص� pTaskData �������ݣ�pUserData �û�����͸��init ���û�����
typedef void (GS_CALLBACK *FunPtrTaskPoolCallback)(void *pTaskData, void *pUserData);

//�������
class GSFDT_CLASS GSTaskPool
{
public :
	 enum
	 {
		eRetNo_SUCCESS = 0,
		eRetNo_EFAILURE = -1,
		eRetNo_ENINIT = -2, //û�г�ʼ��
		eRetNo_EFLOWOUT = -3, //���,�ﵽ�趨�����ȴ�������
		eRetNo_EPAUSE = -4,//����ͣ
		eRetNo_EINVALID = -5,
	 }EnumRetNo;
private :
	GSTaskPoolCore *m_pCore;
public :
	// lMaxWaitTasks ���ȴ��������� 0 ���޶�
	// iThreadNums ����ִ�е��߳��� > 0 
	GSTaskPool(long lMaxWaitTasks = 0, int iThreadNums =  1);
	~GSTaskPool(void);

	// Init ֻ�ܵ���һ��, Init �� Unint ���ʹ��
	// pRefThreadPool ʹ�õ��̳߳أ� ��� Ϊ NULL ��ʹ�� GSRefThreadPool::Global �̳߳�
	// fnTaskCb ����ص�����ΪNULL
	// fnFreeTaskDataCb �ͷ����ݻص������гɹ���ӵ�û�б�fnTaskCb(���Clean/������ͷ�)����
	//   �����øú����ͷŶ���
	// pUserData ͸�����ص����û�����
	// �ɹ����� 0 �� ���󷵻� EnumRetNo
	int Init(GSRefThreadPool *pRefThreadPool,
			 FunPtrTaskPoolCallback fnTaskCb,
			 FunPtrTaskPoolCallback fnFreeTaskDataCb,
			 void *pUserData);

	// �ú��������߳�ͬ���� �ȴ����еĻص�ֹͣ��ŷ���
	// bWaitComplete ���Ϊtrue ���ȴ��������������ɺ󷵻�
	// ���� ûִ�е��������ݽ������ͷŻص������������) �󷵻�
	void Uninit( bool bWaitComplete = false );

	//�ж��Ƿ��г�ʼ��
	bool IsInit(void);
	
	//�ж��Ƿ�������ã� �����ͣ��û��ʼ�����Ѿ�ж�ط��� false
	bool IsReady(void);

	//������� 	
	// pTaskData ��������
	// bPriority �ͷ���Чִ��
	// ����ȴ��������ﵽ�趨�����ֵ����������ã������� eRetNo_EFLOWOUT 
	// �ɹ����� 0 �� ���󷵻� EnumRetNo
	int Task(void *pTaskData, bool bPriority=false);

	// �ú��������߳�ͬ���� �ȴ����еĻص�ֹͣ��ŷ���
	// bWaitComplete �ο� Uninit
	void Pause( bool bWaitComplete = false);
	
	// ����ִ��
	// �ɹ����� 0 �� ���󷵻� EnumRetNo
	int Resume(void);

	//����ȴ�����������
	//���������fnFreeTaskDataCb �ص��� �û����ݽ����ûص��ͷ�
	void Clean(void);

	//���صȴ�ִ�е���������
	long GetWaitTaskCount(void);
	 
private :
	GS_CLASS_NOCOPY(GSTaskPool)
};


//�����ص�ģʽ �����  T ����
template <class T>
class CObjectTaskPool : public GSTaskPool
{
public :
	//��������ص�
	// csTaskPoo ������ж���
	//TaskData ��������
#ifdef _MSWINDOWS
	typedef typename  void (T::*FunPtrObjectCallback)( CObjectTaskPool<T> &csTaskPoo, void *TaskData );
#else
	typedef  void (T::*FunPtrObjectCallback)( CObjectTaskPool<T> &csTaskPoo, void *TaskData );
#endif
private :
	T *m_pFunOwner;
	FunPtrObjectCallback m_fnTaskCb;
	FunPtrObjectCallback m_fnFreeTaskDataCb;
	void *m_pUserData;
public :
	//�ο�GSTaskPool
	CObjectTaskPool(long iMaxWaitTasks = 0, int iThreadNums =  1)
		: GSTaskPool(iMaxWaitTasks, iThreadNums)
		,m_pFunOwner(NULL)
		,m_fnTaskCb(NULL)
		,m_fnFreeTaskDataCb(NULL)
		,m_pUserData(NULL)
	{

	}

	virtual ~CObjectTaskPool(void)
	{
		Uninit(false);
	}

	// pRefThreadPool �ο� GSTaskPool::Init
	//  pFunOwner �ص�����ָ��
	// fnTaskCb   ����ص�
	// fnFreeTaskDataCb  �ͷ��������ݻص� �ο� GSTaskPool::Init
	int Init(GSRefThreadPool *pRefThreadPool,
			T *pFunOwner,
			FunPtrObjectCallback fnTaskCb,
			FunPtrObjectCallback fnFreeTaskDataCb)
	{
		m_pFunOwner = pFunOwner;
		m_fnTaskCb = fnTaskCb;
		m_fnFreeTaskDataCb = fnFreeTaskDataCb;
		return GSTaskPool::Init(pRefThreadPool,DoTaskEventProxy,m_fnFreeTaskDataCb ? DoFreeTaskDataEventProxy : NULL, this );
	}


	//�����û�˽������ 
	void SetUserData(void *pData)
	{
		m_pUserData = pData;
	}
	//����  SetUserData �趨ֵ
	void *GetUserData(void)
	{
		return m_pUserData;
	}
private :
	static void GS_CALLBACK DoTaskEventProxy(void *pTaskData, void *pUserData)
	{
		CObjectTaskPool *p = (CObjectTaskPool*)pUserData;
		(p->m_pFunOwner->*(p->m_fnTaskCb))(*p, pTaskData );
	}
	static void GS_CALLBACK DoFreeTaskDataEventProxy(void *pTaskData, void *pUserData)
	{
		CObjectTaskPool *p = (CObjectTaskPool*)pUserData;
		if( p->m_fnFreeTaskDataCb )
		(p->m_pFunOwner->*(p->m_fnFreeTaskDataCb))(*p, pTaskData );
	}
};

/*
*********************************************************************
*
*@brief : ʹ�����̳߳ع��ܵĳ��� ���������˳�ǰ����Դ�ͷ�
*
*********************************************************************
*/
GSFDT_API void GSThreadPoolGlobalCleanup(void);

/*
��ʾ��ʼ���̳߳ع���
*/
GSFDT_API void GSThreadPoolGlobalInit(void);

#endif //end _GS_H_GSTHREADPOOL_H_
