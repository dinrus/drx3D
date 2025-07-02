// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/FactionSystem.h>

bool Serialize(Serialization::IArchive& archive, SFactionID& value, tukk szName, tukk szLabel);
bool Serialize(Serialization::IArchive& archive, SFactionFlagsMask& value, tukk szName, tukk szLabel);

inline void ReflectType(sxema::CTypeDesc<IFactionMap::ReactionType>& desc)
{
	desc.SetGUID("9317594f-1e3c-4d8e-8dba-3b67bf67b935"_drx_guid);

	desc.AddConstant(IFactionMap::ReactionType::Hostile, "Hostile", "Hostile");
	desc.AddConstant(IFactionMap::ReactionType::Neutral, "Neutral", "Neutral");
	desc.AddConstant(IFactionMap::ReactionType::Friendly, "Friendly", "Friendly");
}

inline void ReflectType(sxema::CTypeDesc<SFactionID>& desc)
{
	desc.SetGUID("8ae8716f-af8a-49d3-be75-bfc7dc81e84a"_drx_guid);
}

inline void ReflectType(sxema::CTypeDesc<SFactionFlagsMask>& desc)
{
	desc.SetGUID("a060b5fd-72f5-44b8-9237-a221474a41ad"_drx_guid);
}

namespace FactionSystemSchematyc
{
	void Register(sxema::IEnvRegistrar& registrar, sxema::CEnvRegistrationScope& parentScope);
}