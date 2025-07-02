// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VOICEPACKET_H__
#define __VOICEPACKET_H__

#pragma once

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include <drx3D/Network/SerializeFwd.h>

class CVoicePacket;

typedef _smart_ptr<CVoicePacket> TVoicePacketPtr;

class CVoicePacket
{
public:
	static i32k MAX_LENGTH = 320;

	void Serialize(TSerialize ser);

	void SetSeq(u8 seq)
	{
		m_seq = seq;
	}
	u8 GetSeq() const
	{
		return m_seq;
	}
	void SetLength(i32 length)
	{
		if (length > MAX_LENGTH)
		{
			NetWarning("[сеть] длина голосового пакета %d слишком большая; рассматривается как отсутствие данных.", length);
			m_length = 0;
		}
		else
			m_length = length;
	}
	i32 GetLength() const
	{
		return m_length;
	}
	u8* GetData()
	{
		return m_data;
	}
	u8k* GetData() const
	{
		return m_data;
	}

	static TVoicePacketPtr Allocate();
	static i32           GetCount();

	void                   AddRef()
	{
		DrxInterlockedIncrement(&m_cnt);
	}
	void Release()
	{
		if (DrxInterlockedDecrement(&m_cnt) <= 0)
			Deallocate(this);
	}

private:
	 i32 m_cnt;

	static void Deallocate(CVoicePacket* pPkt);

	CVoicePacket();
	~CVoicePacket();

	i32        m_length;
	u8        m_seq;
	u8        m_data[MAX_LENGTH];

	static i32 m_count;
};

#endif
#endif
