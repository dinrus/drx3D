// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IConnection.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
class CEventConnection final : public IConnection
{
public:

	enum class EActionType
	{
		Start,
		Stop,
	};

	explicit CEventConnection(ControlId const id)
		: m_id(id)
		, m_actionType(EActionType::Start)
		, m_loopCount(1)
		, m_isInfiniteLoop(false)
	{}

	virtual ~CEventConnection() override = default;

	CEventConnection() = delete;

	// IConnection
	virtual ControlId GetID() const override                                                                   { return m_id; }
	virtual bool      HasProperties() const override                                                           { return true; }
	virtual void      Serialize(Serialization::IArchive& ar) override;
	virtual void      SetPlatformEnabled(PlatformIndexType const platformIndex, bool const isEnabled) override {}
	virtual bool      IsPlatformEnabled(PlatformIndexType const platformIndex) const override                  { return true; }
	virtual void      ClearPlatforms() override                                                                {}
	// ~IConnection

	ControlId const m_id;
	EActionType     m_actionType;
	u32          m_loopCount;
	bool            m_isInfiniteLoop;
};
} //endns PortAudio
} //endns Impl
} //endns ACE
