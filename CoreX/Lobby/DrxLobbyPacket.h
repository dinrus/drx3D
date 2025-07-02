// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

<drx3D/CoreX/Lobby/IDrxMatchMaking.h>               // <> required for Interfuscator

u32k DrxLobbyPacketUINT8Size = 1;
u32k DrxLobbyPacketUINT16Size = 2;
u32k DrxLobbyPacketUINT32Size = 4;
u32k DrxLobbyPacketUINT64Size = 8;
u32k DrxLobbyPacketBoolSize = 1;
u32k DrxLobbyPacketConnectionUIDSize = DrxLobbyPacketUINT64Size + DrxLobbyPacketUINT16Size;

u32k DrxLobbyPacketReliablePacketHeaderSize =
  DrxLobbyPacketUINT8Size +                  // eH_DrxLobby
  DrxLobbyPacketConnectionUIDSize +          // fromUID
  DrxLobbyPacketBoolSize +                   // reliable
  DrxLobbyPacketUINT8Size +                  // counterOut
  DrxLobbyPacketUINT8Size;                   // counterIn

u32k DrxLobbyPacketUnReliablePacketHeaderSize =
  DrxLobbyPacketUINT8Size +                  // eH_DrxLobby
  DrxLobbyPacketConnectionUIDSize +          // fromUID
  DrxLobbyPacketBoolSize;                    // reliable

u32k DrxLobbyPacketReliableDataHeaderSize =
  DrxLobbyPacketUINT8Size +                  // lobbyPacketType
  DrxLobbyPacketUINT16Size;                  // dataSize

u32k DrxLobbyPacketUnReliableDataHeaderSize =
  DrxLobbyPacketUINT8Size;                   // lobbyPacketType

u32k DrxLobbyPacketReliableHeaderSize = DrxLobbyPacketReliablePacketHeaderSize + DrxLobbyPacketReliableDataHeaderSize;
u32k DrxLobbyPacketUnReliableHeaderSize = DrxLobbyPacketUnReliablePacketHeaderSize + DrxLobbyPacketUnReliableDataHeaderSize;
u32k DrxLobbyPacketHeaderSize = DrxLobbyPacketReliableHeaderSize;

struct SDrxLobbyPacketHeader
{
	CTimeValue                   recvTime;   //!< Reliable and unreliable.
	SDrxMatchMakingConnectionUID fromUID;    //!< Reliable and unreliable.
	bool                         reliable;   //!< Reliable and unreliable.
	u8                        counterOut; //!< Reliable.
	u8                        counterIn;  //!< Reliable.
};

struct SDrxLobbyPacketDataHeader
{
	u16 dataSize;                         //!< Reliable.
	u8  lobbyPacketType;                  //!< Reliable and unreliable.
};

//! Creating a packet to send.
//! Call CreateWriteBuffer to create the buffer to write to or call SetWriteBuffer if you wish to specify your own buffer.
//! Call StartWrite with the packet id you wish to use and if the packet should be sent reliably.
//! Game user defined packet id's start at DRXLOBBY_USER_PACKET_START and end at DRXLOBBY_USER_PACKET_MAX there are 128 user defined packet id's.
//! Call the various Write functions to add your data to the packet.
//! Send the packet.
//! Call FreeWriteBuffer if the packet was created with CreateWriteBuffer.
//! Processing a received a packet.
//! Call StartRead which will return the packet id.
//! Call the various Read functions to get your data out.
class CDrxLobbyPacket
{
public:
	CDrxLobbyPacket()
	{
		m_pWriteBuffer = NULL;
		m_bufferSize = 0;
		m_bufferPos = 0;
		m_allocated = false;
	}

	~CDrxLobbyPacket()
	{
		FreeWriteBuffer();
	}

	bool CreateWriteBuffer(u32 bufferSize)
	{
		FreeWriteBuffer();

		m_pWriteBuffer = new u8[bufferSize];
		m_bufferSize = bufferSize;
		m_bufferPos = 0;
		m_allocated = true;
		return true;
	}

	void FreeWriteBuffer()
	{
		if (m_allocated)
		{
			delete[] m_pWriteBuffer;
			m_pWriteBuffer = NULL;
			m_bufferSize = 0;
			m_bufferPos = 0;
			m_allocated = false;
		}
	}

	void SetWriteBuffer(u8* pBuffer, u32 bufferSize)
	{
		FreeWriteBuffer();
		m_pWriteBuffer = pBuffer;
		m_bufferSize = bufferSize;
		m_bufferPos = 0;
	}

	void SetReadBuffer(u8k* pBuffer, u32 bufferSize)
	{
		FreeWriteBuffer();
		m_pReadBuffer = pBuffer;
		m_bufferSize = bufferSize;
		m_bufferPos = 0;
	}

	void StartWrite(u32 packetType, bool reliable)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE((reliable && (DrxLobbyPacketReliableHeaderSize <= m_bufferSize)) || (!reliable && (DrxLobbyPacketUnReliableHeaderSize <= m_bufferSize)), "CDrxLobbyPacket: Buffer too small");

		m_packetHeader.reliable = reliable;
		m_dataHeader.lobbyPacketType = packetType;

		if (reliable)
		{
			m_bufferPos = DrxLobbyPacketReliableHeaderSize;
		}
		else
		{
			m_bufferPos = DrxLobbyPacketUnReliableHeaderSize;
		}
	}

	u32 StartRead()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE((m_packetHeader.reliable && (DrxLobbyPacketReliableHeaderSize <= m_bufferSize)) || (!m_packetHeader.reliable && (DrxLobbyPacketUnReliableHeaderSize <= m_bufferSize)), "CDrxLobbyPacket: Buffer too small");

		if (m_packetHeader.reliable)
		{
			m_bufferPos = DrxLobbyPacketReliableHeaderSize;
		}
		else
		{
			m_bufferPos = DrxLobbyPacketUnReliableHeaderSize;
		}

		return m_dataHeader.lobbyPacketType;
	}

	void WriteUINT8(u8 data)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT8Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + DrxLobbyPacketUINT8Size <= m_bufferSize))
		{
			m_pWriteBuffer[m_bufferPos++] = data;
		}
	}

	u8 ReadUINT8()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT8Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + DrxLobbyPacketUINT8Size <= m_bufferSize))
		{
			return m_pReadBuffer[m_bufferPos++];
		}

		return 0;
	}

	void WriteUINT16(u16 data)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT16Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + DrxLobbyPacketUINT16Size <= m_bufferSize))
		{
			m_pWriteBuffer[m_bufferPos++] = (data >> 8) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data) & 0xff;
		}
	}

	u16 ReadUINT16()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT16Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + DrxLobbyPacketUINT16Size <= m_bufferSize))
		{
			u16 ret;

			ret = m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];

			return ret;
		}

		return 0;
	}

	void WriteUINT32(u32 data)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT32Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + DrxLobbyPacketUINT32Size <= m_bufferSize))
		{
			m_pWriteBuffer[m_bufferPos++] = (data >> 24) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 16) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 8) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data) & 0xff;
		}
	}

	u32 ReadUINT32()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT32Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + DrxLobbyPacketUINT32Size <= m_bufferSize))
		{
			u32 ret;

			ret = m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];

			return ret;
		}

		return 0;
	}

	void WriteUINT64(uint64 data)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT64Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + DrxLobbyPacketUINT64Size <= m_bufferSize))
		{
			m_pWriteBuffer[m_bufferPos++] = (data >> 56) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 48) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 40) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 32) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 24) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 16) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data >> 8) & 0xff;
			m_pWriteBuffer[m_bufferPos++] = (data) & 0xff;
		}
	}

	uint64 ReadUINT64()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketUINT64Size <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + DrxLobbyPacketUINT64Size <= m_bufferSize))
		{
			uint64 ret;

			ret = m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];
			ret = (ret << 8) | m_pReadBuffer[m_bufferPos++];

			return ret;
		}

		return 0;
	}

	void WriteBool(bool data)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketBoolSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + DrxLobbyPacketBoolSize <= m_bufferSize))
		{
			m_pWriteBuffer[m_bufferPos++] = data ? 1 : 0;
		}
	}

	bool ReadBool()
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + DrxLobbyPacketBoolSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + DrxLobbyPacketBoolSize <= m_bufferSize))
		{
			return m_pReadBuffer[m_bufferPos++] ? true : false;
		}

		return false;
	}

	void WriteData(ukk pData, u32 dataSize)
	{
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + dataSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + dataSize <= m_bufferSize))
		{
			memcpy(&m_pWriteBuffer[m_bufferPos], pData, dataSize);
			m_bufferPos += dataSize;
		}
	}

	void ReadData(uk pData, u32 dataSize)
	{
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + dataSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + dataSize <= m_bufferSize))
		{
			memcpy(pData, &m_pReadBuffer[m_bufferPos], dataSize);
			m_bufferPos += dataSize;
		}
	}

	//! Include the null terminator in the length - it won't be sent across the wire, but will set correctly on read.
	void WriteString(tukk pData, u32 dataSize)
	{
		dataSize--;          // Don't bother sending null terminator
		DRX_ASSERT_MESSAGE(m_pWriteBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + dataSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pWriteBuffer && (m_bufferPos + dataSize <= m_bufferSize))
		{
			memcpy(&m_pWriteBuffer[m_bufferPos], pData, dataSize);
			m_bufferPos += dataSize;
		}
	}

	//! Include the null terminator in the length - it won't be sent across the wire, but will set correctly on read.
	void ReadString(tuk pData, u32 dataSize)
	{
		dataSize--;          // NULL terminator is not in stream
		DRX_ASSERT_MESSAGE(m_pReadBuffer, "CDrxLobbyPacket: Buffer not set");
		DRX_ASSERT_MESSAGE(m_bufferPos + dataSize <= m_bufferSize, "CDrxLobbyPacket: Buffer too small");

		if (m_pReadBuffer && (m_bufferPos + dataSize <= m_bufferSize))
		{
			memcpy(pData, &m_pReadBuffer[m_bufferPos], dataSize);
			m_bufferPos += dataSize;
		}
		pData[dataSize] = 0;   // Add null terminator back onto buffer
	}

	void WriteConnectionUID(SDrxMatchMakingConnectionUID data)
	{
		WriteUINT64(data.m_sid);
		WriteUINT16(data.m_uid);
	}

	SDrxMatchMakingConnectionUID ReadConnectionUID()
	{
		SDrxMatchMakingConnectionUID temp;
		temp.m_sid = ReadUINT64();
		temp.m_uid = ReadUINT16();
		return temp;
	}

	void WriteDrxLobbyUserData(const SDrxLobbyUserData* pData)
	{
		switch (pData->m_type)
		{
		case eCLUDT_Int64:
		case eCLUDT_Float64:
			WriteUINT64(pData->m_int64);
			break;

		case eCLUDT_Int32:
		case eCLUDT_Float32:
			WriteUINT32(pData->m_int32);
			break;

		case eCLUDT_Int16:
			WriteUINT16(pData->m_int16);
			break;

		case eCLUDT_Int8:
			WriteUINT8(pData->m_int8);
			break;

		case eCLUDT_Int64NoEndianSwap:
			WriteData(&pData->m_int64, sizeof(pData->m_int64));
			break;

		default:
			DRX_ASSERT_MESSAGE(0, "CDrxLobbyPacket::WriteDrxLobbyUserData: Undefined data type");
			break;
		}
	}

	void ReadDrxLobbyUserData(SDrxLobbyUserData* pData)
	{
		switch (pData->m_type)
		{
		case eCLUDT_Int64:
		case eCLUDT_Float64:
			pData->m_int64 = ReadUINT64();
			break;

		case eCLUDT_Int32:
		case eCLUDT_Float32:
			pData->m_int32 = ReadUINT32();
			break;

		case eCLUDT_Int16:
			pData->m_int16 = ReadUINT16();
			break;

		case eCLUDT_Int8:
			pData->m_int8 = ReadUINT8();
			break;

		case eCLUDT_Int64NoEndianSwap:
			ReadData(&pData->m_int64, sizeof(pData->m_int64));
			break;

		default:
			DRX_ASSERT_MESSAGE(0, "CDrxLobbyPacket::ReadDrxLobbyUserData: Undefined data type");
			break;
		}
	}

	bool       GetReliable() { return m_packetHeader.reliable; }
	CTimeValue GetRecvTime() { return m_packetHeader.recvTime; }

	bool       IsValidToSend() const
	{
		return (m_bufferPos > 0);
	}

	u8*       GetWriteBuffer()              { return m_pWriteBuffer; }
	u32       GetWriteBufferSize()          { return m_bufferSize; }
	u32       GetWriteBufferPos()           { return m_bufferPos; }
	void         SetWriteBufferPos(u32 pos) { m_bufferPos = pos; }

	u8k* GetReadBuffer()               { return m_pReadBuffer; }
	u32       GetReadBufferSize()           { return m_bufferSize; }
	u32       GetReadBufferPos()            { return m_bufferPos; }
	void         SetReadBufferPos(u32 pos)  { m_bufferPos = pos; }

protected:
	SDrxLobbyPacketHeader     m_packetHeader;
	SDrxLobbyPacketDataHeader m_dataHeader;

	union
	{
		u8*       m_pWriteBuffer;
		u8k* m_pReadBuffer;
	};

	u32 m_bufferSize;
	u32 m_bufferPos;
	bool   m_allocated;
};
