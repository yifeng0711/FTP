/*
******************************************
Copyright (C): GOSUN
Filename : GSTIME.H
Author :  zouyx
Description: 
********************************************
*/

#ifndef _GS_H_GSTIME_H_
#define _GS_H_GSTIME_H_
#include "GSFdtDefs.h"
#include <time.h>
#include "GSMutex.h"



//����ö�ٶ���, 0 ��ʾ�����գ� 6 ��ʾ������
typedef enum 
/// Symbolic names for week day numbers (0 to 6).
{
	SUNDAY = 0,
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6
}DaysOfWeek;


/*
********************************************************************
��ע��
����    :    GSDateTime
����    :    zouyx
������  :    ʱ�����ڲ�����
*********************************************************************
*/


typedef enum
{
	//���ڵĸ�ʽ����ʽ
	DT_FORMAT_D = 0,		//   yyyy-mm-dd   ����		
	DT_FORMAT_DF = 1,	    //   mm/dd/yyyy
	DT_FORMAT_DN = 2,       //yyyymmdd
	DT_FORMAT_DM = 3,		//  mm-dd-yyyy ����
	DT_FORMAT_DMF = 4,		//  mm/dd/yyyy ����



	//ʱ��ĸ�������ʽ
	DT_FORMAT_T = 50,	 //   hh24:mm:ss
	DT_FORMAT_TM = 51,	 //   hh24:mm:ss.millisecond
	DT_FORMAT_TN = 52,	 //   hh24miss

}EnumDTFormat;


typedef Int64 GSTimeVal;


class GSFDT_CLASS GSDateTime
{
protected :
	int m_year;  //�� 0 to 9999
	int m_month; //�� 1 - 12
	int m_dayOfMonth;   //�� 1 - 31
	int m_hour;  //ʱ  24Сʱ�� 0 - 23
	int m_minute; //��  0-59
	int m_second;  //�� 0-59
	int m_millisecond; //���� 0-999
	int m_dayOfWeek; // ���� 0-6   0:������, 1:����һ ... 6:������
//	GSTimeVal m_epochTv; //���ص� Epoch UTC ʱ�� 
	char m_buf[64];
public :	
	GSDateTime(void);
	GSDateTime(const time_t &tv);
	GSDateTime(const GSDateTime &csDest);
	GSDateTime(int year, int month, int day, 
				int hour = 0, int minute = 0, int second = 0, int millisecond=0);

	~GSDateTime(void);


	bool Assign(int year, int month, int day, int hour = 0,
		        int minute = 0, int second = 0, int millisecond = 0);

	INLINE int Year(void) const
	{
		return m_year;
	}

	INLINE int Month(void) const
	{
		return m_month;
	}

	INLINE int MonthDay(void) const
	{
		return m_dayOfMonth;
	}

	INLINE int Hour(void) const
	{
		return m_hour;
	}

	INLINE int Minute(void) const
	{
		return m_minute;
	}

	INLINE int Second(void) const
	{
		return m_second;
	}

	INLINE int Millisecond(void) const
	{
		return m_millisecond;
	}

	INLINE int WeekDay(void) const
	{
		return m_dayOfWeek;
	}


	bool IsValid(void) const;   //�ж�ʱ���Ƿ���Ч

	bool GetLocalTime(void); //����ϵͳ����ʱ��
	bool SetLocalTime(void); //����ϵͳ����ʱ��


	time_t Timestamp(void) const;

	GSTimeVal GetEpochTime(void) const; //����UTC ʱ�䣬�� 1970-01-01 00:00:00 ������ ���� Timestamp
	bool   SetEpochTime(GSTimeVal tv);  //ʹ��UTC ʱ���ʼ���ṹ

	bool AddYears( int nYears);	//����
	bool AddMonths( int nMonths); //����
	bool AddDays( int nDays);     //����
	bool AddHours( int nHours);   //��Сʱ
	bool AddSeconds( int nSeconds); //����
	bool AddMilliseconds( int nMilliseconds); //�Ӻ���

	bool SubYears( int nYears);	//����
	bool SubMonths( int nMonths); //����
	bool SubDays( int nDays);    //����
	bool SubHours( int nHours);  //��Сʱ
	bool SubSeconds( int nSeconds); //����
	bool SubMilliseconds( int nMilliseconds); //������

	
	bool operator<(const GSDateTime &dest) const;
	bool operator>(const GSDateTime &dest) const;
	bool operator==(const GSDateTime &dest) const;
	bool operator!=(const GSDateTime &dest) const;


	bool IsAM() const;
	/// Returns true if hour < 12;

	bool IsPM() const;
	/// Returns true if hour >= 12.


	
	//���� yyyy-mm-dd hh24:mi:ss
	const char *Format(char *buf=NULL, int bufsize = 0) const;
	//���ظ�ʽ���ַ���,  delimiters �м�ķָ��
	const char *Format( EnumDTFormat fmt, char *buf, int bufsize = 0 ) const;
	const char *Format( EnumDTFormat fmtfirst, EnumDTFormat fmtsecond, const char *delimiters=" ",
						char *buf = NULL, int bufsize = 0 ) const;

	//�� yyyy-mm-dd hh24:mi:ss
	bool SetFromFormat(const char *datetime );
	//�ɸ�ʽ��ֵ�����ַ���
	bool SetFromFormat( const char *datetime, EnumDTFormat fmt );
	bool SetFromFormat( const char *datetime, EnumDTFormat fmtfirst,EnumDTFormat fmtsecond,
						const char *delimiters = " " );


	static UInt32   GetTickCount(void); //��ȡ�Ӳ���ϵͳ�����������������ĺ����������شӲ���ϵͳ�����������������ĺ�����, 49.7 ���ѭ��

	static UInt64  GetPerformanceCounter(void);    // �߾���ʱ���ȡ�����غ�����

	static UInt32   CountElapsed(UInt32 nowTick, UInt32 oldTick); //��������TICK ��ʱ�����ţ� ���غ���

	//����ϵͳ UTC ʱ�䣬�� 1970-01-01 00:00:00 ��ʱ�����ŵĺ�����, ���󷵻� MAX_UINT64
	static UInt64   GetEpochMilliseconds(void);

};


//��ʱ��
class GSFDT_CLASS GSTimeMeter
{
private :
	UInt64 m_lasttv;
public :
	GSTimeMeter(void);
	~GSTimeMeter(void);

	//���ü�ʱ��
	void Reset(void);
	void Reset(UInt64 iLastTv);
	UInt64 GetElapsed(void) const; //�������ŵĺ�����
};

//��ʱ��

typedef struct _StruGSTimerRunThread StruGSTimerRunThread;
class GSTimerGlobal;

class GSFDT_CLASS GSTimer
{
public :
	//��ʱ���ص�
	typedef void (GS_CALLBACK *FunPtrTimerCallback)(GSTimer &timer, void *pTimerParam);
private:
	friend class GSTimerGlobal;
	long m_timerID;	
	bool m_isStarted;
	int m_interval;
	FunPtrTimerCallback m_fncb;
	void *m_pTimerParam;
	GSMutex m_mutex;
	StruGSTimerRunThread *m_pRunThread;
	GSTimeMeter m_csMeter;
	
public :
	//��ʼ����ʱ��ģ��, nThreadNums �߳����� ���Ϊ 1 ����timerֻʹ��һ���̻߳ص��� ������timer ʹ�õ��Ƕ���̲߳����ص�
	static bool ModuleInit( int nThreadNums = 0);
	static void ModuleUninit(void);


	//nInterval ����� ��λ����
	GSTimer(long timerID);
	~GSTimer(void);

	//��ʼ���� ֻ�ܳ�ʼ��һ��
	bool Init(int nInterval, GSTimer::FunPtrTimerCallback fnCb, void *pTimerParam);

	long GetTimerID(void) const;


	//��ʼ�ص� �� isRunImmediately �Ƿ�����ִ��һ��
	bool Start( bool isRunImmediately );
	//bWait ������ڵ��ȣ� �Ƿ�ȴ���ǰ�������
	void Stop(bool bWait = true);

private :
	void RunCallback(void);
};



#endif //end _GS_H_GSTIME_H_
