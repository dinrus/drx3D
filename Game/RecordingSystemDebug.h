// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RECORDINGSYSTEMDEBUG_H__
#define __RECORDINGSYSTEMDEBUG_H__
#include <drx3D/Game/RecordingSystemDefines.h>

#ifdef RECSYS_DEBUG


class CRecordingSystemDebug : public IRecordingSystemListener
{
public:
	CRecordingSystemDebug(CRecordingSystem& system);
	virtual ~CRecordingSystemDebug();

	//IRecordingSystemListener
	virtual void OnPlaybackRequested( const SPlaybackRequest& info ) {}
	virtual void OnPlaybackStarted( const SPlaybackInfo& info ) {}
	virtual void OnPlaybackEnd( const SPlaybackInfo& info ) {}
	//~IRecordingSystemListener

	void PrintFirstPersonPacketData ( const SRecording_Packet& packet ) const;
	void PrintThirdPersonPacketData ( const SRecording_Packet& packet, float& frameTime ) const;
	void PrintFirstPersonPacketData ( u8* data, size_t size, tukk const msg ) const;
	void PrintThirdPersonPacketData ( u8* data, size_t size, tukk const msg ) const;
	void PrintFirstPersonPacketData ( CRecordingBuffer& buffer, tukk const msg ) const;
	void PrintThirdPersonPacketData ( CRecordingBuffer& buffer, tukk const msg ) const;

private:
	CRecordingSystem& m_system;
};


#endif //RECSYS_DEBUG
#endif // __RECORDINGSYSTEMDEBUG_H__