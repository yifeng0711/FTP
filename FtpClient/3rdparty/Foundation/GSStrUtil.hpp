/*
******************************************
Copyright (C): GOSUN
Filename : GSSTRUTIL.HPP
Author :  zouyx
Description: 字符串操作函数
********************************************
*/

#ifndef _GS_H_GSSTRUTIL_H_
#define _GS_H_GSSTRUTIL_H_


#include "GSTypes.h"
#include "GSDefs.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>

#include <iomanip>



#ifndef _MSWINDOWS_
#include <stdarg.h>
#include <string.h>
#endif


#define GSString std::string

#define GSStringStream std::stringstream

/*
*********************************************************************
*
*@brief : 字符串操作功能
*
*********************************************************************
*/



namespace  GSStrUtil
{


	// 删除字符串左边空白
	INLINE static GSString  TrimLeft(const GSString& str)
	{
		std::string t = str;
		t.erase(0, t.find_first_not_of(" \t\n\r"));
		return t;
	}

	// 删除字符串右边空白
	INLINE static GSString TrimRight(const GSString& str)
	{
		std::string t = str;
		t.erase(t.find_last_not_of(" \t\n\r") + 1);
		return t;
	}

	// 删除字符串左右两边空白
	INLINE static GSString Trim(const GSString& str)
	{
		std::string t = str;
		t.erase(0, t.find_first_not_of(" \t\n\r"));
		t.erase(t.find_last_not_of(" \t\n\r") + 1);
		return t;
	}

	// 转换为小写字符串
	INLINE static GSString ToLower(const GSString& str) 
	{
		std::string t = str;
		std::transform(t.begin(), t.end(), t.begin(), ::tolower);
		return t;
	}

	// 转换为大写字符串
	INLINE static GSString ToUpper(const GSString& str) 
	{
		std::string t = str;
		std::transform(t.begin(), t.end(), t.begin(), ::toupper);
		return t;
	}

	// 分解字符串
	INLINE static  void Split(std::vector<GSString> &csResult, const GSString& str, const GSString& delimiters)
	{
		std::string::size_type iOffset = 0;
		GSString strToken;
		for(;;)
		{

			std::string::size_type i = str.find_first_not_of(delimiters, iOffset);
			if (i == std::string::npos) {
				iOffset = str.length();
				return;
			}

			// 查找标识结束位置
			std::string::size_type j = str.find_first_of(delimiters, i);
			if (j == std::string::npos) {
				strToken = str.substr(i);
				iOffset = str.length();
				csResult.push_back(strToken);
			}
			else
			{
				strToken = str.substr(i, j - i);
				iOffset = j;
				csResult.push_back(strToken);
			}
		}
	}

	//不分大小写的比较
	INLINE static bool EqualsIgnoreCase( const GSString& strSrc, const GSString& strDest)
	{
		return ToLower(strSrc) == ToLower(strDest);
	}


	// 字符串类型转换模板函数
	// 字符串类型转换模板函数
	template<class T> T ToNumber(const GSString& str);
	template<class T> T ToHexNumber(const GSString& str);
	template<class T> GSString ToString(const T value);
	template<class T> GSString ToString(const T value, int fixedPrecision);
	template<class T> GSString ToHexString(const T value);



	/*
	*********************************************
	Function : ToNumber
	DateTime : 2011/1/20 15:23
	Description : 将十进制字符串转换为数值
	Input :
	Output :
	Return :
	Note :
	*********************************************
	*/
	template<class T> 
	T ToNumber(const GSString& str)
	{
		T value;
		std::istringstream iss(str.c_str());
		iss >> value;
		return value;
	}

	/*
	*********************************************
	Function :
	DateTime : 2011/1/20 15:23
	Description : 将十六进制字符串转换为数值
	Input :
	Output :
	Return :
	Note :
	*********************************************
	*/
	template<class T> 
	T ToHexNumber(const GSString& str) 
	{
		T value;
		std::istringstream iss(str.c_str());
		iss >> std::hex >> value;
		return value;
	}


	template<class T> 
	GSString ToString(const T value) 
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	template<class T> 
	GSString ToString(const T value, int fixedPrecision) 
	{
		std::ostringstream oss;
		oss <<std::fixed<<std::setprecision(fixedPrecision)<< value;
		return oss.str();
	}


	template<class T> 
	GSString ToHexString(const T value) 
	{
		std::ostringstream oss;
		oss << "0x" << std::hex << value;
		return oss.str();
	}



	template<class T> 
	int CheckValueRange(T &vValue, const T vMin, const T vMax)
	{
		if( vValue<vMin )
		{
			vValue = vMin;
			return -1;
		}
		else if( vValue > vMax )
		{
			vValue = vMax;
			return 1;
		}
		return 0;
	}

#define MAX_STRING_LENGTH  (100<<20)

	INLINE static  bool VFormat(GSString &strOString,const char *czFormat, va_list ap )
	{
		char sBuffer[256];
		va_list apStart;
		char *pBuffer;
		int n, size = 256; 


		strOString.clear();
#ifdef _MSWINDOWS_
		apStart = ap;       
#else
		va_copy(apStart,ap);
#endif
		pBuffer = sBuffer;
		while (pBuffer) {           
#ifdef _MSWINDOWS_
			ap = apStart;
#else
			va_copy(ap,apStart);
#endif
			n = gs_vsnprintf(pBuffer, size, czFormat, ap);          
			if (n > -1 && n < size )
			{
				//成功格式化
				//pBuffer[n] = '\0';
				strOString =  pBuffer;
				if( pBuffer!=sBuffer )
				{      
					::free(pBuffer);
				}
				return true;
			}
			if( pBuffer!=sBuffer )
			{      
				::free(pBuffer);
			}
			pBuffer = NULL;
			size *= 2;
			if( size>MAX_STRING_LENGTH )
			{
				GSAssert(0);
				return false;
			}

			pBuffer = (char*) ::malloc(size);
			GSAbort(pBuffer!=NULL);
		}
		if( pBuffer && pBuffer!=sBuffer  )
		{
			::free(pBuffer);
		}
		return false;
	}



	INLINE static  bool Format(GSString &strOString, const char* czFormat, ...)
	{
		bool bRet;
		va_list ap;
		va_start(ap, czFormat);
		bRet = GSStrUtil::VFormat(strOString, czFormat, ap);
		va_end(ap);
		return  bRet;

	}

	INLINE static  bool AppendWithFormat(GSString &strIOString, const char* czFormat, ...)
	{
		bool bRet; 
		GSString strTemp;
		va_list ap;
		va_start(ap, czFormat);
		bRet = GSStrUtil::VFormat(strTemp, czFormat, ap);
		va_end(ap);

		if( bRet )
		{
			strIOString += strTemp;
		}
		return bRet;
	}



	INLINE static void FilenameFormat( GSString &strFilename )
	{
		//格式化文件名
#ifdef _MSWINDOWS_
		const char cDelimiters =  '\\';
#else
		const char cDelimiters =  '/';
#endif
		GSString strTemp = strFilename;
		char *pStr = (char*)strTemp.c_str();
		char *p = pStr;;
		while( *p!='\0' )
		{
			if( *p == '\\' || *p == '/' )
			{
				*p = cDelimiters;
			}
			p++;
		}
		p = pStr;
		while(p[0]!='\0' && p[1] !='\0' )
		{
			if( p[0]==cDelimiters && p[1]==cDelimiters )
			{
				for( int i = 1; p[i] !='\0' ; i++ )
				{
					p[i] = p[i+1];
				}
			}
			else
			{
				p++;
			}
		}
		strFilename = pStr;
	}

	INLINE static void FilenameFormatLinux( GSString &strFilename )
	{
		//格式化文件名
		const char cDelimiters =  '/';
		GSString strTemp = strFilename;
		char *pStr = (char*)strTemp.c_str();
		char *p = pStr;;
		while( *p!='\0' )
		{
			if( *p == '\\' || *p == '/' )
			{
				*p = cDelimiters;
			}
			p++;
		}
		p = pStr;
		while(p[0]!='\0' && p[1] !='\0' )
		{
			if( p[0]==cDelimiters && p[1]==cDelimiters )
			{
				for( int i = 1; p[i] !='\0' ; i++ )
				{
					p[i] = p[i+1];
				}
			}
			else
			{
				p++;
			}
		}
		strFilename = pStr;
	}

	// 字符串替换
	INLINE static bool StringReplace( GSString& strMain, const GSString& strFrom, const GSString& strTo )
	{
		if ( strFrom.empty() )
			return false;

		GSString::size_type pos = 0;
		GSString::size_type fromLen = strFrom.size();
		GSString::size_type toLen = strTo.size();

		pos = strMain.find(strFrom, pos); 

		while ((pos != GSString::npos))
		{
			strMain.replace(pos, fromLen, strTo);
			pos=strMain.find(strFrom, (pos+toLen));
		}

		return true;
	}
};


#endif //end _GS_H_GSSTRUTIL_H_
