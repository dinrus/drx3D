// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include <drx3D/Schema2/AggregateTypeId.h>
//#include <drx3D/Schema2/IEnvRegistry.h>
//#include <drx3D/Schema2/IEnvTypeDesc.h>
//#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IScriptFile.h>

namespace sxema2
{
	// Script structure value.
	// #SchematycTODO : Does we really need to derive from IAny or should we create a CScriptAny utility class?
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CScriptStructureValue : public IAny
	{
	public:

		CScriptStructureValue(const IScriptStructure* pStructure);
		CScriptStructureValue(const CScriptStructureValue& rhs);

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

		void Serialize(Serialization::IArchive& archive);

	private:

		typedef std::map<string, IAnyPtr> FieldMap; // #SchematycTODO : Replace map with vector to preserve order!

		void Refresh();

		const IScriptStructure* m_pStructure; // #SchematycTODO : Wouldn't it be safer to reference by GUID?
		FieldMap                m_fields;
	};
}
