// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include  <drx3D/Network/VoicePacket.h>
	#include <drx3D/Network/ISerialize.h>
	#include <drx3D/CoreX/Memory/PoolAllocator.h>

i32 CVoicePacket::m_count = 0;

	#if USE_SYSTEM_ALLOCATOR
TVoicePacketPtr CVoicePacket::Allocate()
{
	return new CVoicePacket;
}

void CVoicePacket::Deallocate(CVoicePacket* pPkt)
{
	delete pPkt;
}
	#else
typedef stl::PoolAllocator<sizeof(CVoicePacket)> TVoicePool;
static TVoicePool* m_pVoicePool = NULL;

TVoicePacketPtr CVoicePacket::Allocate()
{
	if (!m_pVoicePool)
		m_pVoicePool = new TVoicePool;
	uk pPkt = m_pVoicePool->Allocate();
	new(pPkt) CVoicePacket();
	return (CVoicePacket*) pPkt;
}

void CVoicePacket::Deallocate(CVoicePacket* pPkt)
{
	pPkt->~CVoicePacket();
	m_pVoicePool->Deallocate(pPkt);
}
	#endif

CVoicePacket::CVoicePacket() : m_cnt(0), m_length(0)
{
	m_count++;
	++g_objcnt.voicePacket;
}

CVoicePacket::~CVoicePacket()
{
	m_count--;
	--g_objcnt.voicePacket;
}

void CVoicePacket::Serialize(TSerialize ser)
{
	ser.Value("seq", m_seq);
	ser.EnumValue("length", m_length, 0, MAX_LENGTH);
	ser.BeginGroup("data");
	for (i32 i = 0; i < m_length; i++)
		ser.Value("voicebyte", m_data[i]);
	ser.EndGroup();
}

i32 CVoicePacket::GetCount()
{
	return m_count;
}

#endif
