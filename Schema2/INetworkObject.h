// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_Delegate.h>
#include <drx3D/Schema2/TemplateUtils_ScopedConnection.h>

namespace sxema2
{
	typedef TemplateUtils::CDelegate<void (TSerialize, i32, u8, i32)>        NetworkSerializeCallback;
	typedef std::pair<NetworkSerializeCallback, TemplateUtils::CScopedConnection> NetworkSerializeCallbackConnection;

	struct INetworkObject
	{
		enum class EAspectDelegation
		{
			Server, 
			Authority,
		};

		~INetworkObject() {}

		virtual bool RegisterNetworkSerializer(const NetworkSerializeCallbackConnection& callbackConnection, i32 aspects) = 0;
		virtual void MarkAspectsDirty(i32 aspects) = 0;
		virtual void SetAspectDelegation(i32 aspects, const EAspectDelegation delegation) = 0;
		virtual u16 GetChannelId() const = 0;
		virtual bool ServerAuthority() const = 0;
		virtual bool ClientAuthority() const = 0;
		virtual bool LocalAuthority() const = 0;
	};
}
