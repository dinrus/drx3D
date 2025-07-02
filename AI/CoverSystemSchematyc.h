// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/CoverSystem.h>

inline bool Serialize(Serialization::IArchive& archive, CoverID& value, tukk szName, tukk szLabel)
{
	if (!archive.isEdit())
	{
		if (archive.isInput())
		{
			u32 rawValue;
			if (archive(rawValue, szName, szLabel))
			{
				value = CoverID(rawValue);
				return true;
			}
			return false;
		}
		else // Output
		{
			return archive(u32(value), szName, szLabel);
		}
	}
	return true;
}

inline void ReflectType(sxema::CTypeDesc<CoverID>& desc)
{
	desc.SetGUID("62FD8F5A-F3D7-45B9-8384-9D7D9DBEDBAE"_drx_guid);
	desc.SetLabel("CoverId");
	desc.SetDescription("Cover identifier");
	desc.SetDefaultValue(CoverID());
}

namespace CoverSystemSchematyc
{
	void Register(sxema::IEnvRegistrar& registrar, sxema::CEnvRegistrationScope& parentScope);
}