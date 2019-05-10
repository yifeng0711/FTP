

/*
******************************************
Copyright (C), 2010-2011, GOSUN
Filename : GSRCDFILEDEFS.H
Author :  zouyx
Version : 0.1.0.0
Date: 2010/7/22 11:55
Description: 本文件定义录像文件结构和相关格式， 注意参考 <GSMediaDefs.h>
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
brief : 存储文件格式
****************************************

==========文件名结构定义======================
文件名总体结构  具体名字(定长(21Bytes).后续名(最长8Bytes)

名字结构:
录像时间(YYYYMMHHMMSS)-ID(XXXXXXXX)

如:  201007230101-F1234567.bmp


========文件结构定义  只对音视频录像使用， 对于图片直接存为标准格式=====================

文件头 | 帧结构数据(StruGSFrameHeader)

文件头结构:

魔术字(4Bytes)| 头长度(2Bytes) | 文件头版本(1Byte) | 媒体属性个数(1Byte) | 录像时间（YYYYMMDDHHMMSS) | 媒体通道0描述(StruGSMediaDescri)|...|媒体通道n描述(StruGSMediaDescri)
*/

#define GS_RCV_FILE_MAGIC  (('g'<<24)|('s'<<14)|(0xaf<<16)|(0xe3))
//#define GS_RCV_VERSION     0x00
#define GS_RCV_VERSION     0x02


#define GS_GMF_V2 0x02
#define GS_GMF_V0 0x00

// 帧索引开始标志
#define GS_INDEX_BEGIN_MARK (('g'<<24)|('b'<<16)|('i'<<8)|('g'))
// 帧索引结束标志
#define GS_INDEX_END_MARK (('d'<<24)|('e'<<16)|('i'<<8)|('g'))



typedef struct _StruIFrameIndex
{
	UInt32 iTimeStamp;      // 时间戳 (从1970年1月1日0时0分0秒到此时的秒数)
	UInt32 iOffset;         // 对应文件的偏移量
} GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIndex;

//索引结束节点信息
typedef struct _StruIFrameIdxEndNode
{
	UInt32 iIdxStartOffset; //索引信息相对文件开头的偏移, 指向  GS_INDEX_BEGIN_MARK 开头
	UInt32 iEndMarsk; //结束标标志 等于 GS_INDEX_END_MARK

}GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIdxEndNode;

// V2.0 索引定义
typedef struct _StruIFrameIndexV2
{
	UInt64 iTimeStamp;      // 时间戳 (从1970年1月1日0时0分0秒到此时的秒数)
	UInt64 iOffset;         // 对应文件的偏移量
} GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIndexV2;

//V2.0  索引结束节点信息
typedef struct _StruIFrameIdxEndNodeV2
{
	UInt64 iIdxStartOffset; //索引信息相对文件开头的偏移, 指向  GS_INDEX_BEGIN_MARK 开头
	UInt64 iEndMarsk; //结束标标志 等于 GS_INDEX_END_MARK
}GS_MEDIA_ATTRIBUTE_PACKED StruIFrameIdxEndNodeV2;

/* 
文件头 | 帧结构数据 | ... | 帧结构数据 | 索引帧头 | 帧索引开始标志（GS_INDEX_BEGIN_MARK）| 索引信息（StruIFrameIndex）|...| 索引信息（StruIFrameIndex）| StruIFrameIdxEndNode
*/

/* V2 版本
文件头 | 帧结构数据 | ... | 帧结构数据 | 索引帧头 | 帧索引开始标志（GS_INDEX_BEGIN_MARK）| 索引信息（StruIFrameIndexV2）|...| 索引信息（StruIFrameIndexV2）| StruIFrameIdxEndNodeV2
*/


/*
=====独立文件索引文件时的头 文件名为录像文件名 扩展名 .gsidx
文件头结构:
魔术字(4Bytes)| 头长度(2Bytes) | 文件头版本(1Byte)| 保留(1Byte) | 索引帧头(StruGSFrameHeader?) | 帧索引开始标志（GS_INDEX_BEGIN_MARK）| 索引信息（StruIFrameIndex?）|...| 索引信息（StruIFrameIndex?）| StruIFrameIdxEndNode?
*/
#define GS_IDX_FILE_MAGIC  (('g'<<24) | ('i'<<14) | ('d'<<16) | 'x' )
typedef struct _StruGSIdxFHeader
{
	UInt32 iMagic;       //魔术字 为 GS_IDX_FILE_MAGIC
	UInt16 iHeaderSize;   //头长度
	UInt8  iVersion;      //头版本 等于 //GS_GMF_V2
	UInt8  iReserver; //保留
}GS_MEDIA_ATTRIBUTE_PACKED StruGSIdxFHeader;



typedef struct _StruGMFHeader
{
    UInt32 iMagic;       //魔术字 为 GS_RCV_FILE_MAGIC
    UInt16 iHeaderSize;   //头长度
    UInt8  iVersion;      //头版本                                                                                                                                                                                                                                                                                                                                                                                                                               
    UInt8  iChannel;       //媒体通道数  aMediaDescri 的个数
    unsigned char aTv[16];   //录像时间
    StruGSMediaDescri aMediaDescri[1]; //媒体通道描述
}GS_MEDIA_ATTRIBUTE_PACKED StruGMFHeader;

#ifdef _WIN32
#pragma pack( pop )
#endif


#endif
