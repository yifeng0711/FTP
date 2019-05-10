#ifndef FtpClientDefine_H
#define FtpClientDefine_H
//---------------------------------------------------------------------------
enum FTPErrcode
{
	eErrSuccess,		//�ɹ�
	eErrFail,			//ʧ��
	eErrNotInit,		//û�е��ó�ʼ���ӿ�
	eErrInvalidParam,	//�Ƿ�����


	//�ͻ��˴���
	eErrFileOpenFail,	//�����ļ���ʧ��

	//����˴���

	//���
	//�ϴ�ʧ��
	//�ϴ���
	//�ϴ��ɹ�


	//����ʧ��
	//��¼ʧ��
	//ftp���ڷ���ʱ���
	//��ȡftpĿ¼ʧ��
	//����ftpĿ¼ʧ��

	//�ļ�д��ʧ��
	//ftp�ռ䲻��
};


enum FTPEvent
{
	eUploadResult,		//�ϴ�����¼�

	eUploadProgress,	//�ϴ������¼�
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
	INT64		ulNow;		//���ϴ��ֽ���
	INT64		ulTotal;	//��Ҫ�ϴ���ȫ���ֽ���
	std::string	strLocalFile;
	std::string strRemoteFile;
}StructUploadProgress;
//---------------------------------------------------------------------------
/*
 * �¼��ص�
 * @param[out]	: �ص��¼�����
 * @param[out]	: �ص���������ݲ�ͬ�¼����ز�ͬ���͵Ľ����
 * @param[in]	: ����SetEventCBʱ������û�����
 */
typedef void (*FTPEventCB) (FTPEvent event, void* pData, void* pUserData);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------