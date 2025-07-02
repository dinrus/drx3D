// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CommunicationChannel_h__
#define __CommunicationChannel_h__

#pragma once

#include <drx3D/AI/Communication.h>

class CommunicationChannel
	: public _reference_target_t
{
public:
	typedef _smart_ptr<CommunicationChannel> Ptr;

public:
	CommunicationChannel(const CommunicationChannel::Ptr& parent, const SCommunicationChannelParams& params, const CommChannelID& channelID);

	void                                                   Update(float updateTime);
	void                                                   Occupy(bool occupy, float minSilence = -1.0f);

	bool                                                   IsFree() const;
	void                                                   Clear();

	CommChannelID                                          GetID() const               { return m_id; }

	u8                                                  GetPriority() const         { return m_priority; }

	bool                                                   IsOccupied() const          { return m_occupied; }

	SCommunicationChannelParams::ECommunicationChannelType GetType()                   { return m_type; }

	float                                                  GetActorSilence() const     { return m_actorMinSilence; }
	bool                                                   IgnoresActorSilence() const { return m_ignoreActorSilence; }

	void                                                   ResetSilence();

private:
	CommunicationChannel();

	CommChannelID             m_id;
	CommunicationChannel::Ptr m_parent;

	// Minimum silence this channel imposes once normal communication is finished
	const float m_minSilence;
	// Minimum silence this channel imposes on manager if its higher priority and flushes the system
	const float m_flushSilence;

	// Minimum silence this channel imposes on an actor once it starts to play
	const float m_actorMinSilence;
	// Indicates if this channel should ignore minimum actor silence times.
	bool        m_ignoreActorSilence;

	float       m_silence;
	bool        m_occupied;
	SCommunicationChannelParams::ECommunicationChannelType m_type;
	u8       m_priority;
};

#endif //__CommunicationChannel_h__
