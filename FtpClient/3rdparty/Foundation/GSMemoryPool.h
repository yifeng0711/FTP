/*
******************************************
Copyright (C): GOSUN
Filename : GSMEMORYPOOL.H
Author :  zouyx
Description: �ڴ��ʵ��
********************************************
*/

#ifndef _GS_H_GSMEMORYPOOL_H_
#define _GS_H_GSMEMORYPOOL_H_

#include "GSFdtDefs.h"
#include "GxxList.hpp"

/*
*********************************************************************
*
*@brief : �ڴ�ع����࣬ ����̶���С���ڴ�Ƭ����̶���С�������ڴ�Ƭ
*note :   ������̰߳�ȫ
*********************************************************************
*/
class GSFDT_CLASS GSMemoryPool
{
private :
	unsigned int m_nPartitionSize;
	unsigned int m_nSliceSize;
	unsigned int m_nNextCount;
	unsigned int m_nMaxCount;
	bool m_bAbleOrderedMalloc;
	unsigned int m_nCalloced; //�Ѿ�������ڴ�
	
	GxxListInner<char *> m_listCalloced; //������ڴ�	
	UInt32 m_nFree;
	void *m_pFreeFirst;
public :
	// nSliceSize �ڴ��ÿ���ڴ�Ƭ�Ĵ�С(Byte)���� malloc ���صĴ�С
	// nNextCount �ڴ��ÿ�����ӵ��ڴ�Ƭ����, ���ֵС�ڵ������������ڴ�Ƭ�Ĵ�С
	// nMaxCount �ڴ����������ڴ�Ƭ����, ����ﵽ������� malloc������NULL, 0 ���޶�
	// bAbleOrderedMalloc �Ƿ�ʹ�÷����Ƭ�����ڴ�Ƭ���ܣ������ȷ��ʹ�ý�ֹ������Լӿ�����ٶȣ�Ĭ��ʹ��
	GSMemoryPool(UInt32 nSliceSize,  UInt32 nNextCount = 32
				, UInt32 nMaxCount = 0,bool bAbleOrderedMalloc = true );
	~GSMemoryPool(void);

	//�����ڴ棬 �ͷŵ��ڴ���п��е��ڴ�
	//  nSliceReserveCounts �������ͷŵ��ڴ�Ƭ����, 0 ȫ���ͷ�
	void ShrinkMemory(UInt32 nReserveCounts = 32);


	UInt32 GetSliceSize(void) const;
	UInt32 GetNextCount(void) const;
	UInt32 GetMaxCount(void) const;
	void SetMaxCount(UInt32 nMaxCount);

	//���ؿ��е��ڴ�Ƭ����
	UInt32 GetFreeCount(void) const;

	//�����ܵ�Ƭ���� ����+malloc�ĸ���
	UInt32 GetTotalCount(void) const;


	//���������ڴ�, ���ص��ڴ��С����nSliceSize*nCount
	void *Malloc(UInt16 nCount = 1);

	//���������ڴ�, ���� ���ڴ��СnSize
	void *MallocEx(unsigned int nSize);


	// �ͷ� Malloc�ķ���ֵ
	void Free(void *data);
	
	//�Ƿ������� �ڴ�ط�����ڴ棬 *������ʹ�ã� ��ֻ���ڷ����ڴ��ͬһdll ��ʹ��
	static void FreeAny( void *data);
	
private :
	void *CallocNext( UInt16 nCounts );
	void AddOrdered(void *pPartition, unsigned int nPartition, void *pChunk);

	void *MallocN(UInt16 n);
	void *TryMallocN(void * & start, UInt16 n);

	 void * FindPrev(void *const pFreeFirst, void * pPartition);	
	 void *Segregate(void *pPartition, UInt16 nPartition, void *pEnd);

	 void *ChunckMallocSliceN(void *pChunk,UInt16 n);

	GS_CLASS_NOCOPY(GSMemoryPool);
};

#endif //end _GS_H_GSMEMORYPOOL_H_
