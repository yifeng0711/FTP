/*
******************************************
Copyright (C): GOSUN
Filename : GSDEFS.H
Author :  zouyx
Description: 通用宏定义
********************************************
*/

#ifndef _GS_H_GSDEFS_H_
#define _GS_H_GSDEFS_H_

#include "GSTypes.h"
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#ifdef _MSWINDOWS_
#include <process.h>
#include <string.h>
#else
#include <string.h>
#endif

#define gs_sprintf sprintf

#define gs_fprintf fprintf

#define gs_printf printf

#ifdef _MSWINDOWS_
//#define gs_snprintf _snprintf
#define  gs_snprintf(xbuf, xbufsize , xfmt, ... ) _snprintf_s(xbuf,xbufsize, _TRUNCATE,xfmt, __VA_ARGS__)

#ifndef bzero
# define bzero(_m, _s) memset(_m, 0, _s)
#endif

#ifndef gs_vsnprintf
# define gs_vsnprintf(xbuf, xbufsize , xfmt, ... ) _vsnprintf_s(xbuf,xbufsize, _TRUNCATE,xfmt, __VA_ARGS__)
#endif

#ifndef strcasecmp
# define strcasecmp _stricmp 
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp 
#endif

#pragma warning(disable:4996)
#define gs_strncpy(_dest,_src, _ncount ) strncpy(_dest,_src, _ncount)
#define gs_strdup _strdup

#else
#define gs_snprintf snprintf
#define gs_vsnprintf vsnprintf
#define gs_strncpy strncpy
#define gs_strdup strdup
#endif





static void MyAssert(char *s)
{
	gs_printf("%s\n", s);

#ifdef _MSWINDOWS_
	//DebugBreak();
	assert(0);
#else
	(*(int*)0) = 0;
#endif

}

static void MyAbort(char *s)
{
	gs_printf("%s\n", s);
#ifdef _MSWINDOWS_
	abort();
#else
	(*(int*)0) = 0;
#endif
}


#define kAssertBuffSize 256

#ifdef _DEBUG

#define GSAssert(condition)    {                              \
	\
	if (!(condition))                                       \
{                                                       \
	char s[kAssertBuffSize];                            \
	s[kAssertBuffSize -1] = 0;                          \
	gs_snprintf (s,kAssertBuffSize -1, "_Assert: %s, %s, %d", #condition, __FILE__, __LINE__ ); \
	MyAssert(s);                                        \
}   }


#define GSAssertV(condition,errNo)    {                                   \
	if (!(condition))                                                   \
{                                                                   \
	char s[kAssertBuffSize];                                        \
	s[kAssertBuffSize -1] = 0;                                      \
	gs_snprintf( s,kAssertBuffSize -1, "_AssertV: %s, %s, %d (%d)", #condition, __FILE__, __LINE__, errNo );    \
	MyAssert(s);                                                    \
}   }

#else
#define GSAssert(condition) do{ if(condition){break;} } while(0)
#define GSAssertV(condition,errNo) do{ if(condition){break;} } while(0)
#endif


#define GSAbort(condition)    {                              \
	\
	if (!(condition))                                       \
{                                                       \
	char s[kAssertBuffSize];                            \
	s[kAssertBuffSize -1] = 0;                          \
	gs_snprintf (s,kAssertBuffSize -1, "_Abort: %s, %s, %d", #condition, __FILE__, __LINE__ ); \
	MyAbort(s);                                        \
}   }


#define GSAbortV(condition,errNo)    {                                   \
	if (!(condition))                                                   \
{                                                                   \
	char s[kAssertBuffSize];                                        \
	s[kAssertBuffSize -1] = 0;                                      \
	gs_snprintf( s,kAssertBuffSize -1, "_AbortV: %s, %s, %d (%d)", #condition, __FILE__, __LINE__, errNo );    \
	MyAbort(s);                                                    \
}   }


#define GSWarn(condition) {                                       \
	if (!(condition))                                       \
	gs_printf( "_Warn: %s, %s, %d\n",#condition, __FILE__, __LINE__ );     }                                                           

#define GSWarnV(condition,msg)        {                               \
	if (!(condition))                                               \
	gs_printf ("_WarnV: %s, %s, %d (%s)\n",#condition, __FILE__, __LINE__, msg );  }                                                   

#define GSWarnVE(condition,msg,err)  {                           		\
	if (!(condition))                                               \
	gs_printf ("_WarnVE: %s, %s, %d (%s [err=%d])\n",#condition, __FILE__, __LINE__, msg, err );  }


#define GS_CLASS_NOCOPY(classname) \
private : \
classname( const classname &dest ); \
classname &operator= (const  classname &dest);

#ifndef GS_ASSERT_RET_VAL
# define GS_ASSERT_RET_VAL(_condition, _val) if(!(_condition)){GSAssert(0); return _val;}
#endif

#ifndef GS_ASSERT_RET
#define GS_ASSERT_RET(_condition) if(!(_condition)){ GSAssert(0); return;}
#endif

#ifndef MIN
#define MIN(_a , _b) (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef MAX
#define MAX(_a , _b) (((_a) > (_b)) ? (_a) : (_b))
#endif



#define GS_ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))


#define _GST(x) x


#endif //end _GS_H_GSDEFS_H_
