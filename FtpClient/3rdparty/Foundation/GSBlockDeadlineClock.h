/*
******************************************
Copyright (C): GOSUN ���� 2017
Filename : GSBlockDeadlineClock.h
Author : xinxz 
Description: �߾���������
********************************************
*/

 
#ifndef __GSBlockDeadlineClock_H__
#define __GSBlockDeadlineClock_H__
#include "GSTypes.h"
#include "GSFdtDefs.h"
#include <Windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#include <process.h>
#include <time.h>
#include "GxxList.hpp"


class GSFDT_CLASS GSBlockDeadlineClock
{
public:
    GSBlockDeadlineClock();
    ~GSBlockDeadlineClock();

    /**
     * Method		Reset
     * @brief		���ü�ʱ��ʼʱ��
     * @return		void
     */
    void	Reset();
    /**
     * Method		GetElapsed
     * @brief		��ȡ��Resetʱ�̿�ʼ����ǰʱ�̵�����ʱ��
     * @return		UInt64	  �������ŵ�ʱ�䣬��λms
     */
    UInt64	GetElapsed();
    /**
     * Method		BlockToAwaked
     * @brief		������ǰ�̣߳�ֱ����Reset��ʼ����ǰʱ�̵��ۼ�ʱ����ڻ����ulMisiseconds
     * @param[in]	ulMiliseconds   ����Resetʱ�̵��ۼ�ʱ�䣬��λms
     * @return		bool  ��ulMisisecondsС���ۼ�ʱ�䣬����������false,���򷵻�true
     */
    bool	BlockToAwaked(UInt64 ulMiliseconds);

    /**
     * Method		Init
     * @brief		��ʼ������
     * @return		bool	������false����ʾ������֧��
     */
    static bool Init();

    /**
     * Method		UnInit
     * @brief		����ʼ������
     * @return		bool
     */
    static bool UnInit();

protected:
private:

    bool	SuspendCurrentThread(UInt64 milliSecond);
    static	void ResumeTimeoutThread();

    static	void CALLBACK TimerFunc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    static	bool InitialWorkThread();

private:

    const static int	m_nClockStep = 1;

    static CRITICAL_SECTION	m_critical;
    static MMRESULT		    m_timerID;
    static double			m_dCountPerMilliSecond;
    static bool				m_bInitedOK;

    LARGE_INTEGER		    m_refScale;
    struct WaitedHandle
    {
        HANDLE					threadHandle;
        Int64					elapsed;//��ʱʱ��
    } ;

    typedef GxxListInner< WaitedHandle* > HandleListType;
    static HandleListType	waitList;
};


#endif // __GSBlockDeadlineClock_H__
