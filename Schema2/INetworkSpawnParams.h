// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

namespace sxema2
{
	struct INetworkSpawnParams : public ISerializableInfo
	{
		virtual _smart_ptr<INetworkSpawnParams> Clone() const = 0;
	};

	typedef _smart_ptr<INetworkSpawnParams> INetworkSpawnParamsPtr;

	struct IObjectNetworkSpawnParams : public ISerializableInfo
	{
		virtual void AddComponentSpawnParams(u32 componentIdx, const INetworkSpawnParamsPtr& pParams) = 0;
		virtual INetworkSpawnParamsPtr GetComponentSpawnParams(u32 componentIdx) const = 0;
	};

	typedef _smart_ptr<IObjectNetworkSpawnParams> IObjectNetworkSpawnParamsPtr;
}
