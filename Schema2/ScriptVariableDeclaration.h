// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/AggregateTypeId.h>
#include <drx3D/Schema2/IEnvTypeDesc.h>
#include <drx3D/Schema2/IScriptFile.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/IEnvRegistry.h>

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

		virtual u32 GetSize() const override;
		virtual CTypeInfo GetTypeInfo() const override;
		virtual IAny* Clone(uk pPlacement) const override;
		virtual IAnyPtr Clone() const override;
		virtual bool ToString(const CharArrayView& str) const override;
		virtual GameSerialization::IContextPtr BindSerializationContext(Serialization::IArchive& archive) const override;
		virtual bool Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel) override;
		virtual IAnyExtension* QueryExtension(EAnyExtension extension) override;
		virtual const IAnyExtension* QueryExtension(EAnyExtension extension) const override;
		virtual uk ToVoidPtr() override;
		virtual ukk ToVoidPtr() const override;

		virtual IAny& operator = (const IAny& rhs) override;
		virtual bool operator == (const IAny& rhs) const override;

		// ~IAny

	private:

		const IScriptEnumeration* m_pEnumeration; // #SchematycTODO : Wouldn't it be safer to reference by GUID?
		size_t                    m_value; // #SchematycTODO : Would it be safer to store a string?
	};

	// Script structure value.
	// #SchematycTODO : Does we really need to derive from IAny or should we create a CScriptAny utility class?
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CScriptStructureValue : public IAny
	{
	public:

		CScriptStructureValue(const IScriptStructure* pStructure);
		CScriptStructureValue(const CScriptStructureValue& rhs);

		// IAny

		virtual u32 GetSize() const override;
		virtual CTypeInfo GetTypeInfo() const override;
		virtual IAny* Clone(uk pPlacement) const override;
		virtual IAnyPtr Clone() const override;
		virtual bool ToString(const CharArrayView& str) const override;
		virtual GameSerialization::IContextPtr BindSerializationContext(Serialization::IArchive& archive) const override;
		virtual bool Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel) override;
		virtual IAnyExtension* QueryExtension(EAnyExtension extension) override;
		virtual const IAnyExtension* QueryExtension(EAnyExtension extension) const override;
		virtual uk ToVoidPtr() override;
		virtual ukk ToVoidPtr() const override;

		virtual IAny& operator = (const IAny& rhs) override;
		virtual bool operator == (const IAny& rhs) const override;

		// ~IAny

		void Serialize(Serialization::IArchive& archive);

	private:

		typedef std::map<string, IAnyPtr> FieldMap; // #SchematycTODO : Replace map with vector to preserve order!

		void Refresh();

		const IScriptStructure* m_pStructure; // #SchematycTODO : Wouldn't it be safer to reference by GUID?
		FieldMap                m_fields;
	};

	// Validate script variable type information.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool ValidateScriptVariableTypeInfo(const IScriptFile& file, const CAggregateTypeId& typeId);

	// Get script variable type name.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	tukk GetScriptVariableTypeName(const IScriptFile& file, const CAggregateTypeId& typeId);

	// Make script variable value.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	IAnyPtr MakeScriptVariableValueShared(const IScriptFile& file, const CAggregateTypeId& typeId);

	// Script variable declaration.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CScriptVariableDeclaration
	{
	public:

		CScriptVariableDeclaration();
		CScriptVariableDeclaration(tukk szName, const CAggregateTypeId& typeId, const IAnyPtr& pValue = IAnyPtr());
		CScriptVariableDeclaration(tukk szName, const IAnyPtr& pValue); // #SchematycTODO : Ditch this constructor, it's unreliable!!!

		void SetName(tukk szName);
		tukk GetName() const;
		CAggregateTypeId GetTypeId() const;
		IAnyConstPtr GetValue() const;
		void EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const;
		void Serialize(Serialization::IArchive& archive);
		void SerializeInfo(Serialization::IArchive& archive);
		void SerializeValue(Serialization::IArchive& archive);
		void RemapGUIDs(IGUIDRemapper& guidRemapper);

	private:

		string           m_name;
		CAggregateTypeId m_typeId; // #SchematycTODO : Do we really need to store type id when we can query if from m_pValue?
		IAnyPtr          m_pValue;
	};
}
