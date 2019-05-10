#ifndef MC_Ftp_H
#define MC_Ftp_H
//---------------------------------------------------------------------------
#ifndef       FtpClientEventCB
typedef void (__stdcall *FtpClientEventCB)(const char* json, unsigned int len, void* context);
#endif
//---------------------------------------------------------------------------
class MC_Ftp
{
public:
	MC_Ftp(void);
	~MC_Ftp(void);

	bool	Init		(void);
	bool	UnInit		(void);

	bool	SetEventCB	(FtpClientEventCB vCb, void* vUserData);

};
//---------------------------------------------------------------------------
#endif
