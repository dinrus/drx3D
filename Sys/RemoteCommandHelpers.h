// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:  Remote command system helper classes
   -------------------------------------------------------------------------
   История:
   - 17/04/2013   : Tomasz Jonarski, Created

*************************************************************************/

#ifndef __REMOTECOMMANDHELPERS_H__
#define __REMOTECOMMANDHELPERS_H__

//------------------------------------------------------------------------

#include <drx3D/Network/IRemoteCommand.h>

struct IServiceNetworkMessage;

//------------------------------------------------------------------------

// Stream reader for service network message
// Implements automatic byte swapping
class CDataReadStreamFormMessage : public IDataReadStream
{
private:
	const IServiceNetworkMessage* m_pMessage;
	tukk                   m_pData;
	u32                        m_offset;
	u32                        m_size;

private:
	template<typename T>
	ILINE void ReadType(uk pData)
	{
		DRX_ASSERT(m_offset + sizeof(T) < m_size);
		const T& readPos = *reinterpret_cast<const T*>(m_pData + m_offset);
		*reinterpret_cast<T*>(pData) = readPos;
		SwapEndian(*reinterpret_cast<T*>(pData));
		m_offset += sizeof(T);
	}

public:
	CDataReadStreamFormMessage(const IServiceNetworkMessage* message);
	virtual ~CDataReadStreamFormMessage();

	u32k GetOffset() const
	{
		return m_offset;
	}

	void SetPosition(u32 offset)
	{
		m_offset = offset;
	}

public:
	// IDataReadStream interface
	virtual void        Delete();
	virtual void        Skip(u32k size);
	virtual void        Read(uk pData, u32k size);
	virtual void        Read8(uk pData);
	virtual void        Read4(uk pData);
	virtual void        Read2(uk pData);
	virtual void        Read1(uk pData);
	virtual ukk GetPointer();
};

//------------------------------------------------------------------------

// Stream writer that writes into the service network message
class CDataWriteStreamToMessage : public IDataWriteStream
{
private:
	IServiceNetworkMessage* m_pMessage;
	tuk                   m_pData;
	u32                  m_offset;
	u32                  m_size;

private:
	template<typename T>
	ILINE void WriteType(ukk pData)
	{
		DRX_ASSERT(m_offset + sizeof(T) < m_size);
		T& writePos = *reinterpret_cast<T*>(m_pData + m_offset);
		writePos = *reinterpret_cast<const T*>(pData);
		SwapEndian(writePos);
		m_offset += sizeof(T);
	}

public:
	CDataWriteStreamToMessage(IServiceNetworkMessage* pMessage);
	virtual ~CDataWriteStreamToMessage();

	// IDataWriteStream interface implementation
	virtual void                           Delete();
	virtual u32k                   GetSize() const;
	virtual struct IServiceNetworkMessage* BuildMessage() const;
	virtual void                           CopyToBuffer(uk pData) const;
	virtual void                           Write(ukk pData, u32k size);
	virtual void                           Write8(ukk pData);
	virtual void                           Write4(ukk pData);
	virtual void                           Write2(ukk pData);
	virtual void                           Write1(ukk pData);
};

//------------------------------------------------------------------------

/// Stream reader reading from owner memory buffer
class CDataReadStreamMemoryBuffer : public IDataReadStream
{
private:
	u32k m_size;
	u8*       m_pData;
	u32       m_offset;

public:
	// memory is copied!
	CDataReadStreamMemoryBuffer(ukk pData, u32k size);
	virtual ~CDataReadStreamMemoryBuffer();

	virtual void        Delete();
	virtual void        Skip(u32k size);
	virtual void        Read8(uk pData);
	virtual void        Read4(uk pData);
	virtual void        Read2(uk pData);
	virtual void        Read1(uk pData);
	virtual ukk GetPointer();
	virtual void        Read(uk pData, u32k size);
};

//------------------------------------------------------------------------

// Stream writer that writes into the internal memory buffer
class CDataWriteStreamBuffer : public IDataWriteStream
{
	static u32k kStaticPartitionSize = 4096;

private:
	// Default (preallocated) partition
	char m_defaultPartition[kStaticPartitionSize];

	// Allocated dynamic partitions
	std::vector<tuk> m_pPartitions;

	// Size of the dynamic message partitions
	std::vector<u32> m_partitionSizes;

	// Pointer to current writing position in the current partition
	tuk m_pCurrentPointer;

	// Space left in current partition
	u32 m_leftInPartition;

	// Total message size so far
	u32 m_size;

private:
	// Directly write typed data into the stream
	template<typename T>
	ILINE void WriteType(ukk pData)
	{
		// try to use the faster path if we are not crossing the partition boundary
		if (m_leftInPartition >= sizeof(T))
		{
			// faster case
			T& writePos = *reinterpret_cast<T*>(m_pCurrentPointer);
			writePos = *reinterpret_cast<const T*>(pData);
			SwapEndian(writePos);
			m_pCurrentPointer += sizeof(T);
			m_leftInPartition -= sizeof(T);
			m_size += sizeof(T);
		}
		else
		{
			// slower case (more generic)
			T tempVal(*reinterpret_cast<const T*>(pData));
			SwapEndian(tempVal);
			Write(&tempVal, sizeof(tempVal));
		}
	}

public:
	CDataWriteStreamBuffer();
	virtual ~CDataWriteStreamBuffer();

	// IDataWriteStream interface implementation
	virtual void                    Delete();
	virtual u32k            GetSize() const;
	virtual IServiceNetworkMessage* BuildMessage() const;
	virtual void                    CopyToBuffer(uk pData) const;
	virtual void                    Write(ukk pData, u32k size);
	virtual void                    Write8(ukk pData);
	virtual void                    Write4(ukk pData);
	virtual void                    Write2(ukk pData);
	virtual void                    Write1(ukk pData);
};

//-----------------------------------------------------------------------------

// Packet header
struct PackedHeader
{
	// Estimation (or better yet, exact value) of how much data this header will take when written.
	// Please make sure that actual size after serialization is not bigger than this value.
	static u32k kSerializationSize = sizeof(u8) + sizeof(u32) + sizeof(u32);

	// Magic value that identifies command messages vs raw messages
	static u32k kMagic = 0xABBAF00D;

	// Command type
	// Keep the values unchanged as this may break the protocol
	enum ECommand
	{
		// Server class list mapping
		eCommand_ClassList = 0,

		// Command data
		eCommand_Command = 1,

		// Disconnect signal
		eCommand_Disconnect = 2,

		// ACK packet
		eCommand_ACK = 3,
	};

	u32 magic;
	u8  msgType;
	u32 count;

	// serialization operator
	template<class T>
	friend T& operator<<(T& stream, PackedHeader& header)
	{
		stream << header.magic;
		stream << header.msgType;
		stream << header.count;
		return stream;
	}
};

// Header sent with every command
struct CommandHeader
{
	u32 commandId;
	u32 classId;
	u32 size;

	CommandHeader()
		: commandId(0)
		, classId(0)
		, size(0)
	{}

	// serialization operator
	template<class T>
	friend T& operator<<(T& stream, CommandHeader& header)
	{
		stream << header.commandId;
		stream << header.classId;
		stream << header.size;
		return stream;
	}
};

// General Response/ACK header
struct ResponseHeader
{
	u32 magic;
	u8  msgType;
	u32 lastCommandReceived;
	u32 lastCommandExecuted;

	ResponseHeader()
		: lastCommandReceived(0)
		, lastCommandExecuted(0)
		, msgType(PackedHeader::eCommand_ACK)
	{}

	// serialization operator
	template<class T>
	friend T& operator<<(T& stream, ResponseHeader& header)
	{
		stream << header.magic;
		stream << header.msgType;
		stream << header.lastCommandReceived;
		stream << header.lastCommandExecuted;
		return stream;
	}
};

//-----------------------------------------------------------------------------

#endif
