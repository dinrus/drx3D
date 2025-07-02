// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "BaseConnection.h"

namespace ACE
{
namespace Impl
{
namespace Fmod
{
class CBankConnection final : public CBaseConnection
{
public:

	explicit CBankConnection(ControlId const id)
		: CBaseConnection(id)
		, m_configurationsMask(std::numeric_limits<PlatformIndexType>::max())
	{}

	CBankConnection() = delete;

	// CBaseConnection
	virtual bool HasProperties() const override { return true; }
	virtual void Serialize(Serialization::IArchive& ar) override;
	virtual void SetPlatformEnabled(PlatformIndexType const platformIndex, bool const isEnabled) override;
	virtual bool IsPlatformEnabled(PlatformIndexType const platformIndex) const override;
	virtual void ClearPlatforms() override;
	// ~CBaseConnection

private:

	PlatformIndexType m_configurationsMask;
};
} //endns Fmod
} //endns Impl
} //endns ACE
