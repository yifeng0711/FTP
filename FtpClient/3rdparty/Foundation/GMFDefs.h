

/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSRCDFILEDEFS.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/22 11:55
Description: ���ļ�����¼���ļ��ṹ����ظ�ʽ�� ע��ο� <GSMediaDefs.h>
********************************************
*/

#ifndef GSP_GMFDEFS_DEF_H
#define GSP_GMFDEFS_DEF_H

#include "GSMediaDefs.h"

#ifdef _WIN32
#pragma pack( push,1 )
#endif

/*
****************************************
brief : �洢�ļ���ʽ
****************************************

==========�ļ����ṹ����======================
�ļ�������ṹ  ��������(����(21Bytes).������(�8Bytes)

���ֽṹ:
¼��ʱ��(YYYYMMHHMMSS)-ID(XXXXXXXX)

��:  201007230101-F1234567.bmp


========�ļ��ṹ����  ֻ������Ƶ¼��ʹ�ã� ����ͼƬֱ�Ӵ�Ϊ��׼��ʽ=====================

�ļ�ͷ | ֡�ṹ����(StruGSFrameHeader)

�ļ�ͷ�ṹ:

ħ����(4Bytes)| ͷ����(2Bytes) | �ļ�ͷ�汾(1Byte) | ý�����Ը���(1Byte) | ¼��ʱ�䣨YYYYMMDDHHMMSS) | ý��ͨ��0����(StruGSMediaDescri)|...|ý��ͨ��n����(StruGSMediaDescri)
*/

#define GS_RCV_FILE_MAGIC  (('g'<<24)|('s'<<14)|(0xaf<<16)|(0xe3))
//#define GS_RCV_VERSION     0x00
#define GS_RCV_VERSION     0x02


#define GS_GMF_V2 0x02
#define GS_GMF_V0 0x00

// ֡������ʼ��־
#define GS_INDEX_BEGIN_MARK (('g'<<24)|('b'<<16)|('i'<<8)|('g'))
// ֡����������־
#define GS_INDEX_END_MARK (('d'<<24)|('e'<<16)|('i'<<8)|('g'))



typedef struct _StruIFrameIndex
{
	UInt32 iTimeStamp;      // ʱ��� (��1970��1��1��0ʱ0��0�뵽��ʱ������)
	UInt32 iOffset;         // ��Ӧ�ļ���ƫ����
} GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIndex;

//���������ڵ���Ϣ
typedef struct _StruIFrameIdxEndNode
{
	UInt32 iIdxStartOffset; //������Ϣ����ļ���ͷ��ƫ��, ָ��  GS_INDEX_BEGIN_MARK ��ͷ
	UInt32 iEndMarsk; //�������־ ���� GS_INDEX_END_MARK

}GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIdxEndNode;

// V2.0 ��������
typedef struct _StruIFrameIndexV2
{
	UInt64 iTimeStamp;      // ʱ��� (��1970��1��1��0ʱ0��0�뵽��ʱ������)
	UInt64 iOffset;         // ��Ӧ�ļ���ƫ����
} GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIndexV2;

//V2.0  ���������ڵ���Ϣ
typedef struct _StruIFrameIdxEndNodeV2
{
	UInt64 iIdxStartOffset; //������Ϣ����ļ���ͷ��ƫ��, ָ��  GS_INDEX_BEGIN_MARK ��ͷ
	UInt64 iEndMarsk; //�������־ ���� GS_INDEX_END_MARK
}GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIdxEndNodeV2;

/* 
�ļ�ͷ | ֡�ṹ���� | ... | ֡�ṹ���� | ����֡ͷ | ֡������ʼ��־��GS_INDEX_BEGIN_MARK��| ������Ϣ��StruIFrameIndex��|...| ������Ϣ��StruIFrameIndex��| StruIFrameIdxEndNode
*/

/* V2 �汾
�ļ�ͷ | ֡�ṹ���� | ... | ֡�ṹ���� | ����֡ͷ | ֡������ʼ��־��GS_INDEX_BEGIN_MARK��| ������Ϣ��StruIFrameIndexV2��|...| ������Ϣ��StruIFrameIndexV2��| StruIFrameIdxEndNodeV2
*/


/*
=====�����ļ������ļ�ʱ��ͷ �ļ���Ϊ¼���ļ��� ��չ�� .gsidx
�ļ�ͷ�ṹ:
ħ����(4Bytes)| ͷ����(2Bytes) | �ļ�ͷ�汾(1Byte)| ����(1Byte) | ����֡ͷ(StruGSFrameHeader?) | ֡������ʼ��־��GS_INDEX_BEGIN_MARK��| ������Ϣ��StruIFrameIndex?��|...| ������Ϣ��StruIFrameIndex?��| StruIFrameIdxEndNode?
*/
#define GS_IDX_FILE_MAGIC  (('g'<<24) | ('i'<<14) | ('d'<<16) | 'x' )
typedef struct _StruGSIdxFHeader
{
	UInt32 iMagic;       //ħ���� Ϊ GS_IDX_FILE_MAGIC
	UInt16 iHeaderSize;   //ͷ����
	UInt8  iVersion;      //ͷ�汾 ���� //GS_GMF_V2
	UInt8  iReserver; //����
}GS_MEDIA_ATTRIBUTE_PACKED StruGSIdxFHeader;



typedef struct _StruGMFHeader
{
    UInt32 iMagic;       //ħ���� Ϊ GS_RCV_FILE_MAGIC
    UInt16 iHeaderSize;   //ͷ����
    UInt8  iVersion;      //ͷ�汾                                                                                                                                                                                                                                                                                                                                                                                                                               
    UInt8  iChannel;       //ý��ͨ����  aMediaDescri �ĸ���
    unsigned char aTv[16];   //¼��ʱ��
    StruGSMediaDescri aMediaDescri[1]; //ý��ͨ������
}GS_MEDIA_ATTRIBUTE_PACKED StruGMFHeader;

#ifdef _WIN32
#pragma pack( pop )
#endif


#endif
