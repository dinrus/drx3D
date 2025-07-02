// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/ScopedConnection.h>

namespace sxema
{
typedef std::function<void (TSerialize, i32, u8, i32)> NetworkSerializeCallback;

struct INetworkObject
{
	~INetworkObject() {}

	virtual bool   RegisterSerializeCallback(i32 aspects, const NetworkSerializeCallback& callback, CConnectionScope& connectionScope) = 0;
	virtual void   MarkAspectsDirty(i32 aspects) = 0;
	virtual u16 GetChannelId() const = 0;
	virtual bool   ServerAuthority() const = 0;
	virtual bool   ClientAuthority() const = 0;
	virtual bool   LocalAuthority() const = 0;
};
} // sxema
