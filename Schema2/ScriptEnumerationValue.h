// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include <drx3D/Schema2/AggregateTypeId.h>
//#include <drx3D/Schema2/IEnvRegistry.h>
//#include <drx3D/Schema2/IEnvTypeDesc.h>
//#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IScriptFile.h>

namespace sxema2
{
	// Script enumeration value.
	// #SchematycTODO : Does we really need to derive from IAny or should we create a CScriptAny utility class?
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CScriptEnumerationValue : public IAny
	{
	public:

		CScriptEnumerationValue(const IScriptEnumeration* pEnumeration);
		CScriptEnumerationValue(const CScriptEnumerationValue& rhs);

		// IAny
		virtual CTypeInfo GetTypeInfo() const override;
		virtual u32 GetSize() const override;
		virtual bool Copy(const IAny& rhs) override;
		virtual IAny* Clone(uk pPlacement) const override;
		virtual IAnyPtr Clone() const override;
		virtual bool ToString(const CharArrayView& str) const override;
		virtual GameSerialization::IContextPtr BindSerializationContext(Serialization::IArchive& archive) const override;
		virtual bool Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel) override;
		virtual IAnyExtension* QueryExtension(EAnyExtension extension) override;
		virtual const IAnyExtension* QueryExtension(EAnyExtension extension) const override;
		virtual uk ToVoidPtr() override;
		virtual ukk ToVoidPtr() const override;
		// ~IAny

	private:

		const IScriptEnumeration* m_pEnumeration; // #SchematycTODO : Wouldn't it be safer to reference by GUID?
		size_t                    m_value; // #SchematycTODO : Would it be safer to store a string?
	};
}
