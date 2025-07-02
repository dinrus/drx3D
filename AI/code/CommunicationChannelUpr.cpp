// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/CommunicationChannelUpr.h>

#include <drx3D/CoreX/String/StringUtils.h>

bool CommunicationChannelUpr::LoadChannel(const XmlNodeRef& channelNode, const CommChannelID& parentID)
{
	if (!stricmp(channelNode->getTag(), "Channel"))
	{
		tukk name = 0;

		if (!channelNode->getAttr("name", &name))
		{
			AIWarning("Missing 'name' attribute for 'Channel' tag at line %d...",
			          channelNode->getLine());

			return false;
		}

		float minSilence = 0.0f;
		channelNode->getAttr("minSilence", minSilence);

		float flushSilence = minSilence;
		channelNode->getAttr("flushSilence", flushSilence);

		float actorMinSilence = 0.0f;
		channelNode->getAttr("actorMinSilence", actorMinSilence);

		bool ignoreActorSilence = false;
		channelNode->getAttr("ignoreActorSilence", ignoreActorSilence);

		//By default channels flush state on an abort request
		size_t priority = 0;
		channelNode->getAttr("priority", priority);

		SCommunicationChannelParams params;
		params.name = name;
		params.minSilence = minSilence;
		params.flushSilence = flushSilence;
		params.parentID = parentID;
		params.priority = static_cast<u8>(priority);
		params.type = SCommunicationChannelParams::Global;
		params.actorMinSilence = actorMinSilence;
		params.ignoreActorSilence = ignoreActorSilence;

		if (channelNode->haveAttr("type"))
		{
			tukk type;
			channelNode->getAttr("type", &type);

			if (!stricmp(type, "global"))
				params.type = SCommunicationChannelParams::Global;
			else if (!stricmp(type, "group"))
				params.type = SCommunicationChannelParams::Group;
			else if (!stricmp(type, "personal"))
				params.type = SCommunicationChannelParams::Personal;
			else
			{
				AIWarning("Invalid 'type' attribute for 'Channel' tag at line %d...",
				          channelNode->getLine());

				return false;
			}
		}

		std::pair<ChannelParams::iterator, bool> iresult = m_params.insert(
		  ChannelParams::value_type(GetChannelID(params.name.c_str()), params));

		if (!iresult.second)
		{
			if (iresult.first->second.name == name)
				AIWarning("Channel '%s' redefinition at line %d...", name, channelNode->getLine());
			else
				AIWarning("Channel name '%s' hash collision!", name);

			return false;
		}

		i32 childCount = channelNode->getChildCount();

		CommChannelID channelID = GetChannelID(name);

		for (i32 i = 0; i < childCount; ++i)
		{
			XmlNodeRef childChannelNode = channelNode->getChild(i);

			if (!LoadChannel(childChannelNode, channelID))
				return false;
		}
	}
	else
	{
		AIWarning("Unexpected tag '%s' found at line %d...", channelNode->getTag(), channelNode->getLine());

		return false;
	}

	return true;
}

void CommunicationChannelUpr::Clear()
{
	Reset();

	m_params.clear();
}

void CommunicationChannelUpr::Reset()
{
	m_globalChannels.clear();
	m_groupChannels.clear();
	m_personalChannels.clear();
}

void CommunicationChannelUpr::Update(float updateTime)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	Channels::iterator glit = m_globalChannels.begin();
	Channels::iterator glend = m_globalChannels.end();

	for (; glit != glend; ++glit)
		glit->second->Update(updateTime);

	GroupChannels::iterator grit = m_groupChannels.begin();
	GroupChannels::iterator grend = m_groupChannels.end();

	for (; grit != grend; ++grit)
	{
		Channels::iterator it = grit->second.begin();
		Channels::iterator end = grit->second.end();

		for (; it != end; ++it)
			it->second->Update(updateTime);
	}

	PersonalChannels::iterator prit = m_personalChannels.begin();
	PersonalChannels::iterator prend = m_personalChannels.end();

	for (; prit != prend; ++prit)
	{
		Channels::iterator it = prit->second.begin();
		Channels::iterator end = prit->second.end();

		for (; it != end; ++it)
			it->second->Update(updateTime);
	}
}

CommChannelID CommunicationChannelUpr::GetChannelID(tukk name) const
{
	return CommChannelID(DrxStringUtils::CalculateHashLowerCase(name));
}

const SCommunicationChannelParams& CommunicationChannelUpr::GetChannelParams(const CommChannelID& channelID) const
{
	ChannelParams::const_iterator it = m_params.find(channelID);

	static SCommunicationChannelParams noChannel;

	if (it == m_params.end())
		return noChannel;
	else
		return it->second;
}

CommunicationChannel::Ptr CommunicationChannelUpr::GetChannel(const CommChannelID& channelID,
                                                                  EntityId sourceId)
{
	ChannelParams::iterator it = m_params.find(channelID);
	if (it != m_params.end())
	{
		const SCommunicationChannelParams& params = it->second;

		switch (params.type)
		{
		case SCommunicationChannelParams::Global:
			{
				CommunicationChannel::Ptr& channel = stl::map_insert_or_get(m_globalChannels, channelID);
				if (!channel)
				{
					CommunicationChannel::Ptr parent =
					  params.parentID ? GetChannel(params.parentID, sourceId) : CommunicationChannel::Ptr(0);

					channel.reset(new CommunicationChannel(parent, params, channelID));
				}

				return channel;
			}
		case SCommunicationChannelParams::Group:
			{
				i32 groupID = 0;
				Channels& groupChannels = stl::map_insert_or_get(m_groupChannels, groupID);

				CommunicationChannel::Ptr& channel = stl::map_insert_or_get(groupChannels, channelID);
				if (!channel)
				{
					CommunicationChannel::Ptr parent =
					  params.parentID ? GetChannel(params.parentID, sourceId) : CommunicationChannel::Ptr(0);

					channel.reset(new CommunicationChannel(parent, params, channelID));
				}

				return channel;
			}
		case SCommunicationChannelParams::Personal:
			{
				Channels& personalChannels = stl::map_insert_or_get(m_personalChannels, sourceId);

				CommunicationChannel::Ptr& channel = stl::map_insert_or_get(personalChannels, channelID);
				if (!channel)
				{
					CommunicationChannel::Ptr parent =
					  params.parentID ? GetChannel(params.parentID, sourceId) : CommunicationChannel::Ptr(0);

					channel.reset(new CommunicationChannel(parent, params, channelID));
				}

				return channel;
			}
		default:
			assert(0);
			break;
		}
	}

	assert(0);

	return 0;
}
