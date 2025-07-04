// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SharedData.h"

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

namespace ACE
{
struct IConnection
{
	//! \cond INTERNAL
	virtual ~IConnection() = default;
	//! \endcond

	//! Returns id of the connection, which is the same id as its middleware control.
	virtual ControlId GetID() const = 0;

	//! Returns a bool if the connection has properties or not.
	virtual bool HasProperties() const = 0;

	//! Serialize connection proeprties.
	virtual void Serialize(Serialization::IArchive& ar) = 0;

	//! Enables or disables the platform of the given index.
	virtual void SetPlatformEnabled(PlatformIndexType const platformIndex, bool const isEnabled) = 0;

	//! Returns a bool if the platform of the given index is enabled or not.
	virtual bool IsPlatformEnabled(PlatformIndexType const platformIndex) const = 0;

	//! Disables all platform.
	virtual void ClearPlatforms() = 0;

	//! Signal when connection properties have changed.
	CDrxSignal<void()> SignalConnectionChanged;
};
} //endns ACE

