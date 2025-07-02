// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/FundamentalTypes.h>

namespace sxema
{

// Forward declare interfaces.
struct IScriptEnum;

// Script enumeration value.
////////////////////////////////////////////////////////////////////////////////////////////////////
class CScriptEnumValue
{
public:

	CScriptEnumValue(const IScriptEnum* pEnum);
	CScriptEnumValue(const CScriptEnumValue& rhs);

	bool        Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel);
	void        ToString(IString& output) const;

	static void ReflectType(CTypeDesc<CScriptEnumValue>& desc);

private:

	const IScriptEnum* m_pEnum;       // #SchematycTODO : Wouldn't it be safer to reference by GUID?
	u32             m_constantIdx; // #SchematycTODO : Wouldn't it be safer to store a string?
};

bool Serialize(Serialization::IArchive& archive, CScriptEnumValue& value, tukk szName, tukk szLabel);

} // sxema
