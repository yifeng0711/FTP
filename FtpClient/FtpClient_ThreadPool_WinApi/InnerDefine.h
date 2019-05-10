#ifndef InnerDefine_H
#define InnerDefine_H
//---------------------------------------------------------------------------
typedef struct TaskInfo_tag
{
	std::string	strFileName;
	
	TaskInfo_tag()
	{
		strFileName = "";
	}

	~TaskInfo_tag()
	{
		Release();
	}

	void Release(void)
	{
		strFileName = "";
	}

}TASKINFO_tag;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#endif