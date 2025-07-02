// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/INetwork.h>

namespace sxema
{
struct INetworkSpawnParams : public ISerializableInfo
{
	virtual ~INetworkSpawnParams() {}

	virtual _smart_ptr<INetworkSpawnParams> Clone() const = 0;
};

typedef _smart_ptr<INetworkSpawnParams> INetworkSpawnParamsPtr;   // #SchematycTODO : Can we replace _smart_ptr with std::share_ptr?

struct IObjectNetworkSpawnParams : public ISerializableInfo
{
	virtual ~IObjectNetworkSpawnParams() {}

	virtual void                   AddComponentSpawnParams(u32 componentIdx, const INetworkSpawnParamsPtr& pParams) = 0;
	virtual INetworkSpawnParamsPtr GetComponentSpawnParams(u32 componentIdx) const = 0;
};

typedef _smart_ptr<IObjectNetworkSpawnParams> IObjectNetworkSpawnParamsPtr; // #SchematycTODO : Can we replace _smart_ptr with std::share_ptr?

} // sxema
