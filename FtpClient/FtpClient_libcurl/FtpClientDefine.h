#ifndef FtpClientDefine_H
#define FtpClientDefine_H
//---------------------------------------------------------------------------
enum FTPErrcode
{
	eErrSuccess,		//成功
	eErrFail,			//失败
	eErrNotInit,		//没有调用初始化接口
	eErrInvalidParam,	//非法参数


	//客户端错误
	eErrFileOpenFail,	//本地文件打开失败

	//服务端错误

	//结果
	//上传失败
	//上传中
	//上传成功


	//连接失败
	//登录失败
	//ftp不在服务时间段
	//获取ftp目录失败
	//创建ftp目录失败

	//文件写入失败
	//ftp空间不足
};


enum FTPEvent
{
	eUploadResult,		//上传结果事件

	eUploadProgress,	//上传进度事件
};
//---------------------------------------------------------------------------
typedef struct UploadResult_tag
{
	FTPErrcode	eErrcode;
	std::string	eErrstr;
	std::string	strLocalFile;
	std::string strRemoteFile;
}StructUploadResult;

typedef struct UploadProgress_tag
{
	FTPErrcode	eErrcode;
	INT64		ulNow;		//已上传字节数
	INT64		ulTotal;	//需要上传的全部字节数
	std::string	strLocalFile;
	std::string strRemoteFile;
}StructUploadProgress;
//---------------------------------------------------------------------------
/*
 * 事件回调
 * @param[out]	: 回调事件类型
 * @param[out]	: 回调结果（根据不同事件返回不同类型的结果）
 * @param[in]	: 调用SetEventCB时传入的用户参数
 */
typedef void (*FTPEventCB) (FTPEvent event, void* pData, void* pUserData);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------