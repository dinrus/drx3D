// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RECORDINGBUFFER_H__
#define __RECORDINGBUFFER_H__

#define COMPILE_TIME_ASSERT(_Condition)

class CBufferUtil;

enum ERecordingBufferPacketType
{
	eRBPT_Invalid = 0,
	eRBPT_FrameData,
	eRBPT_Custom,
};

// Struct is made to be 4 bytes aligned to ensure all packets within the recording buffer
// are at least 4 byte aligned.
struct DRX_ALIGN(4) SRecording_Packet
{
	SRecording_Packet()
		: size(0)
		, type(eRBPT_Invalid)
	{
		COMPILE_TIME_ASSERT( alignof(SRecording_Packet) == 4 );
	}
	u16 size;
	u8 type;
};

struct SRecording_FrameData : SRecording_Packet
{
	SRecording_FrameData()
		: frametime(0)
	{
		size = sizeof(SRecording_FrameData);
		type = eRBPT_FrameData;
	}

	void Serialise(CBufferUtil &buffer);

	float frametime;
};

class CRecordingBuffer
{
public:
	class iterator : public std::iterator<std::forward_iterator_tag, SRecording_Packet>
	{
	public:
		iterator(CRecordingBuffer* pBuffer, size_t offset) : m_pRecordingBuffer(pBuffer), m_offset(offset)
		{
			size_t prefetchOffset = MIN(m_offset + 128, pBuffer->size());
			PrefetchLine(pBuffer, prefetchOffset);
		}

		SRecording_Packet& operator*() const { return *(SRecording_Packet*)m_pRecordingBuffer->at(m_offset); }
		SRecording_Packet* operator->() const { return (SRecording_Packet*)m_pRecordingBuffer->at(m_offset); }
		iterator& operator++()
		{
			m_offset += ((SRecording_Packet*)m_pRecordingBuffer->at(m_offset))->size;

			size_t prefetchOffset = MIN(m_offset + 256, m_pRecordingBuffer->size());
			PrefetchLine(m_pRecordingBuffer, prefetchOffset);
			return *this;
		}
		iterator operator++(i32)
		{
			iterator prevValue(m_pRecordingBuffer, m_offset);
			++*this;
			return prevValue;
		}
		bool operator==(iterator const& rhs) const { return m_offset == rhs.m_offset; }
		bool operator!=(iterator const& rhs) const { return m_offset != rhs.m_offset; }
	
	private:
		CRecordingBuffer* m_pRecordingBuffer;
		size_t m_offset;
	};

	typedef void PacketDiscardCallback(SRecording_Packet *pDiscardedPacket, float recordedTime, uk pUserData);

	CRecordingBuffer();
	CRecordingBuffer(size_t size);
	CRecordingBuffer(size_t size, u8 *buffer);

	~CRecordingBuffer();

	void Init(size_t size, u8 *buffer);
	// initialises the buffer in the case where initialisation was not performed during construction

	void Reset();
	//   - clear the buffer memory
	//   - reset the packet lists

	void Update();
	//   - add an entry to the list that stores the list of frame/packet things

	void AddPacket(const SRecording_Packet& packet);
	//   - adds a packet to the buffer

	SRecording_Packet *AllocEmptyPacket(i32 inSize, u8 inPacketType);
	//   - allocates an empty packet of the size and type specified. inSize should include sizeof(SRecording_Packet) which all allocations are required to have at their start

	size_t GetData(u8 *pBuffer, size_t bufferSize) const;
	
	u8k* at(size_t offset) const
	{
		DRX_ASSERT_MESSAGE(offset < m_usedSize, "Start offset is too large");
		u8* pStart = m_pStart + offset;
		if (pStart >= m_pBuffer + m_dynamicBufferSize)
		{
			pStart -= m_dynamicBufferSize;
		}
		return pStart;
	}

	iterator begin() { return iterator(this, 0); }
	iterator end() { return iterator(this, m_usedSize); }

	size_t size() const { return m_usedSize; }
	size_t capacity() const { return m_actualBufferSize; }

	void SetPacketDiscardCallback(PacketDiscardCallback *pdc, uk inUserData) {m_pDiscardCallback = pdc; m_pDiscardCallbackUserData=inUserData; }

	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pBuffer,m_actualBufferSize);		
	}

	bool ContainsPtr(ukk inPtr) const;
	
	void RemoveFrame();
	void RemovePacket(float recordedTime = 0);

private:
	u8* GetEnd()
	{
		u8* pEnd = m_pStart + m_usedSize;
		if (pEnd >= m_pBuffer + m_dynamicBufferSize)
		{
			pEnd -= m_dynamicBufferSize;
		}
		return pEnd;
	}

	void RemoveFromStart(size_t size);
	void EnsureFreeSpace(size_t size);

	bool m_allocatedBuffer;

	u8* m_pBuffer;
	size_t m_dynamicBufferSize;		// The size of the physical buffer minus any wasted space at the end (to avoid splitting packets)
	size_t m_actualBufferSize;		// The actual size of the physical buffer
	
	u8* m_pStart;
	size_t m_usedSize;

	PacketDiscardCallback	*m_pDiscardCallback;
	void					*m_pDiscardCallbackUserData;
};

#endif // __RECORDINGBUFFER_H__
