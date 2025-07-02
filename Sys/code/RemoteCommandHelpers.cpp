// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:  Helper classes for remote command system
   -------------------------------------------------------------------------
   История:
   - 17/04/2013   : Tomasz Jonarski, Created

*************************************************************************/
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Sys/RemoteCommandHelpers.h>

//-----------------------------------------------------------------------------

CDataReadStreamFormMessage::CDataReadStreamFormMessage(const IServiceNetworkMessage* message)
	: m_pMessage(message)
	, m_size(message->GetSize())
	, m_pData(static_cast<tukk >(message->GetPointer()))
	, m_offset(0)
{
	// AddRef() is not const unfortunatelly
	const_cast<IServiceNetworkMessage*>(m_pMessage)->AddRef();
}

CDataReadStreamFormMessage::~CDataReadStreamFormMessage()
{
	// Release() is not const unfortunatelly
	const_cast<IServiceNetworkMessage*>(m_pMessage)->Release();
}

void CDataReadStreamFormMessage::Delete()
{
	delete this;
}

void CDataReadStreamFormMessage::Skip(u32k size)
{
	DRX_ASSERT(m_offset + size < m_size);
	m_offset += size;
}

void CDataReadStreamFormMessage::Read(uk pData, u32k size)
{
	DRX_ASSERT(m_offset + size < m_size);
	tukk pReadPtr = m_pData + m_offset;
	memcpy(pData, pReadPtr, size);
	m_offset += size;
}

void CDataReadStreamFormMessage::Read8(uk pData)
{
	// it does not actually matter if its uint64, int64 or double so use any
	ReadType<uint64>(pData);
}

void CDataReadStreamFormMessage::Read4(uk pData)
{
	// it does not actually matter if its u32, i32 or float so use any
	ReadType<u32>(pData);
}

void CDataReadStreamFormMessage::Read2(uk pData)
{
	// it does not actually matter if its u16, i16 so use any
	ReadType<u16>(pData);
}

void CDataReadStreamFormMessage::Read1(uk pData)
{
	// it does not actually matter if its u8, int8 so use any
	ReadType<u8>(pData);
}

ukk CDataReadStreamFormMessage::GetPointer()
{
	tukk pReadPtr = m_pData + m_offset;
	return pReadPtr;
}

//-----------------------------------------------------------------------------

CDataWriteStreamToMessage::CDataWriteStreamToMessage(IServiceNetworkMessage* pMessage)
	: m_pMessage(pMessage)
	, m_size(pMessage->GetSize())
	, m_pData(static_cast<tuk>(pMessage->GetPointer()))
	, m_offset(0)
{
	m_pMessage->AddRef();
}

CDataWriteStreamToMessage::~CDataWriteStreamToMessage()
{
	m_pMessage->Release();
}

void CDataWriteStreamToMessage::Delete()
{
	delete this;
}

u32k CDataWriteStreamToMessage::GetSize() const
{
	return m_size;
}

void CDataWriteStreamToMessage::CopyToBuffer(uk pData) const
{
	memcpy(pData, m_pData, m_size);
}

IServiceNetworkMessage* CDataWriteStreamToMessage::BuildMessage() const
{
	m_pMessage->AddRef();
	return m_pMessage;
}

void CDataWriteStreamToMessage::Write(ukk pData, u32k size)
{
	DRX_ASSERT(m_offset + size < m_size);
	memcpy((tuk)m_pData + m_offset, pData, size);
	m_offset += size;
}

void CDataWriteStreamToMessage::Write8(ukk pData)
{
	// it does not actually matter if its uint64, int64 or double so use any
	WriteType<uint64>(pData);
}

void CDataWriteStreamToMessage::Write4(ukk pData)
{
	// it does not actually matter if its u32, i32 or float so use any
	WriteType<u32>(pData);
}

void CDataWriteStreamToMessage::Write2(ukk pData)
{
	// it does not actually matter if its u16, i16 so use any
	WriteType<u16>(pData);
}

void CDataWriteStreamToMessage::Write1(ukk pData)
{
	// it does not actually matter if its u8, int8 so use any
	WriteType<u8>(pData);
}

//-----------------------------------------------------------------------------

CDataReadStreamMemoryBuffer::CDataReadStreamMemoryBuffer(ukk pData, u32k size)
	: m_size(size)
	, m_offset(0)
{
	m_pData = new u8[size];
	memcpy(m_pData, pData, size);
}

CDataReadStreamMemoryBuffer::~CDataReadStreamMemoryBuffer()
{
	delete[] m_pData;
	m_pData = NULL;
}

void CDataReadStreamMemoryBuffer::Delete()
{
	delete this;
}

void CDataReadStreamMemoryBuffer::Skip(u32k size)
{
	DRX_ASSERT(m_offset + size <= m_size);
	m_offset += size;
}

void CDataReadStreamMemoryBuffer::Read8(uk pData)
{
	Read(pData, 8);
	SwapEndian(*reinterpret_cast<uint64*>(pData));
}

void CDataReadStreamMemoryBuffer::Read4(uk pData)
{
	Read(pData, 4);
	SwapEndian(*reinterpret_cast<u32*>(pData));
}

void CDataReadStreamMemoryBuffer::Read2(uk pData)
{
	Read(pData, 2);
	SwapEndian(*reinterpret_cast<u16*>(pData));
}

void CDataReadStreamMemoryBuffer::Read1(uk pData)
{
	return Read(pData, 1);
}

ukk CDataReadStreamMemoryBuffer::GetPointer()
{
	return m_pData + m_offset;
};

void CDataReadStreamMemoryBuffer::Read(uk pData, u32k size)
{
	DRX_ASSERT(m_offset + size <= m_size);
	memcpy(pData, m_pData + m_offset, size);
	m_offset += size;
}

//-----------------------------------------------------------------------------

CDataWriteStreamBuffer::CDataWriteStreamBuffer()
	: m_size(0)
{
	// Start with the initial (preallocated) partition
	// This optimization assumes that initial size of most of the messages will be small.
	// NOTE: default partition is not added to the partition table (that would require push_backs to vector)
	tuk partitionMemory = &m_defaultPartition[0];
	m_pCurrentPointer = partitionMemory;
	m_leftInPartition = sizeof(m_defaultPartition);
}

CDataWriteStreamBuffer::~CDataWriteStreamBuffer()
{
	// Free all memory partitions that were allocated dynamically
	for (size_t i = 0; i < m_pPartitions.size(); ++i)
	{
		free(m_pPartitions[i]);
	}
}

void CDataWriteStreamBuffer::Delete()
{
	delete this;
}

u32k CDataWriteStreamBuffer::GetSize() const
{
	return m_size;
}

void CDataWriteStreamBuffer::CopyToBuffer(uk pData) const
{
	u32 dataLeft = m_size;
	tuk pWritePtr = (tuk)pData;

	// Copy data from default (preallocated) partition
	{
		u32k partitionSize = sizeof(m_defaultPartition);
		u32k dataToCopy = min<u32>(partitionSize, dataLeft);
		memcpy(pWritePtr, &m_defaultPartition[0], dataToCopy);

		// advance
		pWritePtr += dataToCopy;
		dataLeft -= dataToCopy;
	}

	// Copy data from dynamic partitions
	for (u32 i = 0; i < m_pPartitions.size(); ++i)
	{
		// get size of data to copy
		u32k partitionSize = m_partitionSizes[i];
		u32k dataToCopy = min<u32>(partitionSize, dataLeft);
		memcpy(pWritePtr, m_pPartitions[i], dataToCopy);

		// advance
		pWritePtr += dataToCopy;
		dataLeft -= dataToCopy;
	}

	// Make sure all data was written
	DRX_ASSERT(dataLeft == 0);
}

IServiceNetworkMessage* CDataWriteStreamBuffer::BuildMessage() const
{
	// No data written, no message created
	if (0 == m_size)
	{
		return NULL;
	}

	// Create message to hold all the data
	IServiceNetworkMessage* pMessage = gEnv->pServiceNetwork->AllocMessageBuffer(m_size);
	if (NULL == pMessage)
	{
		return NULL;
	}

	// Copy data to messages
	CopyToBuffer(pMessage->GetPointer());
	return pMessage;
}

void CDataWriteStreamBuffer::Write(ukk pData, u32k size)
{
	static u32k kAdditionalPartitionSize = 65536;

	u32 dataLeft = size;
	while (dataLeft > 0)
	{
		// new partition needed
		if (m_leftInPartition == 0)
		{
			// Allocate new partition data
			tuk partitionMemory = (tuk)malloc(kAdditionalPartitionSize);
			DRX_ASSERT(partitionMemory != NULL);

			// add new partition to list
			m_partitionSizes.push_back(kAdditionalPartitionSize);
			m_pPartitions.push_back(partitionMemory);
			m_pCurrentPointer = partitionMemory;
			m_leftInPartition = kAdditionalPartitionSize;
		}

		// how many bytes can we write to current partition ?
		u32k maxToWrite = min<u32>(m_leftInPartition, dataLeft);
		memcpy(m_pCurrentPointer, pData, maxToWrite);

		// advance
		m_size += maxToWrite;
		dataLeft -= maxToWrite;
		pData = (tukk)pData + maxToWrite;
		m_pCurrentPointer += maxToWrite;
		m_leftInPartition -= maxToWrite;
	}
}

void CDataWriteStreamBuffer::Write8(ukk pData)
{
	// it does not actually matter if its uint64, int64 or double so use any
	WriteType<uint64>(pData);
}

void CDataWriteStreamBuffer::Write4(ukk pData)
{
	// it does not actually matter if its u32, i32 or float so use any
	WriteType<u32>(pData);
}

void CDataWriteStreamBuffer::Write2(ukk pData)
{
	// it does not actually matter if its u16, i16 so use any
	WriteType<u16>(pData);
}

void CDataWriteStreamBuffer::Write1(ukk pData)
{
	// it does not actually matter if its u8, int8 so use any
	WriteType<u8>(pData);
}

//-----------------------------------------------------------------------------
