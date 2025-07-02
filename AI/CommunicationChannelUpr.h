// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CommunicationChannelUpr_h__
#define __CommunicationChannelUpr_h__

#pragma once

#include <drx3D/AI/Communication.h>
#include <drx3D/AI/CommunicationChannel.h>

class CommunicationChannelUpr
{
public:
	bool                               LoadChannel(const XmlNodeRef& channelNode, const CommChannelID& parentID);

	void                               Clear();
	void                               Reset();
	void                               Update(float updateTime);

	CommChannelID                      GetChannelID(tukk name) const;
	const SCommunicationChannelParams& GetChannelParams(const CommChannelID& channelID) const;
	CommunicationChannel::Ptr          GetChannel(const CommChannelID& channelID, EntityId sourceId) const;
	CommunicationChannel::Ptr          GetChannel(const CommChannelID& channelID, EntityId sourceId);

private:
	typedef std::map<CommChannelID, SCommunicationChannelParams> ChannelParams;
	ChannelParams m_params;

	typedef std::map<CommChannelID, CommunicationChannel::Ptr> Channels;
	Channels m_globalChannels;

	typedef std::map<i32, Channels> GroupChannels;
	GroupChannels m_groupChannels;

	typedef std::map<EntityId, Channels> PersonalChannels;
	PersonalChannels m_personalChannels;
};

#endif //__CommunicationChannelUpr_h__
